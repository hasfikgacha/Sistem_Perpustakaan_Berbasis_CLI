#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "admin.h"
#include "file_io.h"
#include "sort.h"
#include "utils.h"

#define MENU_WIDTH 40

/* Lebar kolom tabel riwayat peminjaman (selaras dengan user.c) */
#define W_LOAN_ID     6
#define W_USERNAME    16
#define W_TITLE       34
#define W_LOAN_DATE   12
#define W_LOAN_STATUS 10
#define ADMIN_LOAN_WIDTH \
    (W_LOAN_ID + W_USERNAME + W_TITLE + W_LOAN_DATE + W_LOAN_STATUS + 4 * 3)

int find_book_index_by_id(const Book catalog[], int book_count, const char *ocaid) {
    for (int i = 0; i < book_count; i++) {
        if (strcmp(catalog[i].ocaid, ocaid) == 0) {
            return i;
        }
    }
    return -1;
}

static int has_whitespace(const char *s) {
    while (*s != '\0') {
        if (isspace((unsigned char)*s)) {
            return 1;
        }
        s++;
    }
    return 0;
}

/* Membaca bilangan bulat secara ketat; return 1 jika valid, 0 jika bukan angka */
static int read_int_strict(int *out_value) {
    char line[32];
    char *end = NULL;
    long val;

    read_string(line, sizeof(line));
    if (line[0] == '\0') {
        return 0;
    }
    val = strtol(line, &end, 10);
    if (*end != '\0') {
        return 0;
    }
    *out_value = (int)val;
    return 1;
}

void admin_create_book(Book catalog[], int *book_count, int max_capacity) {
    printf("\n--- TAMBAH BUKU BARU ---\n");

    if (*book_count >= max_capacity) {
        printf("[!] Katalog penuh, tidak bisa menambah buku.\n");
        return;
    }

    char ocaid_baru[MAX_ID_LEN];
    printf("Masukkan ID buku (OCAID, tanpa spasi): ");
    read_string(ocaid_baru, MAX_ID_LEN);

    if (strlen(ocaid_baru) == 0 || has_whitespace(ocaid_baru)) {
        printf("[!] ID tidak boleh kosong atau mengandung spasi.\n");
        return;
    }

    if (find_book_index_by_id(catalog, *book_count, ocaid_baru) != -1) {
        printf("[!] ID \"%s\" sudah dipakai, buku tidak ditambahkan.\n", ocaid_baru);
        return;
    }

    int pos = *book_count;
    Book temp;
    memset(&temp, 0, sizeof(temp));
    strcpy(temp.ocaid, ocaid_baru);

    printf("Judul buku     : ");
    read_string(temp.title, MAX_TITLE_LEN);
    if (strlen(temp.title) == 0) {
        printf("[!] Judul tidak boleh kosong.\n");
        return;
    }

    printf("Pengarang      : ");
    read_string(temp.author, MAX_AUTHOR_LEN);
    if (strlen(temp.author) == 0) {
        printf("[!] Pengarang tidak boleh kosong.\n");
        return;
    }

    printf("Tahun terbit   : ");
    if (!read_int_strict(&temp.year) || temp.year <= 0) {
        printf("[!] Tahun terbit tidak valid.\n");
        return;
    }

    printf("Jumlah halaman : ");
    if (!read_int_strict(&temp.page_count) || temp.page_count <= 0) {
        printf("[!] Jumlah halaman tidak valid.\n");
        return;
    }

    printf("Stok awal      : ");
    if (!read_int_strict(&temp.stock) || temp.stock < 0) {
        printf("[!] Stok awal tidak valid.\n");
        return;
    }

    strcpy(temp.openlibrary_edition, "-");
    strcpy(temp.openlibrary_work, "-");
    strcpy(temp.full_text_url, "-");

    catalog[pos] = temp;
    (*book_count)++;

    save_books_to_db(catalog, *book_count);
    printf("[OK] Buku \"%s\" berhasil ditambahkan.\n", temp.title);
}

