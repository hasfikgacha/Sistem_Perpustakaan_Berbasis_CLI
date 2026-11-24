/*
 * user.c
 * Modul Sirkulasi Anggota (User) - Sistem Manajemen Perpustakaan CLI.
 *
 * Tanggung jawab modul ini:
 *   - menampilkan katalog buku yang tersedia (langsung atau dengan sorting),
 *   - mencari buku berdasarkan ID (ocaid), judul, atau pengarang,
 *   - meminjam dan mengembalikan buku,
 *   - menampilkan daftar peminjaman pribadi,
 *   - menjalankan menu interaktif khusus User.
 *
 * Data disimpan dalam array of struct milik main.c. Modul ini hanya
 * membaca/mengubah array tersebut, lalu memanggil save_books_to_db() dan
 * save_loans() (file_io.c) agar berkas .txt langsung tersinkronisasi.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "models.h"
#include "file_io.h"
#include "sort.h"
#include "utils.h"
#include "user.h"

/* ---------- Konstanta ---------- */

#define INPUT_SIZE 150 /* ukuran buffer untuk membaca satu baris input */
#define PAGE_SIZE 20   /* jumlah baris per halaman saat menampilkan tabel */
#define MENU_WIDTH 40  /* lebar bingkai menu utama (selaras dengan admin.c) */

#define STATUS_BORROWED "DIPINJAM"
#define STATUS_RETURNED "KEMBALI"

/* Lebar kolom tabel buku: satu-satunya sumber kebenaran format tabel. */
#define W_ID 28
#define W_TITLE 32
#define W_AUTHOR 20
#define W_YEAR 4
#define W_STOCK 4
#define BOOK_LINE_WIDTH (W_ID + W_TITLE + W_AUTHOR + W_YEAR + W_STOCK + 4 * 3)

/* Lebar kolom tabel peminjaman. */
#define W_LOAN_ID 6
#define W_LOAN_DATE 12
#define W_LOAN_STATUS 10
#define LOAN_LINE_WIDTH \
    (W_LOAN_ID + W_ID + W_TITLE + W_LOAN_DATE + W_LOAN_STATUS + 4 * 3)

/* ---------- Helper: input ---------- */

typedef enum { INPUT_OK, INPUT_INVALID, INPUT_EOF } InputStatus;

/* Menghapus spasi di awal dan akhir string (in-place). */
static void trim(char *s)
{
    char *start = s;
    size_t len;

    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }

    len = strlen(start);
    while (len > 0 && isspace((unsigned char)start[len - 1])) {
        len--;
    }

    memmove(s, start, len);
    s[len] = '\0';
}

/*
 * Membaca satu baris dari keyboard dengan aman.
 * Hasil: INPUT_OK (buf berisi teks yang sudah di-trim) atau INPUT_EOF.
 */
static InputStatus read_line(const char *prompt, char *buf, size_t size)
{
    printf("%s", prompt);
    fflush(stdout);

    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return INPUT_EOF;
    }

    /* Tidak ada '\n' berarti input lebih panjang dari buffer: buang sisanya. */
    if (strchr(buf, '\n') == NULL) {
        clear_input_buffer();
    }

    buf[strcspn(buf, "\n")] = '\0';
    trim(buf);
    return INPUT_OK;
}

/* ---------- Helper: string ---------- */

/* Mencari 'keyword' di dalam 'text' tanpa membedakan huruf besar/kecil. */
static int contains_substring_ci(const char *text, const char *keyword)
{
    size_t text_len;
    size_t key_len;
    size_t limit;
    size_t i;
    size_t j;

    if (text == NULL || keyword == NULL) {
        return 0;
    }

    text_len = strlen(text);
    key_len = strlen(keyword);

    if (key_len == 0) {
        return 1;
    }
    if (key_len > text_len) {
        return 0;
    }

    limit = text_len - key_len;
    for (i = 0; i <= limit; i++) {
        j = 0;
        while (j < key_len &&
               tolower((unsigned char)text[i + j]) ==
                   tolower((unsigned char)keyword[j])) {
            j++;
        }
        if (j == key_len) {
            return 1;
        }
    }
    return 0;
}

/* Membandingkan dua string utuh tanpa membedakan huruf besar/kecil. */
static int equals_ci(const char *a, const char *b)
{
    if (a == NULL || b == NULL) {
        return 0;
    }

    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == *b;
}

/* ---------- Helper: tampilan ---------- */

