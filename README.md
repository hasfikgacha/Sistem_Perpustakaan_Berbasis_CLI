# Sistem Manajemen Perpustakaan Berbasis CLI

Aplikasi manajemen perpustakaan berbasis *Command-Line Interface* (CLI) yang ditulis dalam bahasa C (standar C11) secara modular. Proyek ini dikembangkan sebagai tugas akhir (UAS) mata kuliah **Praktikum Pemrograman Kelas A — Kelompok 5**.

Program mengelola katalog buku, akun pengguna, serta sirkulasi peminjaman dan pengembalian buku dengan penyimpanan data persisten berbasis berkas teks (`.txt`).

---

## Anggota Kelompok & Pembagian Tugas

| No | Nama | NIM | Pembagian Modul & Tugas |
| :-: | :--- | :--- | :--- |
| 1 | **Hasanul Fikri** | `260810701100013` | Struktur data (`models.h`), File I/O (`file_io.c/.h`), Integrasi (`main.c`), `Makefile`, & Pengujian |
| 2 | **Kaila Zuhra** | `260810701100033` | Modul Admin (`admin.c/.h`) — CRUD katalog buku & riwayat peminjaman |
| 3 | **Khalisah** | `260810701100021` | Modul Autentikasi (`auth.c/.h`), Algoritma Sorting (`sort.c/.h`), & Utilitas (`utils.c/.h`) |
| 4 | **Rajwa Ken Venona** | `260810701100028` | Modul Anggota/User (`user.c/.h`) — Sirkulasi peminjaman, pengembalian, & pencarian buku |

---

## Fitur Utama

### 1. Autentikasi & Manajemen Akun
* **Login Interaktif:** Mendukung sesi login terpisah untuk **Admin** dan **Mahasiswa (User)** dengan batas maksimal 3 kali percobaan.
* **Login via Command-Line Arguments (CLA):** Masuk langsung dari terminal tanpa melewati menu login menggunakan flag `-u` dan `-p`.
* **Daftar Akun Mahasiswa:** Pengguna baru dapat mendaftarkan akun `user` langsung dari halaman login, lengkap dengan validasi spasi dan pengecekan duplikasi username.

### 2. Menu Anggota (User)
* **Lihat Katalog Buku Tersedia:** Menampilkan buku dengan stok tersedia (`stock > 0`), dengan opsi tampil langsung (berhalaman) atau diurutkan terlebih dahulu menggunakan modul sorting.
* **Pencarian Buku (*Case-Insensitive*):** Mencari buku berdasarkan ID Buku (OCAID), judul, atau nama pengarang.
* **Pinjam Buku:** Validasi stok buku, pencegahan peminjaman ganda untuk buku yang sama, pencatatan tanggal pinjam otomatis (`DD-MM-YYYY`), dan pembaruan stok secara langsung.
* **Kembalikan Buku:** Menampilkan daftar pinjaman aktif pengguna, mengubah status transaksi menjadi `KEMBALI`, dan mengembalikan stok buku ke katalog.
* **Peminjaman Aktif & Riwayat Pribadi:** Melihat daftar buku yang sedang dipinjam maupun seluruh riwayat transaksi pribadi.

### 3. Menu Admin
* **Lihat & Urutkan Katalog Buku:** Menampilkan seluruh katalog buku dengan dukungan sorting dan paginasi.
* **Tambah Buku Baru:** Input buku baru dengan validasi ID (OCAID) unik, tanpa spasi, serta validasi angka untuk tahun terbit, jumlah halaman, dan stok awal.
* **Ubah Data Buku:** Memperbarui judul, pengarang, tahun terbit, jumlah halaman, atau menambah/mengurangi stok secara selektif.
* **Hapus Buku:** Menghapus buku dari katalog dengan proteksi agar buku yang berstatus `DIPINJAM` tidak dapat dihapus.
* **Riwayat Seluruh Peminjaman:** Menampilkan tabel log transaksi seluruh pengguna beserta status peminjamannya.