void admin_update_book(Book catalog[], int book_count) {
    char ocaid_cari[MAX_ID_LEN];

    printf("\n--- UBAH DATA BUKU ---\n");
    printf("Masukkan ID buku (OCAID) yang ingin diubah: ");
    read_string(ocaid_cari, MAX_ID_LEN);

    int idx = find_book_index_by_id(catalog, book_count, ocaid_cari);
    if (idx == -1) {
        printf("[!] Buku dengan ID \"%s\" tidak ditemukan.\n", ocaid_cari);
        return;
    }

    printf("\nBuku terpilih: %s\n", catalog[idx].title);
    print_line('-', MENU_WIDTH);
    printf(" 1. Ubah judul\n");
    printf(" 2. Ubah pengarang\n");
    printf(" 3. Ubah tahun terbit\n");
    printf(" 4. Ubah jumlah halaman\n");
    printf(" 5. Tambah/kurangi stok (Saat ini: %d)\n", catalog[idx].stock);
    printf(" 0. Batal\n");
    print_line('-', MENU_WIDTH);
    printf("Pilih field [0-5]: ");
    int pilihan = read_int_range(0, 5);

    switch (pilihan) {
        case 1: {
            char judul_baru[MAX_TITLE_LEN];
            printf("Judul baru: ");
            read_string(judul_baru, MAX_TITLE_LEN);
            if (strlen(judul_baru) == 0) {
                printf("[!] Judul tidak boleh kosong.\n");
                return;
            }
            strcpy(catalog[idx].title, judul_baru);
            break;
        }
        case 2: {
            char pengarang_baru[MAX_AUTHOR_LEN];
            printf("Pengarang baru: ");
            read_string(pengarang_baru, MAX_AUTHOR_LEN);
            if (strlen(pengarang_baru) == 0) {
                printf("[!] Pengarang tidak boleh kosong.\n");
                return;
            }
            strcpy(catalog[idx].author, pengarang_baru);
            break;
        }
        case 3: {
            int tahun_baru;
            printf("Tahun terbit baru: ");
            if (!read_int_strict(&tahun_baru) || tahun_baru <= 0) {
                printf("[!] Tahun tidak valid.\n");
                return;
            }
            catalog[idx].year = tahun_baru;
            break;
        }
        case 4: {
            int halaman_baru;
            printf("Jumlah halaman baru: ");
            if (!read_int_strict(&halaman_baru) || halaman_baru <= 0) {
                printf("[!] Jumlah halaman tidak valid.\n");
                return;
            }
            catalog[idx].page_count = halaman_baru;
            break;
        }
        case 5: {
            int perubahan;
            printf("Perubahan stok (positif menambah, negatif mengurangi): ");
            if (!read_int_strict(&perubahan)) {
                printf("[!] Input stok harus berupa angka.\n");
                return;
            }
            if (catalog[idx].stock + perubahan < 0) {
                printf("[!] Stok tidak boleh kurang dari 0.\n");
                return;
            }
            catalog[idx].stock += perubahan;
            break;
        }
        case 0:
            printf("Perubahan dibatalkan.\n");
            return;
        default:
            printf("[!] Pilihan tidak valid.\n");
            return;
    }

    save_books_to_db(catalog, book_count);
    printf("[OK] Data buku berhasil diperbarui.\n");
}

void admin_delete_book(Book catalog[], int *book_count, const Loan loans[], int loan_count) {
    char ocaid_hapus[MAX_ID_LEN];

    printf("\n--- HAPUS BUKU ---\n");
    printf("Masukkan ID buku (OCAID) yang ingin dihapus: ");
    read_string(ocaid_hapus, MAX_ID_LEN);

    int idx = find_book_index_by_id(catalog, *book_count, ocaid_hapus);
    if (idx == -1) {
        printf("[!] Buku dengan ID \"%s\" tidak ditemukan.\n", ocaid_hapus);
        return;
    }

    for (int i = 0; i < loan_count; i++) {
        if (strcmp(loans[i].ocaid, ocaid_hapus) == 0 &&
            strcmp(loans[i].status, "DIPINJAM") == 0) {
            printf("[!] Buku sedang dipinjam, tidak bisa dihapus.\n");
            return;
        }
    }

    char jawaban[8];
    printf("Yakin hapus \"%s\"? (y/n): ", catalog[idx].title);
    read_string(jawaban, sizeof(jawaban));
    if (jawaban[0] != 'y' && jawaban[0] != 'Y') {
        printf("Penghapusan dibatalkan.\n");
        return;
    }

    for (int i = idx; i < *book_count - 1; i++) {
        catalog[i] = catalog[i + 1];
    }
    (*book_count)--;

    save_books_to_db(catalog, *book_count);
    printf("[OK] Buku berhasil dihapus.\n");
}


