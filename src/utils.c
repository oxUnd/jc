#include "utils.h"
#include <errno.h>
#include <limits.h>
#include <dirent.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

int create_directory(const char *path) {
    struct stat st = {0};
    
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) != 0) {
            perror("mkdir");
            return -1;
        }
    }
    return 0;
}

int file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

int directory_exists(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        return 1;
    }
    return 0;
}

int copy_file(const char *src, const char *dst) {
    FILE *source = fopen(src, "r");
    if (!source) {
        return -1;
    }

    FILE *dest = fopen(dst, "w");
    if (!dest) {
        fclose(source);
        return -1;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        if (fwrite(buffer, 1, bytes, dest) != bytes) {
            fclose(source);
            fclose(dest);
            return -1;
        }
    }

    fclose(source);
    fclose(dest);
    return 0;
}

int write_file(const char *path, const char *content) {
    FILE *file = fopen(path, "w");
    if (!file) {
        perror("fopen");
        return -1;
    }

    if (fputs(content, file) == EOF) {
        perror("fputs");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

char *read_file(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(size + 1);
    if (!content) {
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(content, 1, size, file);
    content[read_size] = '\0';
    
    fclose(file);
    return content;
}

int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Command failed with exit code: %d\n", ret);
        return -1;
    }
    return 0;
}

int execute_command_quiet(const char *cmd) {
    int ret = system(cmd);
    return ret == 0 ? 0 : -1;
}

char *get_template_path(const char *template_name) {
    static char path[PATH_MAX];
    
    // Try to get from environment variable first (for development)
    const char *jc_data = getenv("JC_DATA_DIR");
    if (jc_data) {
        snprintf(path, sizeof(path), "%s/templates/%s", jc_data, template_name);
        if (file_exists(path)) {
            return path;
        }
    }
    
    // Try installed location
    #ifdef PREFIX
    snprintf(path, sizeof(path), "%s/share/jc/templates/%s", PREFIX, template_name);
    if (file_exists(path)) {
        return path;
    }
    #endif
    
    // Try common installation locations
    const char *install_prefixes[] = {
        "/usr/local",
        "/usr",
        NULL
    };
    
    for (int i = 0; install_prefixes[i] != NULL; i++) {
        snprintf(path, sizeof(path), "%s/share/jc/templates/%s", install_prefixes[i], template_name);
        if (file_exists(path)) {
            return path;
        }
    }
    
    // Try local development location
    snprintf(path, sizeof(path), "src/templates/%s", template_name);
    if (file_exists(path)) {
        return path;
    }
    
    return NULL;
}

int is_automake_project(void) {
    return file_exists("configure.ac") || file_exists("configure.in");
}

int find_executable(const char *dir, char *output, size_t output_size) {
    DIR *d = opendir(dir);
    if (!d) {
        return -1;
    }
    
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
        
        struct stat st;
        if (stat(path, &st) == 0 && S_ISREG(st.st_mode) && (st.st_mode & S_IXUSR)) {
            snprintf(output, output_size, "%s", path);
            closedir(d);
            return 0;
        }
    }
    
    closedir(d);
    return -1;
}
