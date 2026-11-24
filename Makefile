# Makefile - Sistem Manajemen Perpustakaan Berbasis CLI
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2

# Deteksi sistem operasi (Windows vs Linux/macOS)
ifeq ($(OS),Windows_NT)
    SHELL  = cmd.exe
    TARGET = perpustakaan.exe
    RUN    = .\$(TARGET)
    RM     = del /F /Q
    NULL   = 2>nul
else
    TARGET = perpustakaan
    RUN    = ./$(TARGET)
    RM     = rm -f
    NULL   =
endif

# Daftar berkas sumber dan objek
SRCS = main.c file_io.c utils.c auth.c sort.c admin.c user.c
OBJS = $(SRCS:.c=.o)

.PHONY: all run clean reset-db

all: $(TARGET)

# Tahap Linking
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Tahap Kompilasi per modul
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dependensi Header (otomatis recompile jika file .h diubah)
main.o:    main.c models.h file_io.h utils.h auth.h sort.h admin.h user.h
file_io.o: file_io.c file_io.h models.h
utils.o:   utils.c utils.h
auth.o:    auth.c auth.h file_io.h utils.h models.h
sort.o:    sort.c sort.h utils.h models.h
admin.o:   admin.c admin.h file_io.h sort.h utils.h models.h
user.o:    user.c user.h file_io.h sort.h utils.h models.h

# Jalankan program
run: $(TARGET)
	$(RUN)

# Hapus hasil kompilasi (.o dan executable)
clean:
	-$(RM) $(OBJS) $(TARGET) $(NULL)

# Hapus database operasional untuk reset cepat saat pengujian/demo
reset-db:
	-$(RM) books_db.txt user.txt peminjaman.txt $(NULL)