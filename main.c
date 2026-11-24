#include <stdio.h>
#include <string.h>
#include "models.h"
#include "file_io.h"
#include "utils.h"
#include "auth.h"
#include "sort.h"
#include "admin.h"
#include "user.h"

/* Array statis di Data Segment agar aman menampung >10.000 struct tanpa Stack Overflow */
static Book catalog[MAX_BOOKS];
static User users[MAX_USERS];
static Loan loans[MAX_LOANS];

int main(int argc, char *argv[]) {
    int book_count = 0;
    int user_count = 0;
    int loan_count = 0;
    User current_user;
    int cla_logged_in = 0;

    /* 1. Inisialisasi dan muat seluruh berkas database (.txt) ke dalam memori */
    book_count = init_and_load_books(catalog, MAX_BOOKS);
    if (book_count < 0) {
        return 1;
    }
    user_count = load_users(users, MAX_USERS);
    loan_count = load_loans(loans, MAX_LOANS);

    /* 2. Cek apakah pengguna melakukan login langsung melalui Command Line Arguments (CLA) */
    if (argc > 1) {
        cla_logged_in = login_via_cla(argc, argv, users, user_count, &current_user);
        if (cla_logged_in == 0) {
            printf("[!] Login via CLA gagal (username/password salah).\n");
            press_enter_to_continue();
        } else if (cla_logged_in == -1) {
            printf("[!] Format argumen salah. Gunakan: %s -u <username> -p <password>\n", argv[0]);
            press_enter_to_continue();
        }
    }

    /* 3. Loop utama navigasi berdasarkan Role */
    while (1) {
        /* Cek == 1 secara eksplisit agar return -1 tidak melewati login */
        if (cla_logged_in != 1) {
            if (!interactive_auth_menu(users, &user_count, MAX_USERS, &current_user)) {
                printf("\nTerima kasih telah menggunakan Sistem Manajemen Perpustakaan!\n");
                break;
            }
        }

        if (strcmp(current_user.role, "admin") == 0) {
            run_admin_session(catalog, &book_count, MAX_BOOKS, loans, loan_count);
        } else {
            run_user_session(&current_user, catalog, book_count, loans, &loan_count, MAX_LOANS);
        }

        /* Setelah logout dari sesi CLA, kembalikan ke gerbang login interaktif */
        cla_logged_in = 0;
    }

    return 0;
}