FROM bitnamilegacy/dotnet-sdk:6 AS gdibuilder

COPY gdibuilder.patch /app/gdibuilder.patch

RUN apt-get update && apt-get install -y \
    clang \
    zlib1g-dev \
    libkrb5-dev \
    libtinfo5 \
    && rm -rf /var/lib/apt/lists/*

RUN cd /app \
    && git clone https://github.com/Sappharad/GDIbuilder.git \
    && cd GDIbuilder \
    && git checkout 43d0a81a55bbce67ac7f64d7e8ef101d11f26c9b \
    && git apply /app/gdibuilder.patch \
    && cd buildgdi \
    && dotnet publish --configuration Release -r linux-x64 --self-contained


FROM php:8.3-cli

RUN addgroup --gid 1000 user \
    && useradd --create-home --home-dir /home/user --shell /bin/bash --uid 1000 --gid user user

RUN apt-get update && apt-get install -y --no-install-recommends \
        wget \
    && rm -rf /var/lib/apt/lists/* \
    && wget -nv https://github.com/decompals/wibo/releases/latest/download/wibo-x86_64 -O /usr/local/bin/wibo \
    && chmod +x /usr/local/bin/wibo

# Install PHP extensions and Composer
ADD https://github.com/mlocati/docker-php-extension-installer/releases/latest/download/install-php-extensions /usr/local/bin/
RUN chmod uga+x /usr/local/bin/install-php-extensions && sync \
    && install-php-extensions \
    xdebug \
    zip \
    @composer \
    && rm /usr/local/bin/install-php-extensions

COPY --from=gdibuilder /app/GDIbuilder/buildgdi/bin/Release/net6.0/linux-x64/publish /opt/buildgdi
RUN ln -s /opt/buildgdi/buildgdi /usr/local/bin/buildgdi

USER user
RUN composer global require lhsazevedo/sh4objtest:v0.1.18

ENV PATH="${PATH}:/home/user/.composer/vendor/bin"
ENV DC_SDK=/sdk
ENV TBG_DISC=/tbgdisc

# Paths as seen by the SDK with wibo
ENV SHINOBI_DIR=Z:\\sdk\\shinobi
ENV KATANA_SDK_DIR=Z:\\sdk
ENV SHC_BIN=Z:\\sdk\\shc\\bin
ENV SHC_TMP=Z:\\app\\build\\tmp
ENV SHC_LIB=Z:\\sdk\\shc\\bin
ENV SHC_INC=Z:\\sdk\\shc\\include,Z:\\sdk\\shinobi\\include
