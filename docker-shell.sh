#!/bin/bash

source .env

# Base mounts
MOUNTS="-v $PWD:/app \
    -v $SDK_PATH:/sdk"

# Add TBG_DISC_ROOT mount if the variable is set
if [ -n "$TBG_DISC_ROOT" ]; then
    MOUNTS="$MOUNTS -v $TBG_DISC_ROOT:/tbgdisc"
fi

# Add TBG_DISC_OUTPUT mount if the variable is set
if [ -n "$TBG_DISC_OUTPUT" ]; then
    MOUNTS="$MOUNTS -v $TBG_DISC_OUTPUT:/tbgoutput"
fi

# Run docker command with mounts
docker run --rm -it \
    $MOUNTS \
    -w /app \
    -u 1000 \
    -e TERM=$TERM \
    lhsazevedo/tbg-decomp bash
