#include <stdio.h>
#include <string.h>
#include "admin.h"

int find_book_index_by_id(const Book catalog[], int book_count, const char *ocaid) {
    for (int i = 0; i < book_count; i++) {
        if (strcmp(catalog[i].ocaid, ocaid) == 0) {
            return i;
        }
    }
    return -1;
}

void admin_create_book(Book catalog[], int *book_count, int max_capacity) {
    if (*book_count >= max_capacity) {
        printf("Katalog penuh, tidak bisa menambah buku.\n");
        return;
    }

    char ocaid_baru[64];
    printf("Masukkan ID buku (ocaid): ");
    scanf("%63s", ocaid_baru);

    if (find_book_index_by_id(catalog, *book_count, ocaid_baru) != -1) {
        printf("ID sudah dipakai, buku tidak ditambahkan.\n");
        return;
    }

}