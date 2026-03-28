# Project Setup
Note: This guide assumes the use of the recommended Docker setup for building
and testing. If you are an advanced user and prefer to set up your own
environment, you can refer to the Dockerfile for guidance. A detailed guide for
manual setup will be provided in the future.

## General Requirements
- Docker.
- Dreamcast SDK (Sega Library) Ver.1.55J (Google is your friend).

## Setup

### SDK Structure
Ensure your SDK files are organized as follows:
```
├── bin (From disc Vol.1 dc_sdk/bin)
│   ├── binadj.exe
│   └── ...
├── shc (From disc Vol.2)
│   ├── bin
│   ├── include
│   └── lib
└── shinobi (From disc Vol.2)
   ├── driver
   ├── include
   ├── lib
   └── sample
```

### Environment Setup with Docker
1. Duplicate the `.env.example` file to create `.env`:
   ```bash
   cp .env.example .env
   ```
2. Edit the `.env` file to update the `SDK_PATH` variable with the absolute path of the SDK files and save the changes.

3. *(Optional)* If rebuilding the disc image, uncomment and set the `TBG_DISC_ROOT` variable in `.env` with the absolute path of the extracted disc image files.

4. Start a bash shell inside the `/app` folder:
   ```bash
   ./docker_shell.sh
   ```

## Build and Test

### Building the Binary
Run the following command in the Docker shell:
```bash
make
```
Upon success, the output will include:
```
Project built :)
```

### Running Unit Tests
After building, run the test suite to validate the code:
```bash
./scripts/run_tests.sh
```
This uses the custom Hitachi SH4 emulator and PHP-based testing framework to ensure decompiled code behaves as expected.

### *(Optional)* Rebuilding the Docker Image
If the Docker image is outdated or causing issues, rebuild it using the `docker` folder:
```bash
docker build -t lhsazevedo/tbg-decomp ./docker
```
