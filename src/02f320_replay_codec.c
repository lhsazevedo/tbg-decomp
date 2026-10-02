/* @unit ReplayCodec */
#include <shinobi.h>
#include "includes.h" /* STATIC */
#include "02f320_replay_codec.h"

/* =======================
 * Non-initialized Globals
 * =======================
 */

Uint32 var_replayPackedSize_8c228ba4;
/* signed: goes negative to trigger a refill */
Sint16 var_readBitsLeft_8c228ba8;
Uint16 var_writeBitsLeft_8c228baa;
Uint16 var_bitBuf_8c228bac;
Uint8 var_dictByte_8c228bae[0x1000];
Sint16 var_dictParent_8c229bae[0x1000];
Sint16 var_dictChild_8c22bbae[0x1000];
Sint16 var_dictNext_8c22dbae[0x1000];
Sint16 var_dictPrev_8c22fbae[0x1000];
Uint16 var_nextCode_8c231bae;
Sint16 var_lruPrev_8c231bb0[0x1000];
Sint16 var_lruNext_8c233bb0[0x1000];
Uint16 var_lruHead_8c235bb0;
Uint16 var_lruTail_8c235bb2;
Sint16 var_runBuf_8c235bb4[100];
Uint16 var_codeBits_8c235c7c;
Uint16 var_codeLimit_8c235c7e;

/* ====================
 * Functions
 * ====================
 */

void ReplayCodecInit_8c02f320(void)
{
    var_replayPackedSize_8c228ba4 = 0;
    var_readBitsLeft_8c228ba8 = 0;
    var_writeBitsLeft_8c228baa = 8;
    var_bitBuf_8c228bac = 0;
    var_lruHead_8c235bb0 = 0x1000;
    var_lruTail_8c235bb2 = 0x1000;
    var_codeBits_8c235c7c = 1;
    var_codeLimit_8c235c7e = 2;
    var_nextCode_8c231bae = 0x100;
    memset(&var_dictByte_8c228bae, 0, 0x1000);
    memset(&var_dictParent_8c229bae, 0, 0x1000);
    memset(&var_dictChild_8c22bbae, 0, 0x1000);
    memset(&var_dictNext_8c22dbae, 0, 0x1000);
    memset(&var_dictPrev_8c22fbae, 0, 0x1000);
    memset(&var_lruPrev_8c231bb0, 0, 0x1000);
    memset(&var_lruNext_8c233bb0, 0, 0x1000);
    memset(&var_runBuf_8c235bb4, 0, 100);
}

/* Bit-stream reader: returns the next bit (MSB-first) from *src, advancing
 * *src and bumping **countPtr once per byte consumed, refilling the bit
 * buffer on underflow. */
STATIC Uint32 getBit_8c02f3a0(Uint8 **src, Uint32 **countPtr)
{
    Sint16 n;
    Uint32 *count;

    n = --var_readBitsLeft_8c228ba8;
    if (n < 0) {
        var_readBitsLeft_8c228ba8 = 7;
        n = 7;
        count = *countPtr;
        var_bitBuf_8c228bac = **src;
        (*count)++;
        (*src)++;
        *countPtr = count;
    }

    return (Uint32)(var_bitBuf_8c228bac >> n) & 1;
}

/* Reads a `count`-bit value (MSB-first) from *src, refilling the bit buffer
 * one byte at a time as needed. */
STATIC Uint32 getBits_8c02f3e0(Sint16 count, Uint8 **src, Uint32 **countPtr)
{
    Uint32 result;
    Sint16 n;
    Uint8 *srcPtr;
    Uint32 *count32;

    result = 0;
    srcPtr = *src;
    count32 = *countPtr;
    while (count > var_readBitsLeft_8c228ba8) {
        n = var_readBitsLeft_8c228ba8;
        count -= n;
        result |= (Uint32)((Uint16)((1 << n) - 1) & var_bitBuf_8c228bac) << count;
        var_bitBuf_8c228bac = *srcPtr++;
        (*count32)++;
        var_readBitsLeft_8c228ba8 = 8;
    }

    var_readBitsLeft_8c228ba8 -= count;
    *src = srcPtr;
    *countPtr = count32;
    return result |
        ((Uint32)(var_bitBuf_8c228bac >> var_readBitsLeft_8c228ba8) &
         (Uint32)((1 << count) - 1));
}

