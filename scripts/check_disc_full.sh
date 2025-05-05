#!/bin/bash

# This script was used to before rebuild mode was added to GDIBuilder.

if [ -z "$TBG_DISC" ]; then
    echo "============="
    echo " Disc: ERROR"
    echo "============="
    echo "Please set the TBG_DISC environment variable."
    exit 1
fi

TBG_DISC_NOT_FOUND=""

check_disc_dir() {
    if [ ! -d $1 ]; then
        TBG_DISC_NOT_FOUND="$TBG_DISC_NOT_FOUND $1"
    fi
}

check_disc_file() {
    if [ ! -f $1 ]; then
        TBG_DISC_NOT_FOUND="$TBG_DISC_NOT_FOUND $1"
    fi
}

check_disc_dir $TBG_DISC/
check_disc_file $TBG_DISC/IP.BIN
check_disc_file $TBG_DISC/root/SYSTEM/BUS_FONT.FFF

if [ ! -z "$TBG_DISC_NOT_FOUND" ]; then
    echo "==================="
    echo " Disc files: ERROR"
    echo "==================="
    echo "Missing files/directories in the original disc, which are needed for rebuilding the disc image."
    echo "Not found: $TBG_DISC_NOT_FOUND"
    echo "Tip: If using docker, ensure the volume is mounted correctly at $TBG_DISC."
    exit 1
fi

echo "=========="
echo " Disc: OK"
echo "=========="
