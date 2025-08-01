# Доступные команды:
```
info
solve ax+b=c
gcd dig1 dig2
lcm dig1 dig2
```
# Цели:
1) ~~функция div~~ (v1.0.0)
2) полноценный ревью кода
3) фиксы багов (v1.0.0)


# Команды для запуска (yasm):
## Загрузчик
```
yasm -p gas -f bin -o bootsect.tmp bootsect.asm
dd bs=31744 skip=1 if=bootsect.tmp of=bootsect.bin
```
## Ядро
```
g++ -ffreestanding -m32 -o kernel.o -c kernel.cpp
ld --oformat binary -Ttext 0x10000 -o kernel.bin --entry=kmain -m elf_i386 kernel.o
```
## Эмулятор
```
qemu -fda bootsect.bin -fdb kernel.bin
```
___________________
```
g++ -ffreestanding -m32 -o kernel.o -c kernel.cpp | ld --oformat binary -Ttext 0x10000 -o kernel.bin --entry=kmain -m elf_i386 kernel.o | qemu -fda bootsect.bin -fdb kernel.bin
```