/* Bit-stream writer: packs one bit (MSB-first) into the bit buffer, flushing
 * the byte to *dest and counting it once full. */
STATIC void putBit_8c02f49c(Sint16 bit, Uint8 **dest)
{
    Uint8 *destPtr;

    destPtr = *dest;
    var_writeBitsLeft_8c228baa--;
    if (bit != 0) {
        var_bitBuf_8c228bac |= 1 << var_writeBitsLeft_8c228baa;
    }

    if (var_writeBitsLeft_8c228baa == 0) {
        *destPtr++ = (Uint8)var_bitBuf_8c228bac;
        var_bitBuf_8c228bac = 0;
        var_writeBitsLeft_8c228baa = 8;
        var_replayPackedSize_8c228ba4++;
    }

    *dest = destPtr;
}

/* Packs a `count`-bit value (MSB-first) into the output stream, flushing a
 * byte at a time as the bit buffer fills. */
STATIC void putBits_8c02f4da(Sint16 count, Uint32 value, Uint8 **dest)
{
    Uint8 *destPtr;

    destPtr = *dest;
    while (var_writeBitsLeft_8c228baa <= count) {
        Sint16 n;

        n = var_writeBitsLeft_8c228baa;
        count -= n;
        var_bitBuf_8c228bac |= (Uint8)(((1 << n) - 1) & (value >> count));
        *destPtr++ = (Uint8)var_bitBuf_8c228bac;
        var_bitBuf_8c228bac = 0;
        var_writeBitsLeft_8c228baa = 8;
        var_replayPackedSize_8c228ba4++;
    }

    var_writeBitsLeft_8c228baa -= count;
    var_bitBuf_8c228bac |=
        (Uint16)(((1 << count) - 1) & value) << var_writeBitsLeft_8c228baa;
    *dest = destPtr;
}

/* Unlinks `code` from the recency list, dragging the tail back with it when
 * `code` is the tail. Unlinking the head works through the NIL sentinel:
 * var_lruNext_8c233bb0[0x1000] is var_lruHead_8c235bb0's storage. */
STATIC void lruUnlink_8c02f556(Sint16 code)
{
    Sint16 p;
    Sint16 c;

    if (code == (Sint16)var_lruTail_8c235bb2) {
        var_lruTail_8c235bb2 = var_lruPrev_8c231bb0[code];
        var_lruNext_8c233bb0[var_lruTail_8c235bb2] = 0x1000;
    } else {
        c = var_lruNext_8c233bb0[code];
        p = var_lruPrev_8c231bb0[code];
        var_lruPrev_8c231bb0[c] = p;
        var_lruNext_8c233bb0[p] = c;
    }
}

/* Inserts `code` into the recency list: as the sole entry if the list is
 * empty, at the tail if `after` is NIL (0x1000), at the head if `after` is
 * the current head, or after node `after` otherwise. */
STATIC void lruInsert_8c02f58a(Sint16 code, Sint16 after)
{
    if (var_lruHead_8c235bb0 == 0x1000) {
        var_lruPrev_8c231bb0[code] = 0x1000;
        var_lruNext_8c233bb0[code] = 0x1000;
        var_lruTail_8c235bb2 = code;
        var_lruHead_8c235bb0 = code;
    } else if (after == 0x1000) {
        var_lruNext_8c233bb0[code] = 0x1000;
        var_lruPrev_8c231bb0[code] = var_lruTail_8c235bb2;
        var_lruNext_8c233bb0[var_lruTail_8c235bb2] = code;
        var_lruTail_8c235bb2 = code;
    } else if (after == (Sint16)var_lruHead_8c235bb0) {
        var_lruNext_8c233bb0[code] = var_lruHead_8c235bb0;
        var_lruPrev_8c231bb0[code] = 0x1000;
        var_lruPrev_8c231bb0[var_lruHead_8c235bb0] = code;
        var_lruHead_8c235bb0 = code;
    } else {
        var_lruNext_8c233bb0[code] = after;
        var_lruPrev_8c231bb0[code] = var_lruPrev_8c231bb0[after];
        var_lruNext_8c233bb0[var_lruPrev_8c231bb0[code]] = code;
        var_lruPrev_8c231bb0[after] = code;
    }
}

