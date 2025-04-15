#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 1024
#define MAX_THREADS 1024

CRITICAL_SECTION print_lock;

typedef struct {
    char filepath[MAX_PATH];
    char substring[256];
} ThreadArgs;


DWORD WINAPI search_in_file_thread(LPVOID param) {
    ThreadArgs* args = (ThreadArgs*)param;
    FILE* file = NULL;

    if (fopen_s(&file, args->filepath, "r") != 0 || file == NULL) {
        EnterCriticalSection(&print_lock);
        fprintf(stderr, "Error opening file: %s\n", args->filepath);
        LeaveCriticalSection(&print_lock);
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    int line_number = 1;

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, args->substring)) {
            EnterCriticalSection(&print_lock);
            printf("[MATCH] %s (line %d): %s \n\n", args->filepath, line_number, line);
            LeaveCriticalSection(&print_lock);
        }
        line_number++;
    }

    fclose(file);
    return 0;
}


void search_directory(const char* dirpath, const char* substring, HANDLE* thread_handles, ThreadArgs* thread_args, int* thread_count) {
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
            search_directory(full_path, substring, thread_handles, thread_args, thread_count);  // рекурсивный вызов
        }
        else {
            // Проверка на расширение .txt
            const char* ext = strrchr(fd.cFileName, '.');
            if (ext && strcmp(ext, ".txt") == 0 && *thread_count < MAX_THREADS) {
                ThreadArgs* args = &thread_args[*thread_count];

                strcpy_s(args->filepath, sizeof(args->filepath), full_path);
                strcpy_s(args->substring, sizeof(args->substring), substring);

                thread_handles[*thread_count] = CreateThread(
                    NULL, 0, search_in_file_thread, args, 0, NULL
                );
                (*thread_count)++;
            }
        }
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <directory> <substring>\n", argv[0]);
        return 1;
    }

    const char* dirpath = argv[1];
    const char* substring = argv[2];

    HANDLE thread_handles[MAX_THREADS];
    ThreadArgs thread_args[MAX_THREADS];
    int thread_count = 0;

    InitializeCriticalSection(&print_lock);

    search_directory(dirpath, substring, thread_handles, thread_args, &thread_count);

    // Ожидаем завершения всех потоков
    WaitForMultipleObjects(thread_count, thread_handles, TRUE, INFINITE);

    for (int i = 0; i < thread_count; i++) {
        CloseHandle(thread_handles[i]);
    }

    DeleteCriticalSection(&print_lock);
    return 0;
}
