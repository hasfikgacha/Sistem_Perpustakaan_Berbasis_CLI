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

static void read_line(char *buffer, int size) {
    if (fgets(buffer, size, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

static int read_int(void) {
    char line[32];
    int value = 0;

    if (fgets(line, sizeof(line), stdin) != NULL) {
        sscanf(line, "%d", &value);
    }
    return value;
}

void admin_create_book(Book catalog[], int *book_count, int max_capacity) {
    if (*book_count >= max_capacity) {
        printf("Katalog penuh, tidak bisa menambah buku.\n");
        return;
    }

    char ocaid_baru[MAX_ID_LEN];
    printf("Masukkan ID buku (ocaid): ");
    read_line(ocaid_baru, MAX_ID_LEN);

    if (strlen(ocaid_baru) == 0) {
        printf("ID tidak boleh kosong.\n");
        return;
    }

    if (find_book_index_by_id(catalog, *book_count, ocaid_baru) != -1) {
        printf("ID sudah dipakai, buku tidak ditambahkan.\n");
        return;
    }

    int pos = *book_count;

    strcpy(catalog[pos].ocaid, ocaid_baru);

    printf("Judul buku: ");
    read_line(catalog[pos].title, MAX_TITLE_LEN);

    printf("Pengarang: ");
    read_line(catalog[pos].author, MAX_AUTHOR_LEN);

    printf("Tahun terbit: ");
    catalog[pos].year = read_int();

    printf("Jumlah halaman: ");
    catalog[pos].page_count = read_int();

    printf("Stok awal: ");
    catalog[pos].stock = read_int();

    if (catalog[pos].year <= 0 || catalog[pos].page_count <= 0 || catalog[pos].stock < 0) {
        printf("Data angka tidak valid, buku tidak ditambahkan.\n");
        return;
    }

    strcpy(catalog[pos].openlibrary_edition, "-");
    strcpy(catalog[pos].openlibrary_work, "-");
    strcpy(catalog[pos].full_text_url, "-");

    (*book_count)++;

    /* save_books_to_db(catalog, *book_count); */
    printf("Buku berhasil ditambahkan.\n");
}