# Project Setup

The game builds and its unit tests run natively on Linux, macOS and Windows.

## Requirements
- Python 3.
- Dreamcast SDK (Sega Library) Ver.1.55J (Google is your friend).
- Docker, only for the Docker setup.

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

The linker can't handle `-` in a path. The build passes it the SDK's path
relative to the repo, so only the folders that path names count: put the SDK
anywhere, but don't call its folder something like `dc-sdk-155j`.

## Native Setup
1. Download the build tools into `build/tools/`:
   ```bash
   python3 scripts/install_tools.py
   ```
   Re-run it after pulling a change that bumps a tool version. It skips a tool
   with no release for your platform: Windows doesn't need wibo.

2. Point `DC_SDK` at the SDK:
   ```bash
   export DC_SDK=/path/to/sdk          # Linux, macOS
   $env:DC_SDK = "C:\path\to\sdk"      # Windows (PowerShell)
   ```

On Windows, run the scripts through `python` (`python build.py` instead of
`./build.py`).

## Docker Setup
1. Duplicate the `.env.example` file to create `.env`:
   ```bash
   cp .env.example .env
   ```
2. Edit the `.env` file to update the `SDK_PATH` variable with the absolute path of the SDK files and save the changes.

3. *(Optional)* If rebuilding the disc image, uncomment and set the `TBG_DISC_ROOT` variable in `.env` with the absolute path of the extracted disc image files.
   Rebuilding the disc image only works in Docker for now.

4. Start a bash shell inside the `/app` folder:
   ```bash
   ./docker-shell.sh
   ```

5. Download the build tools (wibo, sh4objtest, gdworkbench) into `build/tools/`:
   ```bash
   python3 scripts/install_tools.py
   ```
   Re-run it after pulling a change that bumps a tool version.

### *(Optional)* Rebuilding the Docker Image
If the Docker image is outdated or causing issues, rebuild it using the `docker` folder:
```bash
docker build -t lhsazevedo/tbg-decomp ./docker
```

## Build and Test

### Building the Binary
```bash
./build.py
```
Upon success, the output will include:
```
Project built :)
```

`./build.py matching` builds the game from the original asm instead and checks
it against the original binary. See `./build.py --help` for the other targets
and options.

### Running Unit Tests
After building, run the test suite to validate the code:
```bash
python3 scripts/run_tests.py
```
This uses the custom Hitachi SH4 emulator and PHP-based testing framework to ensure decompiled code behaves as expected.
