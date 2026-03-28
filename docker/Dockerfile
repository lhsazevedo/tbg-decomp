FROM php:8.3-cli

RUN addgroup --gid 1000 user \
    && useradd --create-home --home-dir /home/user --shell /bin/bash --uid 1000 --gid user user

RUN apt-get update && apt-get install -y --no-install-recommends \
        wget \
    && rm -rf /var/lib/apt/lists/* \
    && wget -nv https://github.com/decompals/wibo/releases/latest/download/wibo-x86_64 -O /usr/local/bin/wibo \
    && chmod +x /usr/local/bin/wibo \
    && wget -nv https://github.com/lhsazevedo/gdworkbench/releases/download/v1.0.0-beta.5/gdworkbench_1.0.0-beta.5_linux_amd64.tar.gz -O /tmp/gdworkbench.tar.gz \
    && tar -xzf /tmp/gdworkbench.tar.gz -C /usr/local/bin/ gdworkbench \
    && rm /tmp/gdworkbench.tar.gz \
    && chmod +x /usr/local/bin/gdworkbench

# Install PHP extensions and Composer
ADD https://github.com/mlocati/docker-php-extension-installer/releases/latest/download/install-php-extensions /usr/local/bin/
RUN chmod uga+x /usr/local/bin/install-php-extensions && sync \
    && install-php-extensions \
    xdebug \
    zip \
    @composer \
    && rm /usr/local/bin/install-php-extensions

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