/* Walks `parent`'s child chain for the node whose byte equals `value`;
 * returns its code, or 0x1000 (NIL) if absent. */
STATIC Sint32 lzwFindChild_8c02f636(Sint16 parent, Uint16 value)
{
    Sint16 node;

    node = var_dictChild_8c22bbae[parent];
    while (node != 0x1000 && value != var_dictByte_8c228bae[node]) {
        node = var_dictNext_8c22dbae[node];
    }

    return (Sint32)node;
}

/* Adds dictionary node `code` = (parent `parentCode`, appended byte
 * `value`) as the new head of parentCode's child chain. */
STATIC void lzwInsertChild_8c02f668(Sint16 parentCode, Sint16 code, Uint8 value)
{
    Sint16 oldHead;

    var_dictByte_8c228bae[code] = value;
    var_dictParent_8c229bae[code] = parentCode;
    var_dictPrev_8c22fbae[code] = 0x1000;
    var_dictChild_8c22bbae[code] = 0x1000;

    oldHead = var_dictChild_8c22bbae[parentCode];
    var_dictNext_8c22dbae[code] = oldHead;
    if (oldHead != 0x1000) {
        var_dictPrev_8c22fbae[oldHead] = code;
    }
    var_dictChild_8c22bbae[parentCode] = code;
}

/* Unlinks dictionary node `code` from its parent's child chain. */
STATIC void lzwRemoveChild_8c02f6ac(Sint16 code)
{
    Sint16 prevNode;
    Sint16 nextNode;

    prevNode = var_dictPrev_8c22fbae[code];
    nextNode = var_dictNext_8c22dbae[code];

    if (prevNode == 0x1000) {
        var_dictChild_8c22bbae[var_dictParent_8c229bae[code]] = nextNode;
    } else {
        var_dictNext_8c22dbae[prevNode] = nextNode;
    }

    if (nextNode != 0x1000) {
        var_dictPrev_8c22fbae[nextNode] = prevNode;
    }
}

/* Seeds the first 0x100 codes as the literal bytes: identity byte map, and
 * the 0x1000 NIL sentinel in every link table. */
STATIC void initTables_8c02f704(void)
{
    Sint16 i;

    for (i = 0; i < 0x100; i++) {
        var_dictByte_8c228bae[i] = (Uint8)i;
        var_dictNext_8c22dbae[i] = 0x1000;
        var_dictPrev_8c22fbae[i] = 0x1000;
        var_dictChild_8c22bbae[i] = 0x1000;
        var_dictParent_8c229bae[i] = 0x1000;
    }
}

/* Extends the LZW dictionary while walking `count` more input bytes
 * (`symbols`) from parent code `parentCode`, starting at accumulated
 * position `pos` (capped at 100 total updates per call, matching the run
 * buffer's element count). For each byte not already a child of the current
 * code, allocates a new code -- from the free pool while it lasts, otherwise
 * evicting the recency-list tail -- links it into its parent's child chain,
 * and appends it to the recency list. */
STATIC void extendDict_8c02f740(Sint16 *symbols, Sint16 count, Sint16 parentCode, Sint16 pos)
{
    Sint16 i;
    Sint16 newCode;
    Sint16 value;
    Sint16 afterCode;

    if (parentCode == (Sint16)0x1000 || count <= 0) {
        return;
    }

    for (i = 0; i < count; i++) {
        pos++;
        if (pos > 100) {
            return;
        }

        value = symbols[i];
        newCode = (Sint16)lzwFindChild_8c02f636(parentCode, (Uint16)value);

        if (newCode == (Sint16)0x1000) {
            if (var_nextCode_8c231bae < 0x1000) {
                newCode = (Sint16)var_nextCode_8c231bae;
                var_nextCode_8c231bae++;
            } else {
                newCode = (Sint16)var_lruTail_8c235bb2;
                if (parentCode == (Sint16)var_lruTail_8c235bb2) {
                    return;
                }
                lruUnlink_8c02f556(newCode);
                lzwRemoveChild_8c02f6ac(newCode);
            }

            lzwInsertChild_8c02f668(parentCode, newCode, (Uint8)value);

            afterCode = (Sint16)var_lruHead_8c235bb0;
            if (parentCode > 0xff) {
                afterCode = var_lruNext_8c233bb0[parentCode];
            }
            lruInsert_8c02f58a(newCode, afterCode);
        }

        parentCode = newCode;
    }
}

