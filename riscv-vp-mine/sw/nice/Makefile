all : nice.c insn.c bootstrap.S
	riscv64-unknown-elf-gcc nice.c insn.c bootstrap.S -o main -march=rv32i -mabi=ilp32 -nostartfiles -Wl,--no-relax
	
sim: all
	/home/x/riscv-vp-mine/vp/build/bin/tiny32-vp --intercept-syscalls main
	
dump-elf: all
	riscv32-unknown-elf-readelf -a main > dump-elf.txt
	
dump-code: all
	riscv32-unknown-elf-objdump -D main > dump-code.txt
	
dump-comment: all
	objdump -s --section .comment main > dump-comment.txt
	
clean:
	rm -f main dump-elf.txt dump-code.txt dump-comment.txt
