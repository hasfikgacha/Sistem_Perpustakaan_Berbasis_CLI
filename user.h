#ifndef USER_H
#define USER_H

#include "models.h"

/* Menampilkan semua buku yang stoknya > 0. */
void user_view_available_books(const Book catalog[], int book_count);

/* Meminta kata kunci lalu mencari buku berdasarkan ID, judul, atau pengarang. */
void user_search_books(const Book catalog[], int book_count);

/*
 * Meminjam buku: memvalidasi stok dan duplikasi pinjaman aktif, mengurangi
 * stok, mencatat transaksi baru, lalu menyimpan ke berkas.
 * loan_count dikirim sebagai pointer karena nilainya bertambah.
 * loan_capacity = ukuran array loans (batas maksimum data peminjaman).
 */
void user_borrow_book(Book catalog[], int book_count, Loan loans[],
                      int *loan_count, int loan_capacity,
                      const User *current_user);

/* Mengembalikan buku: status menjadi KEMBALI dan stok bertambah 1. */
void user_return_book(Book catalog[], int book_count, Loan loans[],
                      int loan_count, const User *current_user);

/*
 * Menampilkan peminjaman milik current_user.
 * only_active = 1 -> hanya DIPINJAM; 0 -> seluruh riwayat.
 * Mengembalikan jumlah baris yang ditampilkan.
 */
int user_view_personal_loans(const Book catalog[], int book_count,
                             const Loan loans[], int loan_count,
                             const User *current_user, int only_active);

/* Menu interaktif User; berjalan sampai pengguna memilih Logout. */
void run_user_session(const User *current_user, Book catalog[], int book_count,
                      Loan loans[], int *loan_count, int loan_capacity);

#endif /* USER_H */