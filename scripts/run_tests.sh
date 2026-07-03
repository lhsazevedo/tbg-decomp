set -e

sh4objtest=sh4objtest

ASMSH_FLAGS="-define=UNIT_TESTING=1 -debug -cpu=sh4 -endian=little -sjis"

assemble() {
  local src_file="$1"
  local base_name=$(basename "$src_file" .src)
  local obj_file="build\\output\\${base_name}_src.obj"

  wibo "$SHC_BIN/asmsh.exe" $(echo "$src_file"| tr / '\\') -object="$obj_file" $ASMSH_FLAGS
}

compile() {
  local src_file="$1"
  local base_name=$(basename "$src_file" .c)
  local obj_file="build\\output\\${base_name}_c.obj"
  local asm_file="build\\output\\${base_name}_c.src"

  wibo "$SHC_BIN/shc.exe" $(echo "$src_file" | tr / '\\') -object="$obj_file" -sub=build/shc_testing.sub 

  # Generate ASM file, useful for debugging.
  wibo "$SHC_BIN/shc.exe" $(echo "$src_file" | tr / '\\') -code=asm -object="$asm_file" -sub=build/shc_testing.sub 
}

rm -rf build/output build/tmp
mkdir build/output build/tmp

# 012324
assemble  src/asm/decompiled/012324_peripheral_support.src
compile  src/012324_peripheral_support.c

# 014f44
assemble  src/asm/decompiled/014f54_text.src
compile  src/014f54_text.c

# 0100bc_sound
assemble  src/asm/decompiled/0100bc_sound.src
compile  src/0100bc_sound.c

# 010fe8_heap
assemble  src/asm/decompiled/010fe8_heap.src
compile  src/010fe8_heap.c

# 015ab8_title
assemble  src/asm/decompiled/015ab8_title.src
compile  src/015ab8_title.c

# 0193c8
assemble  src/asm/decompiled/0193c8_vm_menu.src
compile  src/0193c8_vm_menu.c

# 0207d4
assemble  src/asm/decompiled/0207d4.src
compile  src/0207d4.c

# 016c58_prompt
assemble  src/asm/decompiled/016c58_prompt.src
compile  src/016c58_prompt.c

# 012f44
assemble  src/asm/decompiled/012f44.src
compile  src/012f44.c

# 011120
assemble  src/asm/decompiled/011120_asset_queues.src
compile  src/011120_asset_queues.c

# 019e98
assemble  src/asm/decompiled/019e98_main_menu.src
compile  src/019e98_main_menu.c

# 019e98
assemble  src/asm/decompiled/016d2c_course_menu.src
compile  src/016d2c_course_menu.c

# 012504_input
assemble  src/asm/decompiled/012504_input.src
compile  src/012504_input.c

# 016bf4_demo_input
assemble  src/asm/decompiled/016bf4_demo_input.src
compile  src/016bf4_demo_input.c

# 01d290_album
assemble  src/asm/decompiled/01d290_album.src
compile  src/01d290_album.c

# 013ae8
assemble  src/asm/decompiled/013ae8.src
compile  src/013ae8.c

$sh4objtest suite -s tests.php "$@"