static void print_book_header(void)
{
    printf("%-*s | %-*s | %-*s | %-*s | %-*s\n",
           W_ID, "ID Buku (OCAID)",
           W_TITLE, "Judul Buku",
           W_AUTHOR, "Pengarang",
           W_YEAR, "Thn",
           W_STOCK, "Stok");
    print_line('-', BOOK_LINE_WIDTH);
}

static void print_book_row(const Book *b)
{
    printf("%-*s | %-*.*s | %-*.*s | %-*d | %-*d\n",
           W_ID, b->ocaid,
           W_TITLE, W_TITLE, b->title,
           W_AUTHOR, W_AUTHOR, b->author,
           W_YEAR, (int)b->year,
           W_STOCK, (int)b->stock);
}

/*
 * Jeda per halaman untuk tabel yang panjang.
 * Mengembalikan 1 jika lanjut, 0 jika pengguna berhenti (atau input habis).
 */
static int ask_continue(int shown)
{
    char answer[INPUT_SIZE];
    char prompt[80];

    snprintf(prompt, sizeof(prompt),
             "-- %d baris tampil. Enter = lanjut, q = berhenti: ", shown);

    if (read_line(prompt, answer, sizeof(answer)) == INPUT_EOF) {
        return 0;
    }
    return !(answer[0] == 'q' || answer[0] == 'Q');
}

/* ---------- Helper: peminjaman ---------- */

/* Menyalin string dengan batas ukuran; hasil selalu diakhiri '\0'. */
static void copy_string(char *dst, size_t size, const char *src)
{
    if (size == 0) {
        return;
    }
    snprintf(dst, size, "%s", src);
}

/* Mengisi 'buffer' dengan tanggal hari ini berformat DD-MM-YYYY. */
static void get_current_date_str(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    if (local == NULL || strftime(buffer, size, "%d-%m-%Y", local) == 0) {
        copy_string(buffer, size, "00-00-0000");
    }
}

/* Mencari indeks buku berdasarkan ocaid. Mengembalikan -1 jika tidak ada. */
static int find_book_index(const Book catalog[], int book_count,
                           const char *ocaid)
{
    int i;

    for (i = 0; i < book_count; i++) {
        if (equals_ci(catalog[i].ocaid, ocaid)) {
            return i;
        }
    }
    return -1;
}

static int loan_is_borrowed(const Loan *loan)
{
    return strcmp(loan->status, STATUS_BORROWED) == 0;
}

/* Apakah 'username' masih meminjam buku 'ocaid' (status DIPINJAM)? */
static int has_active_loan(const Loan loans[], int loan_count,
                           const char *username, const char *ocaid)
{
    int i;

    for (i = 0; i < loan_count; i++) {
        if (strcmp(loans[i].username, username) == 0 &&
            equals_ci(loans[i].ocaid, ocaid) &&
            loan_is_borrowed(&loans[i])) {
            return 1;
        }
    }
    return 0;
}

/* ID transaksi berikutnya = ID terbesar yang ada + 1. */
static unsigned int next_loan_id(const Loan loans[], int loan_count)
{
    unsigned int max_id = 0;
    int i;

    for (i = 0; i < loan_count; i++) {
        if (loans[i].id_peminjaman > max_id) {
            max_id = loans[i].id_peminjaman;
        }
    }
    return max_id + 1;
}

/* Membaca bilangan bulat dalam rentang [min, max]. */
static InputStatus read_long(const char *prompt, long min, long max,
                             long *value)
{
    char buffer[INPUT_SIZE];
    char *end = NULL;
    long parsed;

    if (read_line(prompt, buffer, sizeof(buffer)) == INPUT_EOF) {
        return INPUT_EOF;
    }

    if (buffer[0] == '\0') {
        printf("[!] Input tidak boleh kosong.\n");
        return INPUT_INVALID;
    }

    parsed = strtol(buffer, &end, 10);
    if (*end != '\0' || parsed < min || parsed > max) {
        printf("[!] Masukkan angka bulat antara %ld dan %ld.\n", min, max);
        return INPUT_INVALID;
    }

    *value = parsed;
    return INPUT_OK;
}

/* ---------- Fitur: katalog & pencarian ---------- */

