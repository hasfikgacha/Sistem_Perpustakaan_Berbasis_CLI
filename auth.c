#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "auth.h"
#include "file_io.h"
#include "utils.h"

#define MENU_WIDTH 40

static int has_whitespace(const char *s)
{
    while (*s != '\0') {
        if (isspace((unsigned char)*s)) {
            return 1;
        }
        s++;
    }
    return 0;
}

static int username_exists(const User users[], int user_count,
                           const char *username)
{
    int i;
    for (i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return 1;
        }
    }
    return 0;
}

int verify_credentials(const User users[], int user_count,
                       const char *username, const char *password,
                       User *out_user)
{
    int i;
    for (i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0) {
            *out_user = users[i];
            return 1;
        }
    }
    return 0;
}

int login_via_cla(int argc, char *argv[], const User users[],
                  int user_count, User *out_user)
{
    int i;
    const char *username = NULL;
    const char *password = NULL;

    for (i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "-u") == 0) {
            username = argv[i + 1];
        } else if (strcmp(argv[i], "-p") == 0) {
            password = argv[i + 1];
        }
    }
    if (username == NULL || password == NULL) {
        return -1;
    }
    return verify_credentials(users, user_count, username, password, out_user);
}

int register_student(User users[], int *user_count, int max_users)
{
    char username[32];
    char password[32];

    printf("\n--- DAFTAR AKUN MAHASISWA ---\n");

    if (*user_count >= max_users) {
        printf("[!] Kapasitas akun sudah penuh. Hubungi admin.\n");
        return 0;
    }

    printf("Username baru (tanpa spasi, kosongkan untuk batal): ");
    read_string(username, sizeof(username));
    if (username[0] == '\0') {
        printf("Registrasi dibatalkan.\n");
        return 0;
    }
    if (has_whitespace(username)) {
        printf("[!] Username tidak boleh mengandung spasi.\n");
        return 0;
    }
    if (username_exists(users, *user_count, username)) {
        printf("[!] Username \"%s\" sudah terdaftar.\n", username);
        return 0;
    }

    printf("Password baru (tanpa spasi): ");
    read_string(password, sizeof(password));
    if (password[0] == '\0' || has_whitespace(password)) {
        printf("[!] Password tidak boleh kosong atau mengandung spasi.\n");
        return 0;
    }

    strcpy(users[*user_count].username, username);
    strcpy(users[*user_count].password, password);
    strcpy(users[*user_count].role, "user");
    (*user_count)++;

    if (save_users(users, *user_count)) {
        printf("[OK] Akun \"%s\" berhasil didaftarkan! Silakan login sebagai Mahasiswa.\n",
               username);
        return 1;
    }

    printf("[!] Akun gagal disimpan ke berkas %s.\n", DB_USER_FILE);
    return 0;
}

int interactive_auth_menu(User users[], int *user_count,
                          int max_users, User *out_user)
{
    int choice, attempt;
    char username[32];
    char password[32];
    const char *wanted_role;

    while (1) {
        clear_screen();
        putchar('\n');
        print_line('=', MENU_WIDTH);
        print_centered("LOGIN PERPUSTAKAAN", MENU_WIDTH);
        print_line('=', MENU_WIDTH);
        printf(" 1. Login Admin\n");
        printf(" 2. Login Mahasiswa\n");
        printf(" 3. Daftar Akun Mahasiswa\n");
        printf(" 0. Keluar\n");
        print_line('-', MENU_WIDTH);
        printf("Pilih menu [0-3]: ");

        choice = read_int_range(0, 3);
        if (choice == 0) {
            return 0;
        }
        if (choice == 3) {
            register_student(users, user_count, max_users);
            press_enter_to_continue();
            continue;
        }

        wanted_role = (choice == 1) ? "admin" : "user";
        printf("\n--- LOGIN %s ---\n",
               (choice == 1) ? "ADMIN" : "MAHASISWA");

        for (attempt = 1; attempt <= 3; attempt++) {
            printf("Username: ");
            read_string(username, sizeof(username));
            printf("Password: ");
            read_string(password, sizeof(password));

            if (verify_credentials(users, *user_count, username, password, out_user)
                && strcmp(out_user->role, wanted_role) == 0) {
                return 1;
            }
            printf("[!] Login gagal (%d/3).\n", attempt);
        }
        press_enter_to_continue();
    }
}