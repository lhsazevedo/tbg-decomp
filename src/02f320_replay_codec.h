#include <shinobi.h>

#ifndef _02F320_REPLAY_CODEC_H_
#define _02F320_REPLAY_CODEC_H_

/* Bytes written by the last ReplayCodecPack_8c02f934 */
extern Uint32 var_replayPackedSize_8c228ba4;

void ReplayCodecInit_8c02f320(void);

/* Advances *dest past the output. */
void ReplayCodecPack_8c02f934(void *src, void **dest, Uint32 size);

/* `size` is the packed byte count. `*dest` is advanced only when the size
 * header is satisfied; running out of packed bytes first, the normal way a
 * demo file ends, leaves it untouched. */
void ReplayCodecUnpack_8c02fa14(void *src, void **dest, Uint32 size);

#endif /* _02F320_REPLAY_CODEC_H_ */