void user_view_available_books(const Book catalog[], int book_count)
{
    int found = 0;
    int stopped = 0;
    int i;

    putchar('\n');
    print_line('=', BOOK_LINE_WIDTH);
    print_centered("KATALOG BUKU TERSEDIA UNTUK DIPINJAM", BOOK_LINE_WIDTH);
    print_line('=', BOOK_LINE_WIDTH);

    if (catalog != NULL && book_count > 0) {
        for (i = 0; i < book_count; i++) {
            if (catalog[i].stock <= 0) {
                continue;
            }
            if (found > 0 && found % PAGE_SIZE == 0 && !ask_continue(found)) {
                stopped = 1;
                break;
            }
            if (found == 0) {
                print_book_header();
            }
            print_book_row(&catalog[i]);
            found++;
        }
    }

    if (found == 0) {
        printf("Saat ini tidak ada buku yang tersedia untuk dipinjam.\n");
    } else if (stopped) {
        printf("\nTampilan dihentikan setelah %d buku.\n", found);
    } else {
        printf("\nTotal %d buku tersedia.\n", found);
    }
    print_line('=', BOOK_LINE_WIDTH);
}

static void user_catalog_menu(Book catalog[], int book_count)
{
    long sub_choice = -1;
    InputStatus status;

    printf("\n--- LIHAT KATALOG BUKU ---\n");
    printf(" 1. Lihat langsung (tanpa sorting)\n");
    printf(" 2. Urutkan & lihat katalog (sorting)\n");
    printf(" 0. Kembali\n");
    print_line('-', MENU_WIDTH);

    do {
        status = read_long("Pilih opsi [0-2]: ", 0, 2, &sub_choice);
    } while (status == INPUT_INVALID);

    if (status == INPUT_EOF || sub_choice == 0) {
        return;
    }

    if (sub_choice == 1) {
        user_view_available_books(catalog, book_count);
        press_enter_to_continue();
        return;
    }

    /* Opsi 2: filter buku yang tersedia (stock > 0) lalu masuk menu sorting */
    if (catalog == NULL || book_count <= 0) {
        printf("[!] Katalog buku kosong.\n");
        press_enter_to_continue();
        return;
    }

    Book *available = (Book *)malloc((size_t)book_count * sizeof(Book));
    if (available == NULL) {
        interactive_sort_and_view_catalog(catalog, book_count);
        return;
    }

    int avail_count = 0;
    for (int i = 0; i < book_count; i++) {
        if (catalog[i].stock > 0) {
            available[avail_count++] = catalog[i];
        }
    }

    if (avail_count == 0) {
        printf("[!] Saat ini tidak ada buku yang tersedia untuk dipinjam.\n");
        free(available);
        press_enter_to_continue();
        return;
    }

    interactive_sort_and_view_catalog(available, avail_count);
    free(available);
}

void user_search_books(const Book catalog[], int book_count)
{
    char keyword[INPUT_SIZE];
    int found = 0;
    int stopped = 0;
    int i;

    printf("\n--- PENCARIAN BUKU ---\n");
    if (read_line("Masukkan ID Buku (OCAID), Judul, atau Pengarang: ",
                  keyword, sizeof(keyword)) == INPUT_EOF) {
        return;
    }

    if (keyword[0] == '\0') {
        printf("[!] Kata kunci pencarian tidak boleh kosong.\n");
        return;
    }

    printf("\nHasil pencarian untuk \"%s\":\n", keyword);
    print_line('-', BOOK_LINE_WIDTH);

    if (catalog != NULL && book_count > 0) {
        for (i = 0; i < book_count; i++) {
            if (!(equals_ci(catalog[i].ocaid, keyword) ||
                  contains_substring_ci(catalog[i].title, keyword) ||
                  contains_substring_ci(catalog[i].author, keyword))) {
                continue;
            }
            if (found > 0 && found % PAGE_SIZE == 0 && !ask_continue(found)) {
                stopped = 1;
                break;
            }
            if (found == 0) {
                print_book_header();
            }
            print_book_row(&catalog[i]);
            found++;
        }
    }

    if (found == 0) {
        printf("Tidak ditemukan buku yang cocok dengan kata kunci tersebut.\n");
    } else if (stopped) {
        printf("\nTampilan dihentikan setelah %d buku.\n", found);
    } else {
        printf("\nDitemukan %d buku.\n", found);
    }
    print_line('-', BOOK_LINE_WIDTH);
}

/* ---------- Fitur: peminjaman ---------- */

