#ifndef SORT_H
#define SORT_H

#include "models.h"

#define SORT_BY_OCAID      1
#define SORT_BY_TITLE      2
#define SORT_BY_YEAR       3
#define SORT_BY_PAGE_COUNT 4
#define SORT_BY_STOCK      5

void bubble_sort_books(Book books[], int count, int attribute, int ascending);
void selection_sort_books(Book books[], int count, int attribute, int ascending);
void insertion_sort_books(Book books[], int count, int attribute, int ascending);
void interactive_sort_and_view_catalog(const Book books[], int count);

#endif