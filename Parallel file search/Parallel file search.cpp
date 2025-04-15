#include <windows.h>
#include <stdio.h>

#define MAX_LINE_LENGTH 1024


void search_in_file(const char* filepath, const char* substring) {
    FILE* file = NULL;
    if (fopen_s(&file, filepath, "r") != 0 || file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filepath);
        return;
    }

    char line[MAX_LINE_LENGTH];
    int line_number = 1;

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, substring)) {
            printf("[MATCH] %s (line %d): %s \n\n", filepath, line_number, line);
        }
        line_number++;
    }

    fclose(file);
}


void search_directory(const char* dirpath, const char* substring) {
    char search_path[MAX_PATH];
    snprintf(search_path, MAX_PATH, "%s\\*", dirpath);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search_path, &fd);

    if (hFind == INVALID_HANDLE_VALUE) return;

    do
    {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;

        char full_path[MAX_PATH];
        snprintf(full_path, MAX_PATH, "%s\\%s", dirpath, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            search_directory(full_path, substring);  // рекурсивный вызов
        }
        else {
            // Проверка на расширение .txt
            const char* ext = strrchr(fd.cFileName, '.');
            if (ext && strcmp(ext, ".txt") == 0) {
                search_in_file(full_path, substring);
            }
        }

    } while (FindNextFileA(hFind, &fd));
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <directory> <substring>\n", argv[0]);
        return 1;
    }

    const char* dirpath = argv[1];
    const char* substring = argv[2];

    search_directory(dirpath, substring);
    return 0;
}
