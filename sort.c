#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "sort.h"
#include "utils.h"

#define PAGE_SIZE 15

/* Salinan katalog, supaya urutan data asli tidak berubah. */
static Book sorted_view[MAX_BOOKS];

static const char *ALGO_NAMES[] = {"", "Bubble", "Selection", "Insertion"};
static const char *ATTR_NAMES[] = {"", "ID (ocaid)", "Judul", "Tahun",
                                   "Jumlah Halaman", "Stok"};

/* Bandingkan teks tanpa peduli huruf besar/kecil */
static int str_compare_ci(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) {
            return ca - cb;
        }
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

/* <0 jika a lebih kecil, 0 jika sama, >0 jika a lebih besar */
static int compare_books(const Book *a, const Book *b, int attribute)
{
    switch (attribute) {
    case SORT_BY_OCAID:
        return strcmp(a->ocaid, b->ocaid);
    case SORT_BY_TITLE:
        return str_compare_ci(a->title, b->title);
    case SORT_BY_YEAR:
        return (a->year > b->year) - (a->year < b->year);
    case SORT_BY_PAGE_COUNT:
        return (a->page_count > b->page_count) - (a->page_count < b->page_count);
    case SORT_BY_STOCK:
        return (a->stock > b->stock) - (a->stock < b->stock);
    }
    return 0;
}

/* 1 jika a harus berada SETELAH b pada urutan yang diminta */
static int should_swap(const Book *a, const Book *b, int attribute, int ascending)
{
    int cmp = compare_books(a, b, attribute);
    return ascending ? (cmp > 0) : (cmp < 0);
}

void bubble_sort_books(Book books[], int count, int attribute, int ascending)
{
    int i, j, swapped;
    Book temp;

    for (i = 0; i < count - 1; i++) {
        swapped = 0;
        for (j = 0; j < count - 1 - i; j++) {
            if (should_swap(&books[j], &books[j + 1], attribute, ascending)) {
                temp = books[j];
                books[j] = books[j + 1];
                books[j + 1] = temp;
                swapped = 1;
            }
        }
        if (!swapped) {
            break; /* sudah terurut */
        }
    }
}

void selection_sort_books(Book books[], int count, int attribute, int ascending)
{
    int i, j, target;
    Book temp;

    for (i = 0; i < count - 1; i++) {
        target = i;
        for (j = i + 1; j < count; j++) {
            if (should_swap(&books[target], &books[j], attribute, ascending)) {
                target = j;
            }
        }
        if (target != i) {
            temp = books[i];
            books[i] = books[target];
            books[target] = temp;
        }
    }
}

void insertion_sort_books(Book books[], int count, int attribute, int ascending)
{
    int i, j;
    Book key;

    for (i = 1; i < count; i++) {
        key = books[i];
        j = i - 1;
        while (j >= 0 && should_swap(&books[j], &key, attribute, ascending)) {
            books[j + 1] = books[j];
            j--;
        }
        books[j + 1] = key;
    }
}

/* ---------- Tampilan katalog + pagination ---------- */

static void print_page(int count, int page, int total_pages)
{
    int start = (page - 1) * PAGE_SIZE;
    int end = start + PAGE_SIZE;
    int i;

    if (end > count) {
        end = count;
    }
    printf("%-5s %-16s %-40s %-6s %-6s %-5s\n",
           "No", "OCAID", "Judul", "Tahun", "Hal", "Stok");
    printf("---------------------------------------------------------------------------------\n");
    for (i = start; i < end; i++) {
        printf("%-5d %-16.16s %-40.40s %-6d %-6d %-5d\n",
               i + 1, sorted_view[i].ocaid, sorted_view[i].title,
               sorted_view[i].year, sorted_view[i].page_count,
               sorted_view[i].stock);
    }
    printf("\nHalaman %d dari %d (%d buku)\n", page, total_pages, count);
}

/* Mengembalikan 1 jika ingin mengurutkan ulang, 0 jika keluar */
static int browse_pages(int count, const char *status)
{
    int total_pages = (count + PAGE_SIZE - 1) / PAGE_SIZE;
    int page = 1;
    char cmd[16];

    while (1) {
        clear_screen();
        printf("%s\n\n", status);
        print_page(count, page, total_pages);
        printf("\n[N] Next  [P] Prev  [G] Ke halaman  [S] Urut ulang  [Q] Keluar\nPilihan: ");
        read_string(cmd, 16);
        if (feof(stdin)) {
            return 0;
        }
        switch (toupper((unsigned char)cmd[0])) {
        case 'N':
            if (page < total_pages) {
                page++;
            }
            break;
        case 'P':
            if (page > 1) {
                page--;
            }
            break;
        case 'G':
            printf("Nomor halaman (1-%d): ", total_pages);
            page = read_int_range(1, total_pages);
            break;
        case 'S':
            return 1;
        case 'Q':
            return 0;
        default:
            break;
        }
    }
}

void interactive_sort_and_view_catalog(const Book books[], int count)
{
    int algo, attr, order;
    clock_t start, end;
    double ms;
    char status[200];

    if (count <= 0) {
        printf("Katalog kosong.\n");
        press_enter_to_continue();
        return;
    }
    if (count > MAX_BOOKS) {
        count = MAX_BOOKS;
    }

    while (1) {
        clear_screen();
        printf("=== Urutkan Katalog ===\n");
        printf("1. Bubble Sort\n2. Selection Sort\n3. Insertion Sort\n");
        printf("Pilih algoritma (1-3): ");
        algo = read_int_range(1, 3);

        printf("\n1. ID (ocaid)\n2. Judul\n3. Tahun\n4. Jumlah Halaman\n5. Stok\n");
        printf("Urutkan berdasarkan (1-5): ");
        attr = read_int_range(1, 5);

        printf("\n1. Ascending\n2. Descending\nUrutan (1-2): ");
        order = read_int_range(1, 2);

        memcpy(sorted_view, books, sizeof(Book) * (size_t)count);

        start = clock();
        if (algo == 1) {
            bubble_sort_books(sorted_view, count, attr, order == 1);
        } else if (algo == 2) {
            selection_sort_books(sorted_view, count, attr, order == 1);
        } else {
            insertion_sort_books(sorted_view, count, attr, order == 1);
        }
        end = clock();
        ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        sprintf(status, "%s Sort | %s | %s | waktu: %.2f ms",
                ALGO_NAMES[algo], ATTR_NAMES[attr],
                order == 1 ? "Ascending" : "Descending", ms);

        if (!browse_pages(count, status)) {
            break;
        }
    }
}