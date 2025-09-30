bin_PROGRAMS = jc

jc_SOURCES = \
    main.c \
    cmd_new.c \
    cmd_add.c \
    cmd_build.c \
    cmd_run.c \
    cmd_install.c \
    cmd_clean.c \
    cmd_bt.c \
    cmd_test.c \
    utils.c \
    jc.h \
    utils.h

jc_CFLAGS = -Wall -Wextra -std=c11 -g -O2

# Install templates
templatesdir = $(datadir)/jc/templates
dist_templates_DATA = \
    templates/configure.ac.template \
    templates/Makefile.am.template \
    templates/src_Makefile.am.template \
    templates/main.c.template \
    templates/README.md.template \
    templates/autogen.sh.template
