#!/bin/bash

source .env

# Base mounts
MOUNTS="-v $PWD:/app \
    -v $SDK_PATH:/sdk"

# Add TBG_DISC_PATH mount if the variable is set
if [ -n "$TBG_DISC_PATH" ]; then
    MOUNTS="$MOUNTS -v $TBG_DISC_PATH:/tbgdisc"
fi

# Run docker command with mounts
docker run -it \
    $MOUNTS \
    -w /app \
    -u 1000 \
    -e TERM=$TERM \
    lhsazevedo/tbg-decomp bash
