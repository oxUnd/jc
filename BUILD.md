# Building jc

This document describes how to build and install the `jc` tool.

## Prerequisites

Before building, ensure you have the following tools installed:

- **autoconf** (>= 2.69)
- **automake** (>= 1.15)
- **make**
- **gcc** or **clang** (C11 support)
- **g++** or **clang++** (C++11 support, for tests)
- **Google Test** (optional, for running tests)

### Installing Prerequisites

#### macOS (using Homebrew)
```bash
brew install autoconf automake googletest
```

#### Ubuntu/Debian
```bash
sudo apt-get install autoconf automake build-essential libgtest-dev
```

#### Fedora/RHEL/CentOS
```bash
sudo dnf install autoconf automake gcc gcc-c++ gtest-devel
```

## Quick Build

```bash
./autogen.sh
./configure
make
sudo make install
```

## Detailed Build Steps

### 1. Generate Configuration Script

Run the `autogen.sh` script to generate the `configure` script:

```bash
./autogen.sh
```

This runs `autoreconf --install` to generate all necessary build files.

### 2. Configure the Build

```bash
./configure
```

Options:
- `--prefix=/custom/path` - Install to a custom location (default: `/usr/local`)
- `--enable-tests` - Enable building of test suite

Example:
```bash
./configure --prefix=$HOME/.local --enable-tests
```

### 3. Build

```bash
make
```

This compiles the `jc` binary in the `src/` directory.

### 4. Run Tests (Optional)

If you configured with `--enable-tests`:

```bash
make check
```

### 5. Install

```bash
sudo make install
```

Or without sudo if you configured with a user-writable prefix:
```bash
make install
```

## Development Build

For development, you might want to use the tool without installing:

```bash
./autogen.sh
./configure
make
# Use directly from src/
./src/jc --help
```

Or add to your PATH temporarily:
```bash
export PATH="$PWD/src:$PATH"
jc --help
```

## Cleaning Build Files

Remove compiled files:
```bash
make clean
```

Remove all generated files (back to fresh checkout):
```bash
make distclean
```

## Troubleshooting

### autogen.sh fails
- Ensure `autoconf` and `automake` are installed
- Try: `autoreconf --install --force`

### configure fails
- Check that all prerequisites are installed
- Read the error message in `config.log`

### make fails
- Ensure you have a C11-compatible compiler
- Check compiler error messages

### Tests fail to build
- Google Test must be installed
- Make sure you ran `./configure --enable-tests`
- On some systems, you may need to build and install gtest manually

### Installation permission denied
- Use `sudo make install`
- Or configure with `--prefix=$HOME/.local` and install without sudo
