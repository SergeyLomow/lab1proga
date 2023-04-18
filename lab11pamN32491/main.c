#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <malloc.h>
#include <string.h>
#include <getopt.h>

int isdebug=0;
char *sign;

int read_file(const char* filename, const char* signature, int signature_len) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("Error opening file %s", filename);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char* buffer = malloc(file_size);
    if (!buffer) {
        perror("Error allocating memory");
        fclose(fp);
        return -1;
    }

    int readbytes = fread(buffer, 1, file_size, fp);
    fclose(fp);

    int found = 0;

    if (file_size<signature_len || readbytes<file_size)
    {
        return 0;
    }

    int i;
    for (i=0; i<file_size-signature_len; i++)
    {
        if (memcmp(buffer+i, signature, signature_len) == 0) {
            found = 1;
            break;
        }
    }

    free(buffer);

    if (found>0){
        if (isdebug) {
//            printf("", filename);
            fprintf(stderr, "found singnature %s in %s offset: 0x%x\n", sign, filename, i);
        } else {
            printf("%s \n", filename);
        }



    }
    return found;
}

char* join_strings(char** strings, int num_strings, char* delimiter) {
    // вычисляем общую длину результирующей строки
    int total_length = 0;
    for (int i = 0; i < num_strings; i++) {
        total_length += strlen(strings[i]);
        if (i != num_strings - 1) {
            total_length += strlen(delimiter);
        }
    }

    // выделяем память под результирующую строку
    char* result = (char*) malloc(total_length + 1);
    result[0] = '\0';

    // объединяем строки в результирующую строку
    for (int i = 0; i < num_strings; i++) {
        strcat(result, strings[i]);
        if (i != num_strings - 1) {
            strcat(result, delimiter);
        }
    }

    return result;
}

int main(int argc, char *argv[]) {
    int c;
    struct option long_options[] = {
            {"version", no_argument, 0, 'v'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
    };
    while ((c = getopt_long(argc, argv, "vh", long_options, NULL)) != -1) {
        switch (c) {
            case 'v':
                printf("Version 1.0.\nThe program searches for the specified signature in the subdirectory files.\nProbochkin Andrey Michailovich, N32491, var 1\n");
                return 0;
            case 'h':
                printf("Usage: %s [--version|-v] [--help|-h] <directory path> <signature i.e. 0xdeadbeef>\n", argv[0]);
                return 0;
            default:
                fprintf(stderr, "Error: unknown option '%c'. Use --help for usage information.\n", optopt);
                return 1;
        }
    }

    // Проверяем переменную окружения LAB11DEBUG
    char *debug = getenv("LAB11DEBUG");
    if (debug != NULL) {
        fprintf(stderr, "Debug mode is on\n");
        isdebug = 1;
    }

    if (argc != 3) {
        printf("Usage: program_name arg1 arg2\n");
        return 1;
    }

    int signature_len = (strlen(argv[2])-2)/2;
    char signature[signature_len];
    sign = argv[2];
    size_t i, j;


    // Проходим по строке и выделяем каждый байт
    for (i = 2, j = 0; i < strlen(argv[2]); i += 2, j++) {
        signature[j] = (argv[2][i] >= 'a' ? argv[2][i] - 'a' + 10 : argv[2][i] - '0') * 16;
        signature[j] += argv[2][i + 1] >= 'a' ? argv[2][i + 1] - 'a' + 10 : argv[2][i + 1] - '0';
    }

    char *start_path = (char*) malloc((strlen(argv[1])+1) * sizeof(char));
    strcpy(start_path, argv[1]);

    int n = 1;

    DIR **dir_array = (DIR **)malloc(n * sizeof(DIR*));
    if (dir_array == NULL) {
        printf("malloc err");
        return -1;
    }

    char **paths = (char**) malloc(n * sizeof(char*));
    if (paths == NULL) {
        printf("malloc err");
        return -1;
    }

    paths[0] = (char*) malloc((strlen(start_path)+1) * sizeof(char));
    if (paths[0] == NULL) {
        printf("malloc err");
        return -1;
    }
    strcpy(paths[0], start_path);
    free(start_path);


    struct dirent *dirent_ptr;
    DIR* current_dir;
    char* path = join_strings(paths, n, "/");
    dir_array[0] = opendir(path);
    free(path);
    current_dir = dir_array[0];
    while (n>=1)
    {
        dirent_ptr = readdir(current_dir);
        if (dirent_ptr==NULL)
        {
            n--;
            if (n==0)
            {
                free(dir_array[0]);
                free(dir_array);
                free(paths[0]);
                free(paths);
                continue;
            }

            free(dir_array[n]);
            DIR **new_dir = (DIR **)realloc(dir_array, n * sizeof(DIR*));
            if (new_dir == NULL) {
                printf("realloc err");
                return 1;
            }
            dir_array = new_dir;

            free(paths[n]);
            char **new_paths = (char**)realloc(paths, n * sizeof(char*));
            if (new_paths == NULL) {
                printf("realloc err");
                return 1;
            }
            paths = new_paths;

            current_dir = dir_array[n-1];

            continue;
        }

        if (dirent_ptr->d_type == DT_REG) {
            char *path_dir = join_strings(paths, n, "/");
            char *pathfile = (char*) malloc(strlen(path_dir) + 1 + strlen(dirent_ptr->d_name) + 1);
            if (pathfile == NULL) {
                printf("malloc err");
                return 1;
            }

            strcpy(pathfile, path_dir);
            strcat(pathfile, "/");
            strcat(pathfile, dirent_ptr->d_name);
            free(path_dir);

            read_file(pathfile, signature, signature_len);
            free(pathfile);
        }

        if (dirent_ptr->d_type == DT_DIR)
        {
            if (strcmp(dirent_ptr->d_name, ".") == 0 || strcmp(dirent_ptr->d_name, "..") == 0) {
                continue;
            }

            n++;
            char **new_paths = (char**)realloc(paths, n * sizeof(char*));
            if (new_paths == NULL) {
                printf("realloc err");
                return 1;
            }
            paths = new_paths;

            paths[n-1] = (char*) malloc((strlen(dirent_ptr->d_name)+1) * sizeof(char));
            if (paths[n-1] == NULL) {
                printf("malloc err");
                return 1;
            }
            strcpy(paths[n-1], dirent_ptr->d_name);

            DIR **new_dir = (DIR **)realloc(dir_array, n * sizeof(DIR*));
            if (new_dir == NULL) {
                printf("realloc err");
                return 1;
            }
            dir_array = new_dir;

            path = join_strings(paths, n, "/");
            dir_array[n-1] = opendir(path);
            free(path);
            current_dir = dir_array[n-1];
        }
    }
    return 0;
}