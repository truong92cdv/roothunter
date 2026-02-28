all:
	gcc -fno-stack-protector -z execstack -D_FORTIFY_SOURCE=0 -no-pie -m32 target.c -o target_m32
	gcc -fno-stack-protector -z execstack -D_FORTIFY_SOURCE=0 -no-pie -m64 target.c -o target_m64
	echo 0 > /proc/sys/kernel/randomize_va_space
clean:
	rm target_m32 target_m64
	echo 2 > /proc/sys/kernel/randomize_va_space
