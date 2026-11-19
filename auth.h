#ifndef AUTH_H
#define AUTH_H

#include "models.h"

/* 1 jika cocok (hasil disalin ke out_user), 0 jika tidak */
int verify_credentials(const User users[], int user_count,
                       const char *username, const char *password,
                       User *out_user);

/* Pakai: program -u nama -p sandi
   Return: 1 berhasil, 0 kredensial salah, -1 tidak ada argumen login */
int login_via_cla(int argc, char *argv[], const User users[],
                  int user_count, User *out_user);

/* Return 1 jika login berhasil, 0 jika pengguna keluar */
int interactive_auth_menu(const User users[], int user_count, User *out_user);

#endif