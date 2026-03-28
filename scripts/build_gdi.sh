#!/bin/bash
set -e

script_name=$0
script_path=$(dirname "$0")

echo "Checking original disc files..."
$script_path/check_disc.sh

echo "Building the GDI data track..."

if [ ! -f build/output/tbg.bin ]; then
    echo "The game binary was not found at build/output/tbg.bin"
    echo "Please build the game binary first using make"
    exit 1
fi

cp build/output/tbg.bin /tbgdisc/root/1ST_READ.BIN
rm -f /tbgoutput/tbg_t3.cooked.bin
gdworkbench track build --output /tbgoutput/tbg_t3.cooked.bin /tbgdisc

echo "Data track built successfully at /tbgoutput"
