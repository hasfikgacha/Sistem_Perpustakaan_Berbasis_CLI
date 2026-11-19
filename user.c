#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "models.h"
#include "user.h"

/* ---------- Konstanta ---------- */

#define INPUT_SIZE 150 /* ukuran buffer untuk membaca satu baris input */
#define PAGE_SIZE 20   /* jumlah baris per halaman saat menampilkan tabel */

/* Lebar kolom tabel buku: satu-satunya sumber kebenaran format tabel. */
#define W_ID 18
#define W_TITLE 32
#define W_AUTHOR 20
#define W_YEAR 4
#define W_STOCK 4
#define BOOK_LINE_WIDTH (W_ID + W_TITLE + W_AUTHOR + W_YEAR + W_STOCK + 4 * 3)

/* ---------- Helper: input ---------- */

typedef enum { INPUT_OK, INPUT_INVALID, INPUT_EOF } InputStatus;

/* Membuang sisa karakter di stdin sampai akhir baris. */
static void clear_input_buffer(void)
{
    int c;

    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

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

static void print_line(char ch, int width)
{
    int i;

    for (i = 0; i < width; i++) {
        putchar(ch);
    }
    putchar('\n');
}

static void print_centered(const char *text, int width)
{
    int pad = (width - (int)strlen(text)) / 2;

    if (pad < 0) {
        pad = 0;
    }
    printf("%*s%s\n", pad, "", text);
}

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
    printf("%-*.*s | %-*.*s | %-*.*s | %-*d | %-*d\n",
           W_ID, W_ID, b->ocaid,
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