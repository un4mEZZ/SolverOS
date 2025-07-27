Команды:

yasm -p gas -f bin -o bootsect.tmp bootsect.asm
dd bs=31744 skip=1 if=bootsect.tmp of=bootsect.bin

g++ -ffreestanding -m32 -o kernel.o -c kernel.cpp
ld --oformat binary -Ttext 0x10000 -o kernel.bin --entry=kmain -m elf_i386 kernel.o

qemu -fda bootsect.bin -fdb kernel.bin

g++ -ffreestanding -m32 -o kernel.o -c kernel.cpp | ld --oformat binary -Ttext 0x10000 -o kernel.bin --entry=kmain -m elf_i386 kernel.o | qemu -fda bootsect.bin -fdb kernel.bin