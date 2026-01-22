all : nice.c insn.c bootstrap.S
	riscv64-unknown-elf-gcc nice.c insn.c bootstrap.S -o main -march=rv32i -mabi=ilp32 -nostartfiles -Wl,--no-relax

sim: all
	/home/x/riscv-vp-mine/vp/build/bin/tiny32-vp --intercept-syscalls main

pwt: all
	/home/x/riscv-vp-mine/vp/build/bin/tiny32-vp --intercept-syscalls --quiet --libtlmpwt main

nvmain: all
	/home/x/riscv-vp-mine/vp/build/bin/tiny32-vp --intercept-syscalls --quiet --nvmain main

	@echo "======================================RRAM=========================================" > nvmain_report
	../../NVmain/bin/nvmain.fast ../../NVmain/Config/RRAM_ISSCC_2012_4GB.config nvmain.trace 0  >>nvmain_report
	@echo "======================================STT-MRAM=====================================" >>nvmain_report
	../../NVmain/bin/nvmain.fast ../../NVmain/Config/STTRAM_Everspin_4GB.config nvmain.trace 0  >>nvmain_report
	@echo "======================================PCRAM========================================" >>nvmain_report
	../../NVmain/bin/nvmain.fast ../../NVmain/Config/PCM_ISSCC_2012_4GB.config nvmain.trace 0   >>nvmain_report
	@echo "======================================2D_DRAM======================================" >>nvmain_report
	../../NVmain/bin/nvmain.fast ../../NVmain/Config/2D_DRAM.config nvmain.trace 0              >>nvmain_report
	@echo "======================================3D_DRAM======================================" >>nvmain_report
	../../NVmain/bin/nvmain.fast ../../NVmain/Config/3D_DRAM.config nvmain.trace 0 	            >>nvmain_report

	python3 ../../NVmain/bin/nvmain_power_process.py

dump-elf: all
	riscv32-unknown-elf-readelf -a main > dump-elf.txt

dump-code: all
	riscv32-unknown-elf-objdump -D main > dump-code.txt

dump-comment: all
	objdump -s --section .comment main > dump-comment.txt

clean:
	rm -f main dump-elf.txt dump-code.txt dump-comment.txt atmi_solver_file.atmi temperature.data nvmain.trace nvmain_report
