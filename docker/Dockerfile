FROM debian:trixie-slim

RUN groupadd --gid 1000 user \
    && useradd --create-home --home-dir /home/user --shell /bin/bash --uid 1000 --gid user user

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        python3 \
    && rm -rf /var/lib/apt/lists/*

USER user
# scripts/install_tools.py downloads the tools here
ENV PATH=/app/build/tools:$PATH
ENV DC_SDK=/sdk
ENV TBG_DISC=/tbgdisc
