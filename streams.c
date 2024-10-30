#include <stdio.h>

void redirect_stream(const char *inp, const char *cmd, const char *out) {
    
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <inp> <cmd> <out>\n", argv[0]);
        return 1;
    }

    redirect_stream(argv[1], argv[2], argv[3]);

    return 0;
}