# jc - A Modern C Project Management Tool

`jc` is a friendly command-line tool for managing C projects using automake. It provides modern, intuitive commands while leveraging the power of the automake/autoconf toolchain.

## Features

- 🚀 Quickly initialize C projects with proper automake structure
- 🔨 Simple build commands that handle configuration automatically
- ▶️  Easy project execution with debugging support
- 🐛 Integrated debugger support (lldb on macOS, gdb on Linux)
- 📦 Streamlined installation workflow

## Quick Start

See [QUICKSTART.md](QUICKSTART.md) for a quick start guide.

## Installation

See [BUILD.md](BUILD.md) for detailed build and installation instructions.

Quick install:
```bash
./autogen.sh
./configure
make
sudo make install
```

## Usage

### Create a new project
```bash
jc new myproject
cd myproject
```

### Build the project
```bash
jc build
```

This command automatically:
- Runs `autogen.sh` if needed
- Runs `./configure` if needed
- Runs `make`

### Run the project
```bash
jc run
```

Runs the built executable with any additional arguments:
```bash
jc run --arg1 --arg2
```

### Install the project
```bash
jc install
# or with sudo if needed
sudo jc install
```

### Debug with backtrace
```bash
jc bt
```

When your program crashes, use this command to debug it. It will:
- Use `lldb` on macOS
- Use `gdb` on Linux/Windows
- Load core dumps if available
- Show backtrace automatically

## Examples

Create and run a new project:
```bash
jc new hello-world
cd hello-world
jc build
jc run
```

Debug a crashing program:
```bash
jc run  # Program crashes
jc bt   # Automatically debug with lldb/gdb
```

## Project Structure

When you create a new project with `jc new`, it generates:
```
myproject/
├── configure.ac          # Autoconf configuration
├── Makefile.am           # Top-level automake file
├── autogen.sh            # Script to generate configure
├── src/
│   ├── Makefile.am       # Source directory automake file
│   └── main.c            # Main program source
├── README.md             # Project documentation
└── .gitignore            # Git ignore file
```

## Requirements

- autoconf (>= 2.69)
- automake (>= 1.15)
- C compiler with C11 support (gcc/clang)
- lldb (macOS) or gdb (Linux) for debugging

## Development

This project uses:
- **Language**: C11
- **Build System**: automake/autoconf
- **Testing**: Google Test (gtest)

To build with tests:
```bash
./configure --enable-tests
make
make check
```

## License

MIT License - see LICENSE file for details

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.