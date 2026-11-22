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

void admin_update_book(Book catalog[], int book_count) {
    char ocaid_cari[MAX_ID_LEN];
    printf("Masukkan ID buku yang ingin diubah: ");
    read_line(ocaid_cari, MAX_ID_LEN);

    int idx = find_book_index_by_id(catalog, book_count, ocaid_cari);
    if (idx == -1) {
        printf("Buku dengan ID tersebut tidak ditemukan.\n");
        return;
    }

    printf("\nBuku: %s\n", catalog[idx].title);
    printf("1. Ubah judul\n");
    printf("2. Ubah pengarang\n");
    printf("3. Ubah tahun terbit\n");
    printf("4. Ubah jumlah halaman\n");
    printf("5. Tambah/kurangi stok\n");
    printf("0. Batal\n");
    printf("Pilihan: ");
    int pilihan = read_int();

    switch (pilihan) {
        case 1:
            printf("Judul baru: ");
            read_line(catalog[idx].title, MAX_TITLE_LEN);
            break;
        case 2:
            printf("Pengarang baru: ");
            read_line(catalog[idx].author, MAX_AUTHOR_LEN);
            break;
        case 3: {
            printf("Tahun baru: ");
            int tahun_baru = read_int();
            if (tahun_baru <= 0) {
                printf("Tahun tidak valid.\n");
                return;
            }
            catalog[idx].year = tahun_baru;
            break;
        }
        case 4: {
            printf("Jumlah halaman baru: ");
            int halaman_baru = read_int();
            if (halaman_baru <= 0) {
                printf("Jumlah halaman tidak valid.\n");
                return;
            }
            catalog[idx].page_count = halaman_baru;
            break;
        }
        case 5: {
            printf("Perubahan stok (positif menambah, negatif mengurangi): ");
            int perubahan = read_int();
            if (catalog[idx].stock + perubahan < 0) {
                printf("Stok tidak boleh kurang dari 0.\n");
                return;
            }
            catalog[idx].stock += perubahan;
            break;
        }
        case 0:
            printf("Perubahan dibatalkan.\n");
            return;
        default:
            printf("Pilihan tidak valid.\n");
            return;
    }

    /* save_books_to_db(catalog, book_count); */
    printf("Data buku berhasil diperbarui.\n");
}

void admin_delete_book(Book catalog[], int *book_count, const Loan loans[], int loan_count) {
    char ocaid_hapus[MAX_ID_LEN];
    printf("Masukkan ID buku yang ingin dihapus: ");
    read_line(ocaid_hapus, MAX_ID_LEN);

    int idx = find_book_index_by_id(catalog, *book_count, ocaid_hapus);
    if (idx == -1) {
        printf("Buku dengan ID tersebut tidak ditemukan.\n");
        return;
    }

    for (int i = 0; i < loan_count; i++) {
        if (strcmp(loans[i].ocaid, ocaid_hapus) == 0 &&
            strcmp(loans[i].status, "DIPINJAM") == 0) {
            printf("Buku sedang dipinjam, tidak bisa dihapus.\n");
            return;
        }
    }

    char jawaban[8];
    printf("Yakin hapus \"%s\"? (y/n): ", catalog[idx].title);
    read_line(jawaban, sizeof(jawaban));
    if (jawaban[0] != 'y' && jawaban[0] != 'Y') {
        printf("Penghapusan dibatalkan.\n");
        return;
    }

    for (int i = idx; i < *book_count - 1; i++) {
        catalog[i] = catalog[i + 1];
    }
    (*book_count)--;

    /* save_books_to_db(catalog, *book_count); */
    printf("Buku berhasil dihapus.\n");
}

void admin_view_all_loans(const Loan loans[], int loan_count, const Book catalog[], int book_count) {
    if (loan_count == 0) {
        printf("Belum ada riwayat peminjaman.\n");
        return;
    }

    printf("\n%-5s %-15s %-40s %-12s %-10s\n",
           "ID", "Peminjam", "Judul Buku", "Tanggal", "Status");
    printf("--------------------------------------------------------------------------------------\n");

    for (int i = 0; i < loan_count; i++) {
        const char *judul = "(buku sudah dihapus)";
        int idx = find_book_index_by_id(catalog, book_count, loans[i].ocaid);
        if (idx != -1) {
            judul = catalog[idx].title;
        }

        printf("%-5u %-15s %-40.40s %-12s %-10s\n",
               loans[i].id_peminjaman, loans[i].username, judul,
               loans[i].tanggal_pinjam, loans[i].status);
    }
}