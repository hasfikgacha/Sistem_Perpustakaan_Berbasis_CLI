#ifndef UTILS_H
#define UTILS_H

void clear_input_buffer(void);
void read_string(char *buffer, int size);
int read_int_range(int min, int max);
void press_enter_to_continue(void);
void clear_screen(void);

/* Helper tampilan bersama (Admin & User) */
void print_line(char ch, int width);
void print_centered(const char *text, int width);

#endif