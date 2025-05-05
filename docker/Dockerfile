FROM bitnami/dotnet-sdk:8 AS gdibuilder-builder

# COPY gdibuilder.patch /app/gdibuilder.patch

RUN apt-get update && apt-get install -y \
    clang \
    zlib1g-dev \
    libkrb5-dev \
    libtinfo5 \
    wget \
    && rm -rf /var/lib/apt/lists/*

RUN cd /app \
    && wget -O gdibuilder.tar.gz \
        https://github.com/Sappharad/GDIbuilder/archive/refs/tags/v2.1.1.tar.gz \
    && tar -xzvf gdibuilder.tar.gz \
    && cd GDIbuilder-2.1.1/buildgdi \
    && sed -i '/<add key="dotnet-core".*\/>/d' nuget.config \
    && dotnet publish --configuration Release -r linux-x64 --self-contained

FROM php:8.4-cli

RUN addgroup --gid 1000 user \
    && useradd --create-home \
        --home-dir /home/user \
        --shell /bin/bash \
        --uid 1000 \
        --gid user user

RUN dpkg --add-architecture i386 \
    && apt-get update && apt-get install -y --no-install-recommends \
        wine \
        wine32 \
    && rm -rf /var/lib/apt/lists/*

# Install PHP extensions and Composer
ADD https://github.com/mlocati/docker-php-extension-installer/releases/latest/download/install-php-extensions /usr/local/bin/
RUN chmod uga+x /usr/local/bin/install-php-extensions && sync \
    && install-php-extensions \
    xdebug \
    zip \
    @composer \
    && rm /usr/local/bin/install-php-extensions

COPY --from=gdibuilder-builder \
    /app/GDIbuilder-2.1.1/buildgdi/bin/Release/net8.0/linux-x64/publish \
    /opt/buildgdi
RUN ln -s /opt/buildgdi/buildgdi /usr/local/bin/buildgdi

USER user
RUN composer global require lhsazevedo/sh4objtest:v0.1.9

ENV PATH="${PATH}:/home/user/.composer/vendor/bin"
ENV DC_SDK=/sdk
ENV TBG_DISC=/tbgdisc

# Paths as seen by the SDK with wine
ENV SHINOBI_DIR=Z:\\sdk\\shinobi
ENV KATANA_SDK_DIR=Z:\\sdk
ENV SHC_BIN=Z:\\sdk\\shc\\bin
ENV SHC_TMP=Z:\\tmp
ENV SHC_LIB=Z:\\sdk\\shc\\bin
ENV SHC_INC=Z:\\sdk\\shc\\include,Z:\\sdk\\shinobi\\include