### 4. Algoritma Sorting & Paginasi
Katalog buku dapat diurutkan (menaik/menurun) berdasarkan kriteria tertentu menggunakan tiga pilihan algoritma:
* **Bubble Sort**
* **Selection Sort**
* **Insertion Sort**

---

## Struktur Proyek

```text
Sistem_Perpustakaan_Berbasis_CLI/
├── main.c          # Titik masuk program & orkestrasi alur sesi
├── models.h        # Definisi struct (Book, User, Loan) & konstanta global
├── file_io.c / .h  # Inisialisasi, baca, dan simpan database (.txt)
├── auth.c / .h     # Verifikasi kredensial, login CLA, menu login, & registrasi
├── admin.c / .h    # Fitur CRUD buku & pemantauan log peminjaman admin
├── user.c / .h     # Fitur katalog, pencarian, pinjam, kembali, & riwayat user
├── sort.c / .h     # Implementasi Bubble, Selection, Insertion Sort & paginasi
├── utils.c / .h    # Helper input aman, pembersih layar, & format tampilan UI
├── Makefile        # Otomatisasi kompilasi lintas OS (Windows & Linux/macOS)
└── Books.txt       # Dataset master katalog buku (prasyarat saat dijalankan)
```

---

## Persyaratan

* Kompiler **GCC** yang mendukung standar **C11** (Windows: MSYS2/MinGW-w64; Linux/macOS: GCC bawaan).
* **Make** (`mingw32-make` di Windows atau `make` di Linux/macOS) — opsional, bisa juga kompilasi manual dengan `gcc`.
* Berkas dataset master **`Books.txt`** harus berada di direktori utama proyek sebelum program dijalankan pertama kali.

---

## Cara Kompilasi & Menjalankan

### Menggunakan Makefile

**Windows (PowerShell / CMD dengan MinGW/MSYS2):**
```powershell
mingw32-make          # Kompilasi seluruh modul menjadi perpustakaan.exe
mingw32-make run      # Kompilasi (jika perlu) lalu langsung jalankan program
mingw32-make clean    # Hapus berkas objek (.o) dan perpustakaan.exe
mingw32-make reset-db # Hapus database operasional untuk reset data uji
```

**Linux / macOS:**
```bash
make                  # Kompilasi seluruh modul menjadi ./perpustakaan
make run              # Kompilasi (jika perlu) lalu langsung jalankan program
make clean            # Hapus berkas objek (.o) dan ./perpustakaan
make reset-db         # Hapus database operasional untuk reset data uji
```

### Kompilasi Manual (Tanpa Make)
```bash
gcc -Wall -Wextra -std=c11 -O2 main.c file_io.c utils.c auth.c sort.c admin.c user.c -o perpustakaan
```
*(Di Windows, ubah output menjadi `-o perpustakaan.exe` lalu jalankan dengan `.\perpustakaan.exe`.)*

### Menjalankan dengan Login CLA (Opsional)
```bash
# Windows
.\perpustakaan.exe -u admin -p admin123

# Linux / macOS
./perpustakaan -u mahasiswa -p mhs123
```

---

## Akun Default

Jika berkas `user.txt` belum tersedia saat pertama kali dijalankan, program akan membuatnya secara otomatis dengan dua akun bawaan:

| Role | Username | Password |
| :--- | :--- | :--- |
| **Admin** | `admin` | `admin123` |
| **Mahasiswa (User)** | `mahasiswa` | `mhs123` |

---

## Penyimpanan Data

Program memisahkan dataset master dari berkas operasional agar data asli tetap aman:
* **`Books.txt`** — Dataset master katalog buku (hanya dibaca saat inisialisasi awal, tidak pernah ditimpa).
* **`books_db.txt`** — Salinan operasional katalog buku yang menyimpan seluruh perubahan stok dan CRUD.
* **`user.txt`** — Daftar akun terdaftar dengan format `username password role`.
* **`peminjaman.txt`** — Log seluruh transaksi peminjaman dan pengembalian buku.
