ASM=nasm
LD=ld

all: ld-linux-1

ld-linux-1: ld-linux-1.asm
	$(ASM) -f elf ld-linux-1.asm
	$(LD) -m elf_i386 -o ld-linux-1 ld-linux-1.o 

install: all
	install -d $(DESTDIR)/lib
	install -m755 ld-linux-1 $(DESTDIR)/lib/ld-linux.so.1

clean:
	-rm ld-linux-1 ld-linux-1.o

.PHONY: clean all
