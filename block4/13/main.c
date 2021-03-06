#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <locale.h>
#include <malloc.h>
#include <winsock2.h>
#include <windows.h>

size_t sizeOfFile(const char *file_name) {
    size_t fileSize = 0;
    FILE *f = fopen(file_name, "rb");
    if (f == NULL)
        fileSize = -1;
    else {
        fseek(f, 0, SEEK_END);
        fileSize = ftell(f);

        fclose(f);
    }
    return fileSize;
}

void commandShow(char *file) {
    char temp[10];
    FILE *f = fopen(file, "rb");

    fread(temp, 1, 10, f);
    char *bites = (char *) malloc(sizeof(char) * sizeOfFile(file));
    fread(bites, 1, sizeOfFile(file) - 10, f);

    int i = 0;
    while (bites[i] != 0) {
        char name[5] = {bites[i], bites[i + 1], bites[i + 2], bites[i + 3], '\0'};
        printf("%s: ", name);

        i += 4;
        int size = bites[i] * 256 * 256 * 256 + bites[i + 1] * 256 * 256 + bites[i + 2] * 256 + bites[i + 3];

        i += 6;
        for (int j = 0; j < size; j++, i++)
            printf("%c", bites[i]);
        printf("\n");
    }
    fclose(f);
    free(bites);
}

void commandGet(char *file, char frame[5]) {
    char *bites = (char *) malloc(sizeof(char) * (sizeOfFile(file) + 1));
    FILE *f = fopen(file, "rb");
    fread(bites, 1, sizeOfFile(file), f);

    int i = 10;
    while (bites[i] != 0) {
        int size = 0;
        if (bites[i] == frame[0] && bites[i + 1] == frame[1] && bites[i + 2] == frame[2] && bites[i + 3] == frame[3]) {
            printf("%c", bites[i]);
            printf("%c", bites[i + 1]);
            printf("%c", bites[i + 2]);
            printf("%c: ", bites[i + 3]);

            i += 4;
            size = bites[i] * 256 * 256 * 256 + bites[i + 1] * 256 * 256 + bites[i + 2] * 256 + bites[i + 3];

            i += 6;
            for (int j = 0; j < size; i++, j++)
                printf("%c", bites[i]);
            break;
        } else {
            i += 4;
            int size = bites[i] * 256 * 256 * 256 + bites[i + 1] * 256 * 256 + bites[i + 2] * 256 + bites[i + 3];
            i += size + 6;
        }
    }
    fclose(f);
    free(bites);
}

void commandSet(char *file, char frame[5], char *value) {
    FILE *f = fopen(file, "rb");
    char *bites = (char *) malloc(sizeof(char) * sizeOfFile(file));
    fread(bites, 1, sizeOfFile(file), f);

    for (int i = 0; i < sizeOfFile(file) - 4; i++) {
        if (bites[i] == frame[0] && bites[i + 1] == frame[1] && bites[i + 2] == frame[2] && bites[i + 3] == frame[3]) {
            FILE *nf = fopen("temp.mp3", "ab");
            fwrite(bites, 1, i + 10, nf);

            i += 7;
            int size = bites[i];

            fwrite(value, 1, size, nf);
            fseek(f, i + size + 3, SEEK_SET);

            char temp[1000] = {0};
            int cnt;

            while (!feof(f)) {
                cnt = fread(&temp, 1, 1000 * sizeof(char), f);
                fwrite(&temp, 1, cnt * sizeof(char), nf);
            }

            fclose(f);
            fclose(nf);

            nf = fopen("temp.mp3", "rb");
            f = fopen(file, "wb");

            fseek(nf, 0, SEEK_SET);

            while (!feof(nf)) {
                cnt = fread(&temp, 1, 1000 * sizeof(char), nf);
                fwrite(&temp, 1, cnt * sizeof(char), f);
            }

            fclose(nf);
            remove("temp.mp3");
            break;
        }
    }
    free(bites);
}

int main(int argc, char *argv[]) {
    int show = 0;
    int get = 0;
    int set = 0;

    char frame[5] = {0};
    char *value = (char *) malloc(sizeof(char) * 60);

    for (int i = 0; i < 60; i++)
        value[i] = 0;

    char *file = (char *) malloc(sizeof(char) * 9);

    for (int i = 0; i < argc; i++) {
        if (argv[i][2] == 'f') {
            for (int j = 0; j < 8; j++)
                file[j] = argv[i][j + 11];

            file[8] = '\0';
        }
        if (!strcmp("--show", argv[i]))
            show = 1;

        if (argv[i][2] == 'g') {
            get = 1;

            file[8] = '\0';

            for (int j = 0; j < 4; j++)
                frame[j] = argv[i][j + 6];

            frame[4] = '\0';
        }
        if (argv[i][2] == 's' && argv[i][3] == 'e') {
            set = 1;
            for (int j = 0; j < 4; j++)
                frame[j] = argv[i][j + 6];

            frame[4] = '\0';

            int c = 8;

            for (int j = 0; j < strlen(argv[i + 1]); j++, c++) {
                if (argv[i + 1][c] >= 'a' && argv[i + 1][c] <= 'z' || argv[i + 1][c] >= 'A' && argv[i + 1][c] <= 'Z'
                    || argv[i + 1][c] >= '0' && argv[i + 1][c] <= '9' || argv[i][c] == ' ')
                    value[j] = argv[i + 1][c];

                else break;
            }
            value[c] = '\0';
        }
    }

    if (show) commandShow(file);
    if (get) commandGet(file, frame);
    if (set) commandSet(file, frame, value);

    return 0;
}