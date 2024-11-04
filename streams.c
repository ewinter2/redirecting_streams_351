#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>


#define WORD_BUFFER_SIZE 1000
#define DIRECTORIES_ARRAY_SIZE 1000
#define PATH_SIZE 4096
#define PARSED_COMMAND_SIZE 128


void add_char_to_string(char* str, char c) {
    int len = strlen(str);
    str[len] = c;
    str[len + 1] = '\0';
}

void break_into_words(char* input, char* words[], const char* delimiters) {
    char * current_char = input;
    int word_count = 0;

    char word_so_far[WORD_BUFFER_SIZE];

    while (*current_char != '\0') {
        if (strchr(delimiters, *current_char) != NULL) {
            if (strlen(word_so_far) > 0) {
                words[word_count++] = strdup(word_so_far);
                word_so_far[0] = '\0';
            }
        } else {
            add_char_to_string(word_so_far, *current_char);
        }
        current_char++;
    }

    if (strlen(word_so_far) > 0) {
        words[word_count++] = strdup(word_so_far);
    }

    words[word_count] = NULL;
}

bool find_absolute_path(const char* no_path, char* with_path) {
    char *directories[DIRECTORIES_ARRAY_SIZE];
    char path_copy[PATH_SIZE];
    //for error checking if PATH is not set
    const char* env_path = getenv("PATH");

    if (!env_path) {
        fprintf(stderr, "PATH environment variable not set.\n");
        return false;
    }

    strncpy(path_copy, getenv("PATH"), sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';

    break_into_words(path_copy, directories, ":");

    printf("Parsed PATH directories:\n");
    for (int i = 0; directories[i] != NULL; i++) {
        printf("Directory %d: %s\n", i, directories[i]);
    }

    for (int ix = 0; directories[ix] != NULL; ix++) {
        snprintf(with_path, PATH_SIZE, "%s/%s", directories[ix], no_path);

        if (access(with_path, X_OK) == 0) {
            for (int jx = 0; jx < ix; jx++) {
                free(directories[jx]);
            }
            return true;
        }
    }

    return false;
}

void redirect_stream(const char *inp, const char *cmd, const char *out) {
    char *args[PARSED_COMMAND_SIZE];
    char with_path[PATH_SIZE];

    break_into_words((char *)cmd, args, " ");

    if (!find_absolute_path(args[0], with_path)) {
        fprintf(stderr, "Command not found: %s\n", args[0]);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
        if (strcmp(inp, "-" ) != 0) {
            int inp_fd = open(inp, O_RDONLY);
            if (inp_fd == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(inp_fd, STDIN_FILENO);
            close(inp_fd);
        }

        if (strcmp(out, "-") != 0) {
            int out_fd = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (out_fd == -1) {
                perror("Output file open failed");
                exit(EXIT_FAILURE);
            }
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        if (execv(with_path, args) == -1) {
            perror("exec failed");
            exit(EXIT_FAILURE);
        }
    }
    else {
        int status;
        waitpid(pid, &status, 0);
    }

    for (int ix = 0; args[ix] != NULL; ix++) {
        free(args[ix]);
    }
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <inp> <cmd> <out>\n", argv[0]);
        return 1;
    }

    redirect_stream(argv[1], argv[2], argv[3]);

    return 0;
}