void admin_view_all_loans(const Loan loans[], int loan_count, const Book catalog[], int book_count) {
    putchar('\n');
    print_line('=', ADMIN_LOAN_WIDTH);
    print_centered("RIWAYAT SELURUH PEMINJAMAN", ADMIN_LOAN_WIDTH);
    print_line('=', ADMIN_LOAN_WIDTH);

    if (loan_count == 0) {
        printf("Belum ada riwayat peminjaman.\n");
        print_line('=', ADMIN_LOAN_WIDTH);
        return;
    }

    printf("%-*s | %-*s | %-*s | %-*s | %-*s\n",
           W_LOAN_ID, "ID",
           W_USERNAME, "Peminjam",
           W_TITLE, "Judul Buku",
           W_LOAN_DATE, "Tgl Pinjam",
           W_LOAN_STATUS, "Status");
    print_line('-', ADMIN_LOAN_WIDTH);

    for (int i = 0; i < loan_count; i++) {
        const char *judul = "(buku sudah dihapus)";
        int idx = find_book_index_by_id(catalog, book_count, loans[i].ocaid);
        if (idx != -1) {
            judul = catalog[idx].title;
        }

        printf("%-*u | %-*.*s | %-*.*s | %-*.*s | %-*.*s\n",
               W_LOAN_ID, loans[i].id_peminjaman,
               W_USERNAME, W_USERNAME, loans[i].username,
               W_TITLE, W_TITLE, judul,
               W_LOAN_DATE, W_LOAN_DATE, loans[i].tanggal_pinjam,
               W_LOAN_STATUS, W_LOAN_STATUS, loans[i].status);
    }

    printf("\nTotal %d transaksi.\n", loan_count);
    print_line('=', ADMIN_LOAN_WIDTH);
}

static void print_admin_menu(void) {
    putchar('\n');
    print_line('=', MENU_WIDTH);
    print_centered("MENU ADMIN PERPUSTAKAAN", MENU_WIDTH);
    print_line('=', MENU_WIDTH);
    printf(" 1. Lihat & urutkan katalog buku\n");
    printf(" 2. Tambah buku\n");
    printf(" 3. Ubah data buku\n");
    printf(" 4. Hapus buku\n");
    printf(" 5. Lihat riwayat peminjaman\n");
    printf(" 0. Logout\n");
    print_line('-', MENU_WIDTH);
}

void run_admin_session(Book catalog[], int *book_count, int max_capacity, Loan loans[], int loan_count) {
    int pilihan = 0;

    while (pilihan != 6) {
        printf("\n===== MENU ADMIN =====\n");
        printf("1. Lihat & urutkan katalog buku\n");
        printf("2. Tambah buku\n");
        printf("3. Ubah data buku\n");
        printf("4. Hapus buku\n");
        printf("5. Lihat riwayat peminjaman\n");
        printf("6. Keluar\n");
        printf("Pilihan: ");
        pilihan = read_int_range(1, 6);

        switch (pilihan) {
            case 1:
                interactive_sort_and_view_catalog(catalog, *book_count);
                break;
            case 2:
                admin_create_book(catalog, book_count, max_capacity);
                break;
            case 3:
                admin_update_book(catalog, *book_count);
                break;
            case 4:
                admin_delete_book(catalog, book_count, loans, loan_count);
                break;
            case 5:
                admin_view_all_loans(loans, loan_count, catalog, *book_count);
                break;
            case 6:
                printf("Keluar dari menu admin.\n");
                break;
            default:
                printf("Pilihan tidak valid.\n");
        }
    }
}