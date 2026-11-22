#include <stdio.h>
#include <string.h>
#include "auth.h"
#include "utils.h"

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

int interactive_auth_menu(const User users[], int user_count, User *out_user)
{
    int choice, attempt;
    char username[32];
    char password[32];
    const char *wanted_role;

    while (1) {
        clear_screen();
        printf("=== LOGIN PERPUSTAKAAN ===\n");
        printf("1. Login Admin\n2. Login Mahasiswa\n0. Keluar\nPilih: ");
        choice = read_int_range(0, 2);
        if (choice == 0) {
            return 0;
        }
        wanted_role = (choice == 1) ? "admin" : "user";

        for (attempt = 1; attempt <= 3; attempt++) {
            printf("Username: ");
            read_string(username, 32);
            printf("Password: ");
            read_string(password, 32);

            if (verify_credentials(users, user_count, username, password, out_user)
                && strcmp(out_user->role, wanted_role) == 0) {
                return 1;
            }
            printf("Login gagal (%d/3).\n", attempt);
        }
        press_enter_to_continue();
    }
}