/* Writes one code to the bitstream: a literal byte (control bit 0 + 8 bits)
 * if code < 0x100, otherwise (control bit 1 + an escalating-width value,
 * code - 0x100) after growing the code width to match the current dictionary
 * size -- the write-side mirror of readCode_8c02f892. */
STATIC void writeCode_8c02f824(Sint32 code, Uint8 **dest)
{
    Uint8 *destPtr;

    destPtr = *dest;

    if ((Sint16)code < 0x100) {
        putBit_8c02f49c(0, &destPtr);
        putBits_8c02f4da(8, (Uint32)code, &destPtr);
    } else {
        while (var_codeLimit_8c235c7e <= var_nextCode_8c231bae - 0x100) {
            var_codeBits_8c235c7c++;
            var_codeLimit_8c235c7e <<= 1;
        }
        putBit_8c02f49c(1, &destPtr);
        putBits_8c02f4da((Sint16)var_codeBits_8c235c7c, (Uint32)code - 0x100, &destPtr);
    }

    *dest = destPtr;
}

/* Reads one code from the bitstream: a literal byte (0..0xFF) if the next
 * bit is 0, or 0x100 + an escalating-width value if it's 1 (the width grows
 * once the dictionary outgrows var_codeLimit_8c235c7e). Returns -1 once
 * *countPtr exceeds limit. */
STATIC Sint32 readCode_8c02f892(Uint8 **src, Uint32 limit, Uint32 *countPtr)
{
    Uint8 *srcPtr;
    Uint32 *count;
    Sint16 bit;
    Sint32 result;

    srcPtr = *src;
    if (var_codeLimit_8c235c7e <= var_nextCode_8c231bae - 0x100) {
        var_codeLimit_8c235c7e <<= 1;
        var_codeBits_8c235c7c++;
    }

    count = countPtr;
    bit = (Sint16)getBit_8c02f3a0(&srcPtr, &count);

    if (*count <= limit) {
        if (bit == 0) {
            result = (Sint32)getBits_8c02f3e0(8, &srcPtr, &count);
            *src = srcPtr;
            return result;
        }

        result = (Sint32)getBits_8c02f3e0((Sint16)var_codeBits_8c235c7c, &srcPtr, &count);
        if (*count <= limit) {
            *src = srcPtr;
            return result + 0x100;
        }
    }

    *src = srcPtr;
    return -1;
}

/* LZW-encodes `size` bytes from `src` into **dest (advancing it): a 4-byte
 * size header, then one code per longest-matching dictionary run, extending
 * the dictionary (extendDict_8c02f740) after each run and promoting
 * reused multi-byte codes to the front of the recency list as they're
 * matched. Finishes with a 7-bit pad flush. */
