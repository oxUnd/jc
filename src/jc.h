#ifndef JC_H
#define JC_H

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

// Command function prototypes
int cmd_new(int argc, char *argv[]);
int cmd_build(int argc, char *argv[]);
int cmd_run(int argc, char *argv[]);
int cmd_install(int argc, char *argv[]);
int cmd_bt(int argc, char *argv[]);
int cmd_clean(int argc, char *argv[]);
int cmd_add(int argc, char *argv[]);
int cmd_test(int argc, char *argv[]);

// Version info
#define JC_VERSION "1.0.0"

#endif // JC_H
