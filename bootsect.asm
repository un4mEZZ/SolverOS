.code16
.org 0x7c00

_start:
	cli
	movw %cs, %ax #запись адреса сегмента
	movw %ax, %ds #запись адреса сегмена в качестве значения, указывающего на начало сегмента данных
	movw %ax, %ss #сохранение в качестве сегмета стека
	movw $_start, %sp #сохранение стека в качестве адреса первой инструкции код, после чего стэк будет расти вверх.
	sti
	
	movb $0x01, %ch
output:
	movw $0x0003, %ax #очистить экран
	int $0x10
	movw $0x0e, %ax
	
	cmp $0x01, %ch
	je print_gray
	cmp $0x02, %ch
	je print_white
	cmp $0x03, %ch
	je print_yellow
	cmp $0x04, %ch
	je print_cian
	cmp $0x05, %ch
	je print_magenta
	cmp $0x06, %ch
	je print_green

input:
	movb $0x00, %ah
	int $0x16
	cmp $0x50, %ah
	je up
	cmp $0x48, %ah
	je down
	cmp $0x1c, %ah
	je kernel
	jmp input

up:
	addb $0x01, %ch
	cmp $7, %ch
	je mod_up
	jmp output
down:
	subb $0x01, %ch
	cmp $0, %ch
	je mod_down
	jmp output

mod_up:
	movb $1, %ch
	jmp output
	
mod_down:
	movb $6, %ch
	jmp output
	
print_gray:
	movw $gray, %bx
	call puts
	jmp input
print_white:
	movw $white, %bx
	call puts
	jmp input
print_yellow:
	movw $yellow, %bx
	call puts
	jmp input
print_cian:
	movw $cian, %bx
	call puts
	jmp input
print_magenta:
	movw $magenta, %bx
	call puts
	jmp input
print_green:
	movw $green, %bx
	call puts
	jmp input

puts:
	movb 0(%bx), %al
	test %al, %al
	jz end_puts
	movb $0x0e, %ah
	int $0x10
	addw $1, %bx
	jmp puts
end_puts:
	ret

gray:
	.asciz "Gray\n\rW\n\rY\n\rC\n\rM\n\rG"
white:
	.asciz "G\n\rWhite\n\rY\n\rC\n\rM\n\rG"
yellow:
	.asciz "G\n\rW\n\rYellow\n\rC\n\rM\n\rG"
cian:
	.asciz "G\n\rW\n\rY\n\rCian\n\rM\n\rG"
magenta:
	.asciz "G\n\rW\n\rY\n\rC\n\rMagenta\n\rG"
green:
	.asciz "G\n\rW\n\rY\n\rC\n\rM\n\rGreen"
	
kernel:
	movb %ch, 0x3000
	
	movw $0x0003, %ax #очистка экрана
	int $0x10
	
	movw $0x1000, %ax #addres of kernel
	movw %ax, %es
	movw $0x0000, %bx
	movb $0x28, %al #сколько секторов считать
	movb $0x01, %cl #номер сегмента
	movb $0x00, %ch #номер цилиндра
	movb $0x00, %dh #номер головки
	movb $0x01, %dl #номер диска

	movb $0x02, %ah
	int $0x13

	cli
	lgdt gdt_info
	inb $0x92, %al
	orb $2, %al
	outb %al, $0x92
	movl %cr0, %eax
	orb $1, %al
	movl%eax, %cr0
	ljmp $0x8, $protected_mode

.code32

protected_mode:
	movw $0x10, %ax
	movw %ax, %es
	movw %ax, %ds
	movw %ax, %ss
	call 0x10000 #вызвать ядро

gdt_info:
	.word gdt_info -gdt
	.word gdt, 0

gdt:
	.byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00
	.byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00

.zero (512 -(. - _start) - 2)
.byte 0x55, 0xAA