void ReplayCodecPack_8c02f934(void *src, void **dest, Uint32 size)
{
    Uint8 *srcPtr;
    Uint8 *destPtr;
    Uint32 consumed;
    Sint16 count;
    Sint16 matchCode;
    Sint16 currentCode;
    Sint16 nextByte;
    Sint16 prevParent;
    Sint16 prevCount;
    Sint16 recency;

    count = 0;
    consumed = 0;

    ReplayCodecInit_8c02f320();

    destPtr = *dest;
    memcpy(destPtr, &size, 4);
    destPtr += 4;

    initTables_8c02f704();

    srcPtr = (Uint8 *)src;
    nextByte = *srcPtr++;
    matchCode = 0x1000;
    consumed++;

    while (consumed <= size) {
        prevParent = matchCode;
        prevCount = count;
        recency = (Sint16)var_lruHead_8c235bb0;
        count = 0;
        currentCode = nextByte;

        do {
            matchCode = currentCode;
            if (matchCode > 0xff) {
                if (matchCode == recency) {
                    recency = var_lruNext_8c233bb0[matchCode];
                } else {
                    lruUnlink_8c02f556(matchCode);
                    lruInsert_8c02f58a(matchCode, recency);
                }
            }

            var_runBuf_8c235bb4[count] = nextByte;
            count++;
            nextByte = *srcPtr++;
            consumed++;
            currentCode = (Sint16)lzwFindChild_8c02f636(matchCode, (Uint16)nextByte);
        } while (currentCode != 0x1000);

        writeCode_8c02f824((Sint32)matchCode, &destPtr);
        extendDict_8c02f740(var_runBuf_8c235bb4, count, prevParent, prevCount);
    }

    putBits_8c02f4da(7, 0, &destPtr);
    *dest = destPtr;
}

/* LZW-decodes from *src (a 4-byte decompressed-size header, then `size`
 * bytes' worth of codes) into **dest (advancing it): each code is expanded
 * by walking its dictionary parent chain back to a literal byte, writing the
 * run into the tail of the run buffer, then copied forward to the output;
 * the dictionary is extended
 * (extendDict_8c02f740) after each run, mirroring ReplayCodecPack. */
void ReplayCodecUnpack_8c02fa14(void *src, void **dest, Uint32 size)
{
    Uint8 *destPtr;
    Uint8 *srcPtr;
    Uint32 consumed;
    Uint32 headerSize;
    Uint32 outputCount;
    Sint16 prevCode;
    Sint16 prevExpandCount;
    Sint16 expandCount;
    Sint32 readResult;
    Sint16 code;
    Sint16 leafCode;
    Sint16 walkCode;
    Sint16 *run;
    Sint16 i;

    consumed = 0;
    destPtr = (Uint8 *)*dest;
    srcPtr = (Uint8 *)src;

    memcpy(&headerSize, src, 4);
    srcPtr = (Uint8 *)src + 4;
    consumed += 4;

    ReplayCodecInit_8c02f320();
    initTables_8c02f704();

    expandCount = 0;
    prevCode = 0x1000;
    outputCount = 0;

    while (1) {
        if (headerSize <= outputCount) {
            *dest = destPtr;
            return;
        }

        readResult = readCode_8c02f892(&srcPtr, size, &consumed);
        code = (Sint16)readResult;
        if (code == -1) {
            break;
        }
        if ((Sint16)var_nextCode_8c231bae <= code) {
            return;
        }

        prevExpandCount = expandCount;
        expandCount = 0;
        leafCode = code;
        walkCode = code;

        while (walkCode != 0x1000) {
            code = (Sint16)readResult;
            if (code > 0xff && code != (Sint16)var_lruHead_8c235bb0) {
                lruUnlink_8c02f556((Sint16)readResult);
                lruInsert_8c02f58a((Sint16)readResult, (Sint16)var_lruHead_8c235bb0);
            }
            expandCount++;
            var_runBuf_8c235bb4[100 - expandCount] = var_dictByte_8c228bae[code];
            walkCode = var_dictParent_8c229bae[code];
            readResult = walkCode;
        }

        /* The walk above filled the run backwards, so it ends at the top of
         * var_runBuf_8c235bb4; `run` is its first symbol. extendDict reads the
         * run from here too -- passing the buffer base instead feeds it
         * whatever the previous run left at the bottom. */
        run = &var_runBuf_8c235bb4[100 - expandCount];

        {
            /* Byte-typed view, not a plain Sint16 index: SHC compiles a
             * direct word-array copy loop into MOV.W @Rm+,Rn, which
             * sh4objtest's interpreter doesn't implement. */
            Uint8 *runBytes = (Uint8 *)run;

            for (i = 0; i < expandCount; i++) {
                destPtr[i] = runBytes[i * 2];
            }
            destPtr += expandCount;
        }

        extendDict_8c02f740(run, expandCount, prevCode, prevExpandCount);
        outputCount += (Uint32)(Uint16)expandCount;
        prevCode = leafCode;
    }
}
