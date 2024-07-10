#include <stdio.h>
#include <time.h>
#include <string.h>

#define SIZE 1024

void revert_buffer(char *, int);

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Nieprawidlowy format uruchomienia programu. Prawidlowy format:\n");
        printf("'`./reverse plik_wejsciowy plik_wyjsciowy1 plik_wyjsciowy2`\n");
        return 1;
    }

    FILE *in, *out1, *out2, *time_measure;
    
    in = fopen(argv[1], "r");
    if (in == NULL) {
        perror("Nie udalo sie otworzyc pliku wejsciowego.\n");
        return 1;
    }

    out1 = fopen(argv[2], "w");
    if (out1 == NULL) {
        perror("Nie udalo sie otworzyc pliku wyjsciowego nr 1.\n");
        return 1;
    }

    out2 = fopen(argv[3], "w");
    if (out2 == NULL) {
        perror("Nie udalo sie otworzyc pliku wyjsciowego nr 2.\n");
        return 1;
    }

    char buffer[SIZE];
    int count;
    clock_t start, end;
    float time;
    char string[SIZE];

    printf("Najpierw przedstawie wariant z czytaniem po 1 znaku, pozniej czytajac po 1024 znaki\n");
    printf("Dla obu sposobow zmierze czas wykonania, bazujac na pliku wejsciowym\n");
    printf("Wyniki pomiarow beda zapisane w pliku `pomiar_zad_2_txt`\n\n");
    printf("Wariant 1 - przepisywanie znak po znaku\n");

    time_measure = fopen("pomiar_zad_2.txt", "a");
    start = clock();
    fseek(in, -1, SEEK_END);

    while(ftell(in) > 0) {
        fread(buffer, sizeof(char), 1, in);
        fwrite(buffer, sizeof(char), 1, out1);
        fseek(in, -2, SEEK_CUR);
    }

    fread(buffer, sizeof(char), 1, in);
    fwrite(buffer, sizeof(char), 1, out1);

    end = clock();
    time = (float)(end - start) / CLOCKS_PER_SEC;
    strcpy(string, "Uruchomienie programu\nZnak po znaku: ");
    fwrite(string, sizeof(char), strlen(string), time_measure);
    fprintf(time_measure, "%f", time);
    fwrite("s", sizeof(char), 1, time_measure);

    printf("Wariant 2 - czytanie blokow po 1024 znaki\n\n");

    long left, to_read;
    /*fclose(in);
    in = fopen(argv[1], "r");*/
    fseek(in, 0, SEEK_END);
    left = ftell(in);

    start = clock();

    while(left > 0) {
        to_read = left > SIZE ? SIZE : left;
        fseek(in, -to_read, SEEK_CUR);
        count = fread(buffer, sizeof(char), to_read, in);
        revert_buffer(buffer, count);
        fwrite(buffer, sizeof(char), count, out2);
        fseek(in, -SIZE, SEEK_CUR);
        left -= count;
    }

    /*count = fread(buffer, sizeof(char), SIZE, in);
    revert_buffer(buffer, count);
    fwrite(buffer, sizeof(char), count, out2);*/

    end = clock();
    time = (float)(end - start) / CLOCKS_PER_SEC;
    strcpy(string, "\nW blokach po 1024 znaki: ");
    fwrite(string, sizeof(char), strlen(string), time_measure);
    fprintf(time_measure, "%f", time);
    fwrite("s\n\n", sizeof(char), 3, time_measure);

    fclose(in);
    fclose(out1);
    fclose(out2);
    fclose(time_measure);
    return 0;
}

void revert_buffer(char *buf, int size) {
    char temp;
    
    for (int i = 0; i < size / 2; i++) {
        temp = buf[i];
        buf[i] = buf[size - 1 - i];
        buf[size - 1 - i] = temp;
    }
}
