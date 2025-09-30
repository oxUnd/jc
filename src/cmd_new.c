#include "jc.h"
#include "utils.h"

// Template contents embedded in the code
static const char *configure_ac_template = 
"AC_PREREQ([2.69])\n"
"AC_INIT([%s], [1.0.0], [support@example.com])\n"
"AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])\n"
"AC_CONFIG_SRCDIR([src/main.c])\n"
"AC_CONFIG_HEADERS([config.h])\n"
"\n"
"# Checks for programs\n"
"AC_PROG_CC\n"
"\n"
"# Checks for header files\n"
"AC_CHECK_HEADERS([stdlib.h string.h])\n"
"\n"
"# Checks for typedefs, structures, and compiler characteristics\n"
"AC_TYPE_SIZE_T\n"
"\n"
"# Checks for library functions\n"
"AC_FUNC_MALLOC\n"
"\n"
"AC_CONFIG_FILES([\n"
"    Makefile\n"
"    src/Makefile\n"
"])\n"
"\n"
"AC_OUTPUT\n";

static const char *makefile_am_template =
"SUBDIRS = src\n"
"\n"
"ACLOCAL_AMFLAGS = -I m4\n"
"\n"
"EXTRA_DIST = README.md\n";

static const char *src_makefile_am_template =
"bin_PROGRAMS = %s\n"
"\n"
"%s_SOURCES = main.c\n"
"%s_CFLAGS = -Wall -Wextra -std=c11 -g\n";

static const char *main_c_template =
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    printf(\"Hello from %s!\\n\");\n"
"    return 0;\n"
"}\n";

static const char *readme_template =
"# %s\n"
"\n"
"A C project created with jc.\n"
"\n"
"## Building\n"
"\n"
"```bash\n"
"jc build\n"
"```\n"
"\n"
"## Running\n"
"\n"
"```bash\n"
"jc run\n"
"```\n"
"\n"
"## Installing\n"
"\n"
"```bash\n"
"jc install\n"
"```\n";

static const char *autogen_sh_template =
"#!/bin/sh\n"
"autoreconf --install\n";

static const char *gitignore_template =
"# Automake/Autoconf\n"
"Makefile\n"
"Makefile.in\n"
"aclocal.m4\n"
"autom4te.cache/\n"
"compile\n"
"config.h\n"
"config.h.in\n"
"config.log\n"
"config.status\n"
"configure\n"
"depcomp\n"
"install-sh\n"
"missing\n"
"stamp-h1\n"
".deps/\n"
".dirstamp\n"
"\n"
"# Build artifacts\n"
"*.o\n"
"*.a\n"
"*.so\n"
"*.dylib\n"
"src/%s\n"
"\n"
"# Debug\n"
"*.dSYM/\n"
"core\n"
"vgcore.*\n";

// Convert project name to valid automake variable name (replace - with _)
static void to_automake_var(const char *name, char *output, size_t size) {
    size_t i;
    for (i = 0; i < size - 1 && name[i] != '\0'; i++) {
        output[i] = (name[i] == '-') ? '_' : name[i];
    }
    output[i] = '\0';
}

int cmd_new(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: jc new <project-name>\n");
        return 1;
    }

    const char *project_name = argv[1];
    
    // Validate project name
    if (strlen(project_name) == 0) {
        fprintf(stderr, "Error: Project name cannot be empty\n");
        return 1;
    }
    
    // Check if directory already exists
    if (directory_exists(project_name)) {
        fprintf(stderr, "Error: Directory '%s' already exists\n", project_name);
        return 1;
    }

    printf("Creating new project: %s\n", project_name);
    
    // Create automake-compatible variable name
    char am_var_name[256];
    to_automake_var(project_name, am_var_name, sizeof(am_var_name));

    // Create project directory
    if (create_directory(project_name) != 0) {
        fprintf(stderr, "Error: Failed to create project directory\n");
        return 1;
    }

    // Create src directory
    char src_dir[1024];
    snprintf(src_dir, sizeof(src_dir), "%s/src", project_name);
    if (create_directory(src_dir) != 0) {
        fprintf(stderr, "Error: Failed to create src directory\n");
        return 1;
    }

    // Create m4 directory for autoconf macros
    char m4_dir[1024];
    snprintf(m4_dir, sizeof(m4_dir), "%s/m4", project_name);
    create_directory(m4_dir);

    // Write configure.ac
    char configure_ac_path[1024];
    char configure_ac_content[4096];
    snprintf(configure_ac_path, sizeof(configure_ac_path), "%s/configure.ac", project_name);
    snprintf(configure_ac_content, sizeof(configure_ac_content), configure_ac_template, project_name);
    if (write_file(configure_ac_path, configure_ac_content) != 0) {
        fprintf(stderr, "Error: Failed to create configure.ac\n");
        return 1;
    }

    // Write Makefile.am
    char makefile_am_path[1024];
    snprintf(makefile_am_path, sizeof(makefile_am_path), "%s/Makefile.am", project_name);
    if (write_file(makefile_am_path, makefile_am_template) != 0) {
        fprintf(stderr, "Error: Failed to create Makefile.am\n");
        return 1;
    }

    // Write src/Makefile.am
    char src_makefile_am_path[1024];
    char src_makefile_am_content[4096];
    snprintf(src_makefile_am_path, sizeof(src_makefile_am_path), "%s/src/Makefile.am", project_name);
    snprintf(src_makefile_am_content, sizeof(src_makefile_am_content), 
             src_makefile_am_template, am_var_name, am_var_name, am_var_name);
    if (write_file(src_makefile_am_path, src_makefile_am_content) != 0) {
        fprintf(stderr, "Error: Failed to create src/Makefile.am\n");
        return 1;
    }

    // Write src/main.c
    char main_c_path[1024];
    char main_c_content[4096];
    snprintf(main_c_path, sizeof(main_c_path), "%s/src/main.c", project_name);
    snprintf(main_c_content, sizeof(main_c_content), main_c_template, project_name);
    if (write_file(main_c_path, main_c_content) != 0) {
        fprintf(stderr, "Error: Failed to create src/main.c\n");
        return 1;
    }

    // Write README.md
    char readme_path[1024];
    char readme_content[4096];
    snprintf(readme_path, sizeof(readme_path), "%s/README.md", project_name);
    snprintf(readme_content, sizeof(readme_content), readme_template, project_name);
    if (write_file(readme_path, readme_content) != 0) {
        fprintf(stderr, "Error: Failed to create README.md\n");
        return 1;
    }

    // Write autogen.sh
    char autogen_path[1024];
    snprintf(autogen_path, sizeof(autogen_path), "%s/autogen.sh", project_name);
    if (write_file(autogen_path, autogen_sh_template) != 0) {
        fprintf(stderr, "Error: Failed to create autogen.sh\n");
        return 1;
    }
    chmod(autogen_path, 0755);

    // Write .gitignore
    char gitignore_path[1024];
    char gitignore_content[4096];
    snprintf(gitignore_path, sizeof(gitignore_path), "%s/.gitignore", project_name);
    snprintf(gitignore_content, sizeof(gitignore_content), gitignore_template, am_var_name);
    if (write_file(gitignore_path, gitignore_content) != 0) {
        fprintf(stderr, "Warning: Failed to create .gitignore\n");
    }

    printf("\n✓ Project '%s' created successfully!\n\n", project_name);
    printf("Next steps:\n");
    printf("  cd %s\n", project_name);
    printf("  jc build\n");
    printf("  jc run\n");
    printf("\n");

    return 0;
}
