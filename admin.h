#ifndef ADMIN_H
#define ADMIN_H

#include "models.h"

int  find_book_index_by_id(const Book catalog[], int book_count, const char *ocaid);
void admin_create_book(Book catalog[], int *book_count, int max_capacity);
void admin_update_book(Book catalog[], int book_count);
void admin_delete_book(Book catalog[], int *book_count, const Loan loans[], int loan_count);
void admin_view_all_loans(const Loan loans[], int loan_count, const Book catalog[], int book_count);
void run_admin_session(Book catalog[], int *book_count, int max_capacity, Loan loans[], int loan_count);

#endif