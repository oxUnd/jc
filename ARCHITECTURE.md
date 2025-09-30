# Architecture

This document describes the internal architecture of the `jc` tool.

## Project Structure

```
jc/
├── src/              # Source code
│   ├── main.c        # Main entry point and command dispatcher
│   ├── jc.h          # Main header with command function declarations
│   ├── utils.c       # Utility functions
│   ├── utils.h       # Utility function declarations
│   ├── cmd_new.c     # Implementation of 'jc new' command
│   ├── cmd_build.c   # Implementation of 'jc build' command
│   ├── cmd_run.c     # Implementation of 'jc run' command
│   ├── cmd_install.c # Implementation of 'jc install' command
│   ├── cmd_bt.c      # Implementation of 'jc bt' command
│   └── templates/    # Templates for new projects
│       ├── configure.ac.template
│       ├── Makefile.am.template
│       ├── src_Makefile.am.template
│       ├── main.c.template
│       ├── README.md.template
│       └── autogen.sh.template
├── tests/            # Unit tests (Google Test)
│   ├── test_main.cpp
│   └── test_utils.cpp
├── configure.ac      # Autoconf configuration
├── Makefile.am       # Top-level automake file
└── autogen.sh        # Script to generate configure

```

## Command Flow

### Main Entry Point (`main.c`)

1. Parse command-line arguments
2. Dispatch to appropriate command handler
3. Return exit code

```
main(argc, argv)
  │
  ├─> "new"     → cmd_new()
  ├─> "build"   → cmd_build()
  ├─> "run"     → cmd_run()
  ├─> "install" → cmd_install()
  ├─> "bt"      → cmd_bt()
  ├─> "help"    → print_usage()
  └─> "version" → print_version()
```

## Command Implementations

### cmd_new (Create New Project)

**File**: `src/cmd_new.c`

**Process**:
1. Validate project name
2. Check if directory exists
3. Create project directory structure
4. Generate files from embedded templates:
   - `configure.ac` - Autoconf configuration
   - `Makefile.am` - Root automake file
   - `src/Makefile.am` - Source directory automake file
   - `src/main.c` - Sample C program
   - `README.md` - Project documentation
   - `autogen.sh` - Build system generator
   - `.gitignore` - Git ignore rules

**Key Features**:
- Converts project names with dashes to underscores for automake variables
  (e.g., `my-project` → `my_project_SOURCES`)
- Templates are embedded in the C code for portability
- Creates proper directory structure

### cmd_build (Build Project)

**File**: `src/cmd_build.c`

**Process**:
1. Check if current directory is an automake project (has `configure.ac`)
2. If `configure` doesn't exist:
   - Run `autogen.sh` or `autoreconf --install`
3. If `Makefile` doesn't exist:
   - Run `./configure`
4. Run `make` to build

**Design Philosophy**:
- Idempotent: Can be run multiple times safely
- Smart: Only regenerates what's necessary
- Friendly: Shows clear progress messages

### cmd_run (Run Project)

**File**: `src/cmd_run.c`

**Process**:
1. Check if project is built (Makefile exists)
2. If not built, automatically call `cmd_build()`
3. Find executable in `src/` or current directory
4. Execute with any additional arguments
5. Detect crashes and suggest using `jc bt`

**Key Features**:
- Automatically builds if necessary
- Passes through command-line arguments to the program
- Detects common crash signals (SIGSEGV, SIGABRT)
- Provides helpful debugging suggestions

### cmd_install (Install Project)

**File**: `src/cmd_install.c`

**Process**:
1. Check if project is built
2. If not, build it first
3. Run `make install`
4. Provide helpful error messages if permission denied

**Key Features**:
- Automatic build if necessary
- Suggests using `sudo` if permission is denied

### cmd_bt (Backtrace/Debug)

**File**: `src/cmd_bt.c`

**Process**:
1. Find the executable
2. Detect operating system:
   - macOS: Use `lldb`
   - Linux: Use `gdb`
3. Check for core dump
4. If core dump exists:
   - Load debugger with core dump
5. If no core dump:
   - Run program under debugger
   - Automatically show backtrace on crash

**Platform-Specific**:
- Uses preprocessor directives to detect OS
- Different command syntax for lldb vs gdb
- Provides usage instructions for each debugger

## Utility Functions

**File**: `src/utils.c`

Key utilities:
- `create_directory()` - Create directory with error handling
- `file_exists()` - Check if file exists
- `directory_exists()` - Check if directory exists
- `write_file()` - Write content to file
- `read_file()` - Read entire file into memory
- `copy_file()` - Copy file from source to destination
- `execute_command()` - Execute shell command with output
- `execute_command_quiet()` - Execute shell command silently
- `is_automake_project()` - Check if current directory has configure.ac
- `find_executable()` - Find executable in directory
- `get_template_path()` - Locate template files (for future use)

## Build System

### configure.ac

Key features:
- Checks for required tools (autoconf, automake, make)
- Detects available debugger (lldb or gdb)
- Optional test suite support (`--enable-tests`)
- Checks for required headers and functions
- Platform detection (macOS vs Linux)

### Makefile.am Files

**Top-level** (`Makefile.am`):
- Defines subdirectories to build
- Conditionally includes tests if enabled

**Source directory** (`src/Makefile.am`):
- Defines `jc` as a binary program
- Lists all source files
- Sets compiler flags (C11, warnings, debug info)
- Specifies template files to install

**Tests directory** (`tests/Makefile.am`):
- Only active if `--enable-tests` is used
- Links with Google Test
- Defines test programs

## Testing

**Framework**: Google Test (gtest)

**Test Files**:
- `test_main.cpp` - Test environment setup
- `test_utils.cpp` - Tests for utility functions

**Test Coverage**:
- Directory creation
- File operations (read, write, copy)
- File existence checks
- Command execution
- Automake project detection

**Running Tests**:
```bash
./configure --enable-tests
make check
```

## Design Principles

1. **User-Friendly**: Clear messages, automatic handling of common cases
2. **Robust**: Check for errors, provide helpful suggestions
3. **Portable**: Works on macOS, Linux (Windows support via WSL)
4. **Standard**: Uses standard automake/autoconf conventions
5. **Self-Contained**: Minimal dependencies (just standard Unix tools)
6. **Idempotent**: Commands can be run multiple times safely

## Future Enhancements

Possible future improvements:

1. **Template System**: 
   - Support for custom project templates
   - Multiple project types (library, application, etc.)

2. **Configuration**:
   - User-level configuration file (`~/.jcrc`)
   - Project-level configuration

3. **Package Management**:
   - Support for common dependency managers
   - Auto-detection of required libraries

4. **IDE Integration**:
   - Generate `.vscode/` or `.idea/` configurations
   - Compilation database for clangd

5. **Enhanced Debugging**:
   - Memory leak detection (valgrind integration)
   - Performance profiling helpers

6. **Cross-Compilation**:
   - Easy cross-compilation support
   - Target platform selection

7. **Documentation Generation**:
   - Integrate with Doxygen
   - Generate man pages

## Contributing

When adding new features:

1. Add command implementation in `src/cmd_<name>.c`
2. Declare function in `src/jc.h`
3. Add dispatcher case in `src/main.c`
4. Add tests in `tests/test_<name>.cpp`
5. Update documentation (README.md, QUICKSTART.md)
6. Test on multiple platforms if possible