void user_borrow_book(Book catalog[], int book_count, Loan loans[],
                      int *loan_count, int loan_capacity,
                      const User *current_user)
{
    char ocaid[INPUT_SIZE];
    Loan new_loan;
    int index;

    if (catalog == NULL || loans == NULL || loan_count == NULL ||
        current_user == NULL) {
        printf("[!] Data internal tidak valid.\n");
        return;
    }

    printf("\n--- PINJAM BUKU ---\n");

    if (*loan_count >= loan_capacity) {
        printf("[!] Kapasitas data peminjaman penuh. Hubungi pustakawan.\n");
        return;
    }

    if (read_line("Masukkan ID Buku (OCAID) yang ingin dipinjam "
                  "(kosongkan untuk batal): ",
                  ocaid, sizeof(ocaid)) == INPUT_EOF) {
        return;
    }
    if (ocaid[0] == '\0') {
        printf("Peminjaman dibatalkan.\n");
        return;
    }

    index = find_book_index(catalog, book_count, ocaid);
    if (index < 0) {
        printf("[!] Buku dengan ID \"%s\" tidak ditemukan.\n", ocaid);
        return;
    }

    if (catalog[index].stock <= 0) {
        printf("[!] Maaf, stok buku \"%s\" sedang habis.\n",
               catalog[index].title);
        return;
    }

    if (has_active_loan(loans, *loan_count, current_user->username,
                        catalog[index].ocaid)) {
        printf("[!] Anda masih meminjam buku \"%s\". "
               "Kembalikan dulu sebelum meminjam lagi.\n",
               catalog[index].title);
        return;
    }

    memset(&new_loan, 0, sizeof(new_loan));
    new_loan.id_peminjaman = next_loan_id(loans, *loan_count);
    copy_string(new_loan.username, sizeof(new_loan.username),
                current_user->username);
    copy_string(new_loan.ocaid, sizeof(new_loan.ocaid), catalog[index].ocaid);
    get_current_date_str(new_loan.tanggal_pinjam,
                         sizeof(new_loan.tanggal_pinjam));
    copy_string(new_loan.status, sizeof(new_loan.status), STATUS_BORROWED);

    /* Ubah data di memori dulu, baru simpan ke berkas. */
    catalog[index].stock--;
    loans[*loan_count] = new_loan;
    (*loan_count)++;

    save_books_to_db(catalog, book_count);
    save_loans(loans, *loan_count);

    printf("[OK] Berhasil meminjam \"%s\".\n", catalog[index].title);
    printf("     ID transaksi: %u | Tanggal pinjam: %s | Sisa stok: %d\n",
           new_loan.id_peminjaman, new_loan.tanggal_pinjam,
           (int)catalog[index].stock);
}

/* ---------- Fitur: peminjaman pribadi ---------- */

/*
 * Menampilkan peminjaman milik current_user.
 * only_active = 1 -> hanya yang berstatus DIPINJAM; 0 -> seluruh riwayat.
 * Mengembalikan jumlah baris yang ditampilkan.
 */
int user_view_personal_loans(const Book catalog[], int book_count,
                             const Loan loans[], int loan_count,
                             const User *current_user, int only_active)
{
    const char *title;
    int shown = 0;
    int index;
    int i;

    if (current_user == NULL || loans == NULL) {
        return 0;
    }

    putchar('\n');
    print_line('=', LOAN_LINE_WIDTH);
    print_centered(only_active ? "PEMINJAMAN AKTIF SAYA"
                               : "RIWAYAT PEMINJAMAN SAYA",
                   LOAN_LINE_WIDTH);
    print_line('=', LOAN_LINE_WIDTH);

    for (i = 0; i < loan_count; i++) {
        if (strcmp(loans[i].username, current_user->username) != 0) {
            continue;
        }
        if (only_active && !loan_is_borrowed(&loans[i])) {
            continue;
        }

        if (shown == 0) {
            printf("%-*s | %-*s | %-*s | %-*s | %-*s\n",
                   W_LOAN_ID, "ID",
                   W_ID, "ID Buku (OCAID)",
                   W_TITLE, "Judul Buku",
                   W_LOAN_DATE, "Tgl Pinjam",
                   W_LOAN_STATUS, "Status");
            print_line('-', LOAN_LINE_WIDTH);
        }

        index = (catalog != NULL)
                    ? find_book_index(catalog, book_count, loans[i].ocaid)
                    : -1;
        title = (index >= 0) ? catalog[index].title : "(buku sudah dihapus)";

        printf("%-*u | %-*.*s | %-*.*s | %-*.*s | %-*.*s\n",
               W_LOAN_ID, loans[i].id_peminjaman,
               W_ID, W_ID, loans[i].ocaid,
               W_TITLE, W_TITLE, title,
               W_LOAN_DATE, W_LOAN_DATE, loans[i].tanggal_pinjam,
               W_LOAN_STATUS, W_LOAN_STATUS, loans[i].status);
        shown++;
    }

    if (shown == 0) {
        printf(only_active ? "Anda tidak sedang meminjam buku apa pun.\n"
                           : "Anda belum pernah meminjam buku.\n");
    } else {
        printf("\nTotal %d transaksi.\n", shown);
    }
    print_line('=', LOAN_LINE_WIDTH);
    return shown;
}

