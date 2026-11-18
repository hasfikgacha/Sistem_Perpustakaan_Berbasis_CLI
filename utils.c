#include <stdio.h>
#include "utils.h"
#include <string.h>

void clear_input_buffer(void){
    int ch;
    while ((ch = getchar ()) != '\n' && ch != EOF);
}
void read_string(char *buffer, int size)
     {
        size_t len;
        if (fgets(buffer, size, stdin) == NULL){
            buffer[0] = '\0';
            return;
        }
        len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }  else{
            clear_input_buffer();
        }
    }