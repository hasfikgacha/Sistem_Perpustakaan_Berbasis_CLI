#ifndef MODELS_H
#define MODELS_H

// Kapasitas maksimum array statis 
#define MAX_BOOKS 10500
#define MAX_USERS 500
#define MAX_LOANS 5000

// Ukuran maksimum buffer karakter per kolom 
#define MAX_LINE_LEN 2048
#define MAX_ID_LEN 64
#define MAX_TITLE_LEN 512
#define MAX_AUTHOR_LEN 512
#define MAX_META_LEN 64
#define MAX_URL_LEN 256

// Struktur data untuk setiap buku di dalam source file "Books.txt"
typedef struct {
    char ocaid[MAX_ID_LEN];
    char title[MAX_TITLE_LEN];
    char author[MAX_AUTHOR_LEN];
    int year;
    int page_count;
    char openlibrary_edition[MAX_META_LEN];
    char openlibrary_work[MAX_META_LEN];
    char full_text_url[MAX_URL_LEN];
    int stock;

} Book;

// Struktur data untuk akun pengguna/Users maupun admin secara sekaligus
typedef struct {
    char username[32];
    char password[32];
    char role[12];

} User;

typedef struct {
    unsigned int id_peminjaman;
    char username[32];
    char ocaid[64];
    char tanggal_pinjam[20]; // MM-DD-YY
    char status[20]; // penanda sudah dikembalikan atau belum
} Loan;


#endif