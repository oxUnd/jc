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

/**
 * Replace all occurrences of a regex pattern in a string with support for capture groups
 * 
 * @param input The input string
 * @param pattern The regex pattern to search for
 * @param replacement The replacement string (supports $1, $2, etc. for capture groups)
 * @return A new dynamically allocated string with replacements, or NULL on error
 *         (Caller is responsible for freeing the returned string)
 */
char *regex_replace(const char *input, const char *pattern, const char *replacement) {
    if (!input || !pattern || !replacement) {
        return NULL;
    }

    regex_t regex;
    int ret;

    // Compile the regex pattern
    ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NEWLINE);
    if (ret != 0) {
        char error_buf[1024];
        regerror(ret, &regex, error_buf, sizeof(error_buf));
        fprintf(stderr, "Regex compilation error: %s\n", error_buf);
        return NULL;
    }

    // Get the number of capture groups
    size_t num_groups = regex.re_nsub + 1; // +1 for the entire match

    // Find the first match to determine required buffer size
    regmatch_t *matches = (regmatch_t *)malloc(num_groups * sizeof(regmatch_t));
    if (!matches) {
        regfree(&regex);
        return NULL;
    }

    const char *current = input;
    size_t output_size = strlen(input) + 1; // Start with the original size

    // Calculate the required output buffer size
    while ((ret = regexec(&regex, current, num_groups, matches, 0)) == 0) {
        // Calculate the length of the replacement with capture groups
        size_t repl_len = 0;
        const char *r = replacement;
        while (*r) {
            if (*r == '$' && *(r + 1) >= '0' && *(r + 1) <= '9') {
                // Handle capture group reference $1, $2, etc.
                int group_num = *(r + 1) - '0';
                if (group_num < num_groups && 
                        matches[group_num].rm_so != -1 && 
                        matches[group_num].rm_eo != -1) {
                    repl_len += matches[group_num].rm_eo - matches[group_num].rm_so;
                }
                r += 2; // Skip $ and digit
            } else {
                repl_len++; // Regular character
                r++;
            }
        }

        // Update output size
        output_size += repl_len - (matches[0].rm_eo - matches[0].rm_so);
        current += matches[0].rm_eo;
    }

    // Allocate output buffer
    char *output = (char *)malloc(output_size);
    if (!output) {
        free(matches);
        regfree(&regex);
        return NULL;
    }

    // Perform the actual replacement with capture groups
    current = input;
    char *out_ptr = output;
    size_t input_len = strlen(input);

    while ((ret = regexec(&regex, current, num_groups, matches, 0)) == 0) {
        // Copy text before match
        size_t prefix_len = matches[0].rm_so;
        strncpy(out_ptr, current, prefix_len);
        out_ptr += prefix_len;

        // Process replacement string with capture groups
        const char *r = replacement;
        while (*r) {
            if (*r == '$' && *(r + 1) >= '0' && *(r + 1) <= '9') {
                // Handle capture group reference $1, $2, etc.
                int group_num = *(r + 1) - '0';
                if (group_num < num_groups && 
                        matches[group_num].rm_so != -1 && 
                        matches[group_num].rm_eo != -1) {
                    size_t group_len = matches[group_num].rm_eo - matches[group_num].rm_so;
                    strncpy(out_ptr, current + matches[group_num].rm_so, group_len);
                    out_ptr += group_len;
                }
                r += 2; // Skip $ and digit
            } else {
                // Copy regular character
                *out_ptr++ = *r++;
            }
        }

        // Move current pointer past the match
        current += matches[0].rm_eo;
    }

    // Copy any remaining text after the last match
    if (current < input + input_len) {
        strcpy(out_ptr, current);
        out_ptr += strlen(current);
    }

    // Null-terminate the output string
    *out_ptr = '\0';

    // Free resources
    free(matches);
    regfree(&regex);

    return output;
}
