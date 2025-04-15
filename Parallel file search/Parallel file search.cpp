#include <iostream>

#define MAX_LINE_LENGTH 1024

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <substring>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    const char* substring = argv[2];

    FILE* file = NULL;
    if (fopen_s(&file, filename, "r") != 0) {
        perror("Error opening file");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    int line_number = 1;

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, substring)) {
            printf("[MATCH] %s (line %d): %s", filename, line_number, line);
        }
        line_number++;
    }

    fclose(file);
    return 0;

}
