#!/usr/bin/env python3
import sys, struct, mmap, os

# Constants from prompt
BASE_ADDR   = 0x8C010000
TABLE_ADDR  = 0x8C044C08
ENTRY_COUNT = 67

def addr_to_off(addr: int, size: int) -> int:
    """Convert 32-bit little-endian 'virtual' address to file offset."""
    off = addr - BASE_ADDR
    if off < 0 or off >= size:
        raise ValueError(f"Address 0x{addr:08X} -> file offset 0x{off:X} out of range")
    return off

def read_c_string(mm: mmap.mmap, off: int) -> bytes:
    """Read a null-terminated byte string starting at file offset off."""
    if off < 0 or off >= len(mm):
        return b""
    end = mm.find(b"\x00", off)
    if end == -1:
        end = len(mm)
    return mm[off:end]

def main(path: str):
    with open(path, "rb") as f, mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ) as mm:
        size = len(mm)

        # Locate dialog sequence table in file and read ENTRY_COUNT pointers
        table_off = addr_to_off(TABLE_ADDR, size)
        if table_off + ENTRY_COUNT * 4 > size:
            raise ValueError("Dialog sequence table extends beyond file size")

        seq_ptrs = struct.unpack_from("<" + "I"*ENTRY_COUNT, mm, table_off)

        for seq_idx, seq_addr in enumerate(seq_ptrs):
            try:
                seq_off = addr_to_off(seq_addr, size)
            except ValueError:
                print(f"Sequence {seq_idx}: INVALID sequence pointer 0x{seq_addr:08X}")
                continue

            print(f"Sequence {seq_idx}:")  # required: output the dialog sequence index
            line_i = 0
            while True:
                struct_off = seq_off + line_i * 8
                if struct_off + 8 > size:
                    print(f"  [end] (struct out of range at 0x{struct_off:X})")
                    break

                str_ptr, unknown = struct.unpack_from("<II", mm, struct_off)

                # Resolve string pointer; treat invalid pointers as end-of-sequence
                try:
                    str_off = addr_to_off(str_ptr, size)
                except ValueError:
                    print(f"  [{line_i:04}] unk=0x{unknown:08X} <invalid string ptr 0x{str_ptr:08X}>")
                    break

                raw = read_c_string(mm, str_off)

                # End marker: empty string (first byte is null => zero-length decoded)
                if len(raw) == 0:
                    break

                try:
                    text = raw.decode("shift_jis", errors="strict")
                except UnicodeDecodeError:
                    # Fall back to replacement to keep extraction going
                    text = raw.decode("shift_jis", errors="replace")

                print(f"  [{line_i:04}] unk=0x{unknown:08X} {text}")
                line_i += 1

if __name__ == "__main__":
    if len(sys.argv) != 2 or not os.path.isfile(sys.argv[1]):
        sys.exit(f"Usage: {os.path.basename(sys.argv[0])} <path/to/binary>")
    main(sys.argv[1])
