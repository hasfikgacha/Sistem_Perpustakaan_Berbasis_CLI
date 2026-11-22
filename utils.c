#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

void clear_input_buffer(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        /* membuang sisa input */
    }
}

void read_string(char *buffer, int size)
{
    size_t len;

    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }
    len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else {
        clear_input_buffer();
    }
}

int read_int_range(int min, int max)
{
    char line[32];
    char *end;
    long value;

    while (1) {
        read_string(line, 32);
        if (feof(stdin)) {
            return min; /* input habis: hindari loop tanpa henti */
        }
        value = strtol(line, &end, 10);
        if (end == line || *end != '\0') {
            printf("Input harus berupa angka. Coba lagi: ");
        } else if (value < min || value > max) {
            printf("Angka harus antara %d dan %d. Coba lagi: ", min, max);
        } else {
            return (int)value;
        }
    }
}

void press_enter_to_continue(void)
{
    printf("Tekan Enter untuk melanjutkan...");
    clear_input_buffer();
}

void clear_screen(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}