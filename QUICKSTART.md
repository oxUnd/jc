# Quick Start Guide

## Build and Install jc

```bash
# Generate build system
./autogen.sh

# Configure
./configure

# Build
make

# Install (requires sudo)
sudo make install

# Or install to custom location
./configure --prefix=$HOME/.local
make
make install
```

## Using jc

### 1. Create a New Project

```bash
jc new myproject
cd myproject
```

This creates a complete automake project structure with:
- `configure.ac` - Autoconf configuration
- `Makefile.am` - Top-level build rules
- `src/` - Source code directory
- `src/main.c` - Sample main program
- `autogen.sh` - Script to generate configure
- `.gitignore` - Git ignore rules
- `README.md` - Project documentation

### 2. Build Your Project

```bash
jc build
```

This command automatically:
1. Runs `autogen.sh` if `configure` doesn't exist
2. Runs `./configure` if `Makefile` doesn't exist
3. Runs `make` to build the project

No need to remember multiple commands!

### 3. Run Your Project

```bash
jc run
```

This finds and executes your built program. You can also pass arguments:

```bash
jc run --arg1 --arg2
```

### 4. Debug Crashes

If your program crashes:

```bash
jc bt
```

This automatically:
- Uses `lldb` on macOS or `gdb` on Linux
- Loads core dumps if available
- Shows backtrace to help you debug

Example debugging session:
```bash
$ jc run
Segmentation fault: 11

$ jc bt
Using lldb debugger...
Executable: src/myproject
Running with lldb...
----------------------------------------
(lldb) bt
* thread #1: tid = 0x1234, 0x0000000100000f50 myproject`main + 16
  * frame #0: 0x0000000100000f50 myproject`main + 16
    frame #1: 0x00007fff5fc5d7fd libdyld.dylib`start + 1
```

### 5. Install Your Project

```bash
# System-wide installation (needs sudo)
sudo jc install

# Or if configured with --prefix=$HOME/.local
jc install
```

## Complete Example

Create, build, and run a new project:

```bash
# Create project
jc new hello-world
cd hello-world

# Edit the source code
vim src/main.c

# Build it
jc build

# Run it
jc run

# If it crashes, debug it
jc bt
```

## Working with Existing Code

If you have an existing automake project, you can use `jc` commands directly:

```bash
cd my-existing-project
jc build  # Build the project
jc run    # Run the executable
```

## Tips

1. **Development workflow**: Just use `jc build && jc run` repeatedly
2. **Clean build**: Run `make clean` or `make distclean` when needed
3. **Add files**: Edit `src/Makefile.am` to add more source files
4. **Dependencies**: Edit `configure.ac` to check for libraries
5. **Project name with dashes**: Automatically converted (e.g., `my-project` → `my_project` in automake variables)

## Getting Help

```bash
jc help     # Show help
jc version  # Show version
jc --help   # Same as help
```

## Next Steps

- Read [BUILD.md](BUILD.md) for detailed build instructions
- Read [README.md](README.md) for full documentation
- Check out the automake manual: https://www.gnu.org/software/automake/manual/
- Check out the autoconf manual: https://www.gnu.org/software/autoconf/manual/