/* ---------- Fitur: pengembalian ---------- */

void user_return_book(Book catalog[], int book_count, Loan loans[],
                      int loan_count, const User *current_user)
{
    long loan_id = 0;
    InputStatus status;
    int loan_index = -1;
    int book_index;
    int i;

    if (catalog == NULL || loans == NULL || current_user == NULL) {
        printf("[!] Data internal tidak valid.\n");
        return;
    }

    printf("\n--- KEMBALIKAN BUKU ---\n");

    if (user_view_personal_loans(catalog, book_count, loans, loan_count,
                                 current_user, 1) == 0) {
        return;
    }

    status = read_long("Masukkan ID transaksi yang dikembalikan "
                       "(0 untuk batal): ",
                       0, 2147483647L, &loan_id);
    if (status != INPUT_OK) {
        return;
    }
    if (loan_id == 0) {
        printf("Pengembalian dibatalkan.\n");
        return;
    }

    for (i = 0; i < loan_count; i++) {
        if (loans[i].id_peminjaman == (unsigned int)loan_id &&
            strcmp(loans[i].username, current_user->username) == 0 &&
            loan_is_borrowed(&loans[i])) {
            loan_index = i;
            break;
        }
    }
    if (loan_index < 0) {
        printf("[!] ID transaksi %ld tidak ditemukan di peminjaman aktif "
               "Anda.\n",
               loan_id);
        return;
    }

    copy_string(loans[loan_index].status, sizeof(loans[loan_index].status),
                STATUS_RETURNED);

    book_index = find_book_index(catalog, book_count, loans[loan_index].ocaid);
    if (book_index >= 0) {
        catalog[book_index].stock++;
    }

    save_books_to_db(catalog, book_count);
    save_loans(loans, loan_count);

    if (book_index >= 0) {
        printf("[OK] Buku \"%s\" berhasil dikembalikan. Stok sekarang: %d\n",
               catalog[book_index].title, (int)catalog[book_index].stock);
    } else {
        printf("[OK] Buku dikembalikan. (Buku sudah tidak ada di katalog, "
               "stok tidak diubah.)\n");
    }
}

/* ---------- Menu User ---------- */

static void print_user_menu(const User *current_user)
{
    char header[64];
    snprintf(header, sizeof(header), "MENU ANGGOTA - %s", current_user->username);

    putchar('\n');
    print_line('=', MENU_WIDTH);
    print_centered(header, MENU_WIDTH);
    print_line('=', MENU_WIDTH);
    printf(" 1. Lihat katalog buku tersedia\n");
    printf(" 2. Cari buku\n");
    printf(" 3. Pinjam buku\n");
    printf(" 4. Kembalikan buku\n");
    printf(" 5. Peminjaman aktif saya\n");
    printf(" 6. Riwayat peminjaman saya\n");
    printf(" 0. Logout\n");
    print_line('-', MENU_WIDTH);
}

void run_user_session(const User *current_user, Book catalog[], int book_count,
                      Loan loans[], int *loan_count, int loan_capacity)
{
    long choice = -1;
    InputStatus status;

    if (current_user == NULL || catalog == NULL || loans == NULL ||
        loan_count == NULL) {
        printf("[!] Sesi User tidak dapat dimulai: data tidak valid.\n");
        return;
    }

    do {
        clear_screen();
        print_user_menu(current_user);

        do {
            status = read_long("Pilih menu [0-6]: ", 0, 6, &choice);
        } while (status == INPUT_INVALID);

        if (status == INPUT_EOF) {
            break; /* input habis: keluar dengan aman */
        }

        switch (choice) {
        case 1:
            user_catalog_menu(catalog, book_count);
            break;
        case 2:
            user_search_books(catalog, book_count);
            press_enter_to_continue();
            break;
        case 3:
            user_borrow_book(catalog, book_count, loans, loan_count,
                             loan_capacity, current_user);
            press_enter_to_continue();
            break;
        case 4:
            user_return_book(catalog, book_count, loans, *loan_count,
                             current_user);
            press_enter_to_continue();
            break;
        case 5:
            user_view_personal_loans(catalog, book_count, loans, *loan_count,
                                     current_user, 1);
            press_enter_to_continue();
            break;
        case 6:
            user_view_personal_loans(catalog, book_count, loans, *loan_count,
                                     current_user, 0);
            press_enter_to_continue();
            break;
        case 0:
            printf("\n[OK] Logout berhasil. Sampai jumpa, %s!\n",
                   current_user->username);
            press_enter_to_continue();
            break;
        default:
            break;
        }
    } while (choice != 0);
}