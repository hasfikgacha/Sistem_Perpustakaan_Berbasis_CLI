#ifndef FILE_IO_H
#define FILE_IO_H

#include "models.h"

/* Fungsi inisialisasi dan sinkronisasi data buku */
int init_and_load_books(Book catalog[], int max_capacity);
int save_books_to_db(const Book catalog[], int book_count);

#endif /* FILE_IO_H */