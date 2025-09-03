__asm("jmp kmain");

#define COLOUR_ADDRESS (0x3000)
#define ERR_CLR (0x0c)
#define VIDEO_BUF_PTR (0xb8000) 
#define IDT_TYPE_INTR (0x0E)
#define IDT_TYPE_TRAP (0x0F)

#define GDT_CS (0x8)
#define PIC1_PORT (0x20)
#define CURSOR_PORT (0x3D4)
#define VIDEO_WIDTH (80)
#define VIDEO_LENGTH (25)

#define MAX_INT (2147483647)
#define MIN_INT (-2147483648)

struct idt_entry
{
	unsigned short base_lo;		// Младшие биты адреса обработчика
	unsigned short segm_sel;	// Селектор сегмента кода
	unsigned char always0;		// Этот байт всегда 0
	unsigned char flags;		// Флаги
	unsigned short base_hi;
} __attribute__((packed));		// Выравнивание запрещено

// Структура, адрес которой передается как аргумент команды lidt
struct idt_ptr
{
	unsigned short limit;
	unsigned int base;
} __attribute__((packed));		// Выравнивание запрещено

typedef void (*intr_handler)();

struct idt_entry g_idt[256];		// Реальная таблица IDT
struct idt_ptr g_idtp;			// Описатель таблицы для команды lidt
	
unsigned int global_str = 0;
unsigned int global_pos = 0;
bool shift = false;

void on_key(unsigned char scan_code);
void commands();
bool strcmp(unsigned char *str1, const char *str2);
int strlen(unsigned char *str);

void info();
void gcd(unsigned char* str);
void solve(unsigned char* str);
void lcm(unsigned char* str);
void div(unsigned char* str);
int char_to_int(unsigned char *str);
unsigned char *int_to_char(int integer);
void clean();
void scroll();
bool check_of(int num, char* str_num);
bool check_of_sum(int num1, int num2);
int nod(int m, int n);

void shift_check(unsigned char scan_code);
void clean(bool chk);
void backspace();
void tab();
void enter();
void symbol(unsigned char scan_code);

void default_intr_handler();
void intr_reg_handler(int num, unsigned short segm_sel, unsigned short flags, intr_handler hndlr);
void intr_init();
void intr_start();
void intr_enable();
void intr_disable();
void out_str(int color, const char* ptr, unsigned int strnum);
void out_char(int color, unsigned char simbol);
void out_word(int color, const char* ptr);
void out_num(int num);
static inline void outw (unsigned int port, unsigned int data);
static inline unsigned char inb (unsigned short port);
static inline void outb (unsigned short port, unsigned char data);
void keyb_init();
void keyb_handler();
void keyb_process_keys();
void cursor_move_to(unsigned int strnum, unsigned int pos);

char scan_codes[] = {
    0, 
    0,
    '1','2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    0,
    0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
    0,
    0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '<','>','+',
    0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,
    '*',
    0,
    ' ',
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,
    0,
    0,
    0,
    0,
    '-',
    0, 0,
    0,
    '+',
    0,
    0 };


char shift_char[] = {
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '=', '8',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '+', '*' };

char currentColour = 0x08;
char colours[] = { 0x07, 0x0f, 0x0e, 0x0b, 0x0d, 0x0a };

void clean(){	
	unsigned char *video_buf = (unsigned char*) VIDEO_BUF_PTR;
	for (int i = 0; i < VIDEO_WIDTH * VIDEO_LENGTH; i++){
		*(video_buf + i*2) = '\0';
	}
	global_str = 0;
	global_pos = 0;
	out_str(currentColour, "# ", global_str);
	global_pos = 2;
	cursor_move_to(global_str, global_pos);
}

void scroll(unsigned int strnm){
	unsigned char *video_buf = (unsigned char*) (VIDEO_BUF_PTR + VIDEO_WIDTH * 2);
	unsigned char *video_buf_new = (unsigned char*) VIDEO_BUF_PTR;
	for (int i = 0; i < VIDEO_WIDTH * VIDEO_LENGTH * 2; i++){
		*(video_buf_new + i) = *(video_buf + i);
	}
	for (int i = VIDEO_WIDTH * (VIDEO_LENGTH - 1) * 2; i < VIDEO_WIDTH * VIDEO_LENGTH * 2; i++){
		*(video_buf + i) = '\0';
	}
	global_str = VIDEO_LENGTH - 1;
	strnm = VIDEO_LENGTH - 1;
	cursor_move_to(global_str, global_pos);
}

extern "C" int kmain(){
	currentColour = colours[(*(unsigned char*)COLOUR_ADDRESS) - 1];
	clean();

	intr_disable();
	intr_init();
	keyb_init();
	intr_start();
	intr_enable();
	
	while(1){
		asm("hlt");
	}

	return 0;
}

void intr_reg_handler(int num, unsigned short segm_sel, unsigned short flags, intr_handler hndlr){
	unsigned int hndlr_addr = (unsigned int) hndlr;

	g_idt[num].base_lo = (unsigned short) (hndlr_addr & 0xFFFF);
	g_idt[num].segm_sel = segm_sel;
	g_idt[num].always0 = 0;
	g_idt[num].flags = flags;
	g_idt[num].base_hi = (unsigned short) (hndlr_addr >> 16);
}

void default_intr_handler(){
	asm("pusha");
	// ... (реализация обработки)
	asm("popa; leave; iret");
}

// Функция инициализации системы прерываний: 
// заполнение массива с адресами обработчиков
void intr_init(){
	int i;
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);

	for(i = 0; i < idt_count; i++)
		intr_reg_handler(i, GDT_CS, 0x80 | IDT_TYPE_INTR,
			default_intr_handler);
}


void intr_start(){
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);

	g_idtp.base = (unsigned int) (&g_idt[0]);
	g_idtp.limit = (sizeof (struct idt_entry) * idt_count) - 1;

	asm("lidt %0" : : "m" (g_idtp) );
}

void intr_enable(){
	asm("sti");
}

void intr_disable(){
	asm("cli");
}


static inline unsigned char inb (unsigned short port) {
	unsigned char data;
	asm volatile ("inb %w1, %b0" : "=a" (data) : "Nd" (port));
	return data;
}


static inline void outb (unsigned short port, unsigned char data){
	asm volatile ("outb %b0, %w1" : : "a" (data), "Nd" (port));
}

static inline void outw (unsigned int port, unsigned int data){
	asm volatile ("outw %w0, %w1" : : "a" (data), "Nd" (port));
}

void keyb_init(){
	// Регистрация обработчика прерывания
	intr_reg_handler(0x09, GDT_CS, 0x80 | IDT_TYPE_INTR, keyb_handler);
	// Разрешение только прерываний клавиатуры от контроллера 8259
	outb(PIC1_PORT + 1, 0xFF ^ 0x02);
// 0xFF - все прерывания, 0x02 -бит IRQ1 (клавиатура).
// Разрешены будут только прерывания, чьи биты установлены в 0
}

void keyb_handler(){
	asm("pusha");
	// Обработка поступивших данных
	keyb_process_keys();
// Отправка контроллеру 8259 нотификации о том, что прерывание обработано
	outb(PIC1_PORT, 0x20);
	asm("popa; leave; iret");
}

// считывает поступивший от пользователя символ
void keyb_process_keys(){
// Проверка что буфер PS/2 клавиатуры не пуст (младший бит присутствует)
	if (inb(0x64) & 0x01){
		unsigned char scan_code;
		unsigned char state;
		scan_code = inb(0x60);	// Считывание символа с PS/2 клавиатуры
		if (scan_code < 128)	// Скан-коды выше 128 - это отпускание клавиши
			on_key(scan_code);
		else shift_check(scan_code);
	}
}

// Функция переводит курсор на строку strnum (0 – самая верхняя) в позицию
// pos на этой строке (0 – самое левое положение).
void cursor_move_to(unsigned int strnum, unsigned int pos)
{
	unsigned short new_pos = (strnum * VIDEO_WIDTH) + pos;
	outb(CURSOR_PORT, 0x0F);
	outb(CURSOR_PORT + 1, (unsigned char)(new_pos & 0xFF));
	outb(CURSOR_PORT, 0x0E);
	outb(CURSOR_PORT + 1, (unsigned char)( (new_pos >> 8) & 0xFF));
}

void out_str(int color, const char* ptr, unsigned int strnum){
	if (strnum >= VIDEO_LENGTH){
		char nm = VIDEO_LENGTH - strnum + 1;
		for (int i = 0; i < nm; i++) scroll(strnum);
		strnum = VIDEO_LENGTH - 1;
	}
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf += VIDEO_WIDTH * 2 * strnum;
	while (*ptr){
		video_buf[0] = (unsigned char) *ptr;
		video_buf[1] = color;
		video_buf += 2;
		ptr++;
	}
}

void out_word(int color, const char* ptr){
	if (global_str >= VIDEO_LENGTH){
		char nm = VIDEO_LENGTH - global_str + 1;
		for (int i = 0; i < nm; i++) scroll(global_str);
	}
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf += 2*(VIDEO_WIDTH * global_str + global_pos);
	while (*ptr){
		video_buf[0] = (unsigned char) *ptr;
		video_buf[1] = color;
		video_buf += 2;
		global_pos++;
		ptr++;
	}
}

void out_char(int color, unsigned char simbol){
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf += 2*(global_str * VIDEO_WIDTH + global_pos);
	video_buf[0] = simbol;
	video_buf[1] = color;
	cursor_move_to(global_str, ++global_pos);
}

void shift_check(unsigned char scan_code){
	if (scan_code == 170 || scan_code == 182)
		shift = false;
}

void on_key(unsigned char scan_code){
	if (scan_code == 14) backspace();
	else if (scan_code == 28) enter();
	else if (scan_code == 42 || scan_code == 54)
		shift = true;
	else if (global_pos < 42 && scan_codes[scan_code] != 0) symbol(scan_code);
}


void backspace(){
	if (global_pos > 2){
		unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
		video_buf += 2*(global_str*VIDEO_WIDTH + global_pos - 1);
		video_buf[0] = '\0';
		cursor_move_to(global_str, --global_pos);
	}
}

void enter(){
	commands();
	out_str(currentColour, "# ", ++global_str);
	global_pos = 2;
	cursor_move_to(global_str, global_pos);
}


void symbol(unsigned char scan_code){
	char c = scan_codes[(unsigned int)scan_code];
	if (shift == false) out_char(currentColour, c);
	else
	{
		int i;
		for (i = 0; i < 28; i++)
			if (c == shift_char[i])
			break;
		out_char(currentColour, shift_char[i + 28]);
	}
}

unsigned char* lower(unsigned char* str){
	unsigned char* temp = str;
	for (int i = 0; i < strlen(temp); i+=2)
		for (int j = 28; j < 54; j++)
			if (str[i] == shift_char[j]) temp[i] = shift_char[j % 28];
	return temp;
}


void commands(){
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf += 2 * (global_str * VIDEO_WIDTH + 2);

	if (strcmp(lower(video_buf), "info")) info();
	else if (strcmp(lower(video_buf), "gcd ")) gcd(video_buf + 8);
	else if (strcmp(lower(video_buf), "solve ")) solve(video_buf + 12);
	else if (strcmp(lower(video_buf), "lcm ")) lcm(video_buf + 8);
	else if (strcmp(lower(video_buf), "div ")) div(video_buf + 8);
	else if (strcmp(lower(video_buf), "shutdown")) outw(0x604, 0x2000);
	else out_str(ERR_CLR, "Error: command incorrect", ++global_str);
}

bool strcmp(unsigned char *str1, const char *str2){
    while (*str1 != '\0' && *str1 != ' ' && *str2 != '\0' && *str1 == *str2){
      str1+=2;
      str2++;
    }

   if (*str1 == *str2) return true; 
   return false;
}

int strlen(unsigned char *str){
	int i = 0;
	while(*str != '\0') { str++; i++;}
	return i;
}

void info(){
	global_pos = 0;
	out_str(currentColour, "Version 1.0.5 debug (fixed solve result OF)", ++global_str);

	out_str(currentColour, "..######...#######..##.......##.....##.########.########...#######...######.", ++global_str);
	out_str(currentColour, ".##....##.##.....##.##.......##.....##.##.......##.....##.##.....##.##....##", ++global_str);
	out_str(currentColour, ".##.......##.....##.##.......##.....##.##.......##.....##.##.....##.##......", ++global_str);
	out_str(currentColour, "..######..##.....##.##.......##.....##.######...########..##.....##..######.", ++global_str);
	out_str(currentColour, ".......##.##.....##.##........##...##..##.......##...##...##.....##.......##", ++global_str);
	out_str(currentColour, ".##....##.##.....##.##.........##.##...##.......##....##..##.....##.##....##", ++global_str);
	out_str(currentColour, "..######...#######..########....###....########.##.....##..#######...######.", ++global_str);


	out_str(currentColour, " ", ++global_str);
	out_str(currentColour, "Assembler: YASM (AT&T)", ++global_str);
	out_str(currentColour, "OS:        Linux", ++global_str);
	switch(currentColour) {
		case 0x07: { out_str(currentColour, "Color:     gray", ++global_str); break; }
		case 0x0f: { out_str(currentColour, "Color:     white", ++global_str); break; }
		case 0x0e: { out_str(currentColour, "Color:     yellow", ++global_str); break; }
		case 0x0b: { out_str(currentColour, "Color:     cian", ++global_str); break; }
		case 0x0d: { out_str(currentColour, "Color:     magenta", ++global_str); break; }
		case 0x0a: { out_str(currentColour, "Color:     green", ++global_str); break; }
	}
	out_str(currentColour, "Author:    Konstantin Pinegin, gr. 5131001/30001", ++global_str);
}

int char_to_int(unsigned char *str){
	int res = 0;
	int size = strlen(str);
	if (str[0]=='-'){
		if (size == 1) res = 1;
		for (int i = 1; i < size; i++){
			res = res * 10 + (str[i] - '0');
		}
		res = 0 - res;
	}
	else if (str[0]=='+'){
		if (size == 1) res = 1;
		for (int i = 1; i < size; i++){
			res = res * 10 + (str[i] - '0');
		}
	}
	else{
		if (size == 0) res = 1;
		for (int i = 0; i < size; i++){
			res = res * 10 + (str[i] - '0');
		}
	}
	return res;
}

unsigned int char_to_uint(unsigned char *str){
	unsigned int res = 0;
	int sz = strlen(str);
	if (str[0]=='-'){
		if (sz == 1) res = 1;
		for (int i = 1; i < sz; i ++){
			res = res * 10 + (str[i] - '0');
		}
		res = 0 - res;
	}
	else if (str[0]=='+'){
		if (sz == 1) res = 1;
		for (int i = 1; i < sz; i ++){
			res = res * 10 + (str[i] - '0');
		}
	}
	else{
		if (sz == 0) res = 1;
		for (int i = 0; i < sz; i ++){
			res = res * 10 + (str[i] - '0');
		}
	}
	return res;
}

unsigned char *int_to_char(int value) {
    static unsigned char buffer[12];
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return buffer;
    }
    int is_neg = (value < 0);
    unsigned int abs_value = is_neg ? -value : value;
    int i = 0;
    do {
        buffer[i++] = '0' + (abs_value % 10);
        abs_value /= 10;
    } while (abs_value > 0);
    if (is_neg) buffer[i++] = '-';
    buffer[i] = '\0';
    // Реверс строки
    for (int j = 0; j < i / 2; j++) {
        unsigned char tmp = buffer[j];
        buffer[j] = buffer[i - 1 - j];
        buffer[i - 1 - j] = tmp;
    }
    return buffer;
}

bool strcmp_s(unsigned char* str1, unsigned char* str2){
	while (*str1 != '\0' && *str2 != '\0' && *str1 == *str2) 
    {
      str1++;
      str2++;
    }
   if (*str1 == *str2) return 1; 
   return 0;
}

int check_of(int value, unsigned char *str) {
    unsigned char *int_char = int_to_char(value);
    // Дебаг
    global_pos = 0;
    global_str++;
    out_word(currentColour, "Debug: check_of: value=");
    out_word(currentColour, (const char*)int_char);
    out_word(currentColour, ", expected_string=");
    out_word(currentColour, (const char*)str);
    out_word(currentColour, ", result=");
    bool comp = strcmp_s(int_char, str);
    out_word(currentColour, strcmp_s(int_char, str) ? "0" : "1");
    out_word(currentColour, "\n");
    return comp;
}

int check_of_signed(int value, unsigned char *str) { //5, '+5'
    unsigned char *int_char = int_to_char(value); //'5'
    if (*str == '+') {
	int len = strlen(str); //len=2
	str = str+1;
    }

    // Дебаг
    global_pos = 0;
    global_str++;
    out_word(currentColour, "Debug: check_of: value=");
    out_word(currentColour, (const char*)int_char);
    out_word(currentColour, ", expected_string=");
    out_word(currentColour, (const char*)str);
    out_word(currentColour, ", result=");
    bool comp = strcmp_s(int_char, str);
    out_word(currentColour, strcmp_s(int_char, str) ? "0" : "1");
    out_word(currentColour, "\n");
    return comp;
}

bool check_of_sum(int num1, int num2){
	if (num1 >= 0 && num2 >= 0) return (num1 > MAX_INT - num2);
	if (num1 < 0 && num2 < 0) return (num1 < MIN_INT - num2);
	return 0;
}

// Вспомогательная функция для валидации всей строки
int validate_str(unsigned char *s) {
    for (int k = 0; s[k*2]; k++) {
        unsigned char c = s[k*2];
        if (!(c >= '0' && c <= '9') && c != '-' && c != '+' && c != '=' && c != 'x') return 0;
    }
    return 1;
}

// Вспомогательная функция для валидации числа
int validate_num(unsigned char *num) {
    for (int k = 0; num[k]; k++) {
        if (k == 0 && (num[k] == '+' || num[k] == '-')) continue;
        if (!(num[k] >= '0' && num[k] <= '9')) return 0; // Только цифры после знака
    }
    return 1;
}

bool check_string(unsigned char *str, int string_length, unsigned char *allowed, int allowed_length) {
    if (!str || string_length <= 0) return false; // Проверка на NULL или пустую строку

    for (int i = 0; i < string_length; i++) {
        bool found = false;
        for (int j = 0; j < allowed_length; j++) {
            if (str[i] == allowed[j]) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false; // Недопустимый символ
        }
    }
    return true; // Все символы корректны
}

void solve(unsigned char *str) {
    // Инициализация массивов для коэффициентов ax+b=c
    unsigned char num1[30] = {0}, num2[31] = {0}, num3[31] = {0};
    int i = 0, j = 0;

    // 0) Проверка исходной строки на допустимые символы
    if (!validate_str(str)) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect ( 0) )", ++global_str);
        return;
    }

    // 1) Парсинг строки: ax+b=c
    while (str[i*2] && str[i*2] != 'x') {
        num1[i] = str[i*2];
        i++;
    }

    i++; // Пропуск 'x'

    while (str[i*2] && str[i*2] != '=') {
        num2[j] = str[i*2];
        i++;
        j++;
    }

    i++; // Пропуск '='

    j = 0;
    while (str[i*2]) {
        num3[j] = str[i*2];
        i++;
        j++;
    }

// DEBUG
/*
    global_pos = 0;
    global_str++;
    out_word(currentColour, "Debug: Parsed a=");
    out_word(currentColour, (const char*)num1);
    out_word(currentColour, ", b=");
    out_word(currentColour, (const char*)num2);
    out_word(currentColour, ", c=");
    out_word(currentColour, (const char*)num3);
    out_word(currentColour, "\n");
*/
    // 2) Длина строк
    int len1 = strlen(num1);
    int len2 = strlen(num2);
    int len3 = strlen(num3);

// Дебаг-вывод len
/*
    global_pos = 0;
    global_str++;
    out_word(currentColour, "Debug: Parsed len1=");
    out_word(currentColour, (const char*)int_to_char(len1));
    out_word(currentColour, ", len2=");
    out_word(currentColour, (const char*)int_to_char(len2));
    out_word(currentColour, ", len3=");
    out_word(currentColour, (const char*)int_to_char(len3));
    out_word(currentColour, "\n");
*/
    // 3) Проверка ведущих нулей
    int start1 = (num1[0] == '-' || num1[0] == '+') ? 1 : 0; // Пропускаем знак для a
    if (len1 > start1 + 1 && num1[start1] == '0') { // Ведущие нули после знака
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect (vedush 0-1)", ++global_str);
        return;
    }
    int start2 = (num2[0] == '-' || num2[0] == '+') ? 1 : 0; // Пропускаем знак для b
    if (len2 > start2 + 1 && num2[start2] == '0') { // Ведущие нули после знака
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect (vedush 0-2)", ++global_str);
        return;
    }
    int start3 = (num3[0] == '-' || num3[0] == '+') ? 1 : 0; // Пропускаем знак для c
    if (len3 > start3 + 1 && num3[start3] == '0') { // Ведущие нули после знака
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect (vedush 0-3)", ++global_str);
        return;
    }

    // 4) Преобразование строк в числа
    int dig1 = (*num1) ? (validate_num(num1) ? char_to_int(num1) : 0) : 1;
    if (dig1 == 1) num1[0] = '1';
    if (dig1 == -1) { num1[0] = '-'; num1[1] = '1'; }
    if (dig1 == 0) {
	out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: Division by zero", ++global_str);
	return;
    }
    int dig2 = (*num2) ? (validate_num(num2) ? char_to_int(num2) : 0) : 0;
    int dig3 = (*num3) ? (validate_num(num3) ? char_to_int(num3) : 0) : 0;

//debug digits
/*
    global_pos = 0;
    global_str++;
    out_word(currentColour, "Debug: Parsed dig1=");
    out_word(currentColour, (const char*)int_to_char(dig1));
    out_word(currentColour, ", dig2=");
    out_word(currentColour, (const char*)int_to_char(dig2));
    out_word(currentColour, ", dig3=");
    out_word(currentColour, (const char*)int_to_char(dig3));
    out_word(currentColour, "\n");
*/
    // 5) Проверка переполнения
    if (!check_of_signed(dig1, num1)) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: Integer overflow (dig1)", ++global_str);
        return;
    }
    if (!check_of_signed(dig2, num2)) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: Integer overflow (dig2)", ++global_str);
        return;
    }
    if (!check_of_signed(dig3, num3)) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: Integer overflow (dig3)", ++global_str);
        return;
    }

    // 6) Вычисление результата: x = (c - b) / a

    global_pos = 0;
    global_str++;

    // Проверка переполнения результата
    int ax_half = (dig3 / 2) - (dig2 / 2);
    int sign = 0;   //0=+, 1=-
    if (ax_half < 0) {
        sign = 1; 
        ax_half--;
    }
    out_word(currentColour, "ax_half=");
    if (sign == 0) out_word(currentColour, (const char*)int_to_char(ax_half));
    else out_word(currentColour, (const char*)int_to_char(ax_half));
    if (ax_half > (MAX_INT/2) || ax_half < -1073741824) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: Integer overflow", ++global_str);
        return;
    }
    
    out_word(currentColour, "Result: x=");

    float resfl = ((float)dig3 - (float)dig2) / (float)dig1;
    bool isNegative = (dig3 - dig2) * dig1 < 0; // Определяем знак результата

    // Проверка на целое число с допуском
    float fractionalPart = resfl - (int)resfl;
    bool isInteger = (fractionalPart < 1e-6 && fractionalPart > -1e-6); // Допуск 0.000001

    if (isInteger) {
        out_word(currentColour, (const char*)int_to_char((int)resfl));
    } else {
        // Целая часть
        int whole = (int)resfl;
        out_word(currentColour, (const char*)int_to_char(whole));

        // Дробная часть с пятью знаками
        out_word(currentColour, ".");
        float absFraction = (resfl < 0 ? -resfl : resfl) - (float)(whole < 0 ? -whole : whole); // Абсолютная дробная часть
        absFraction *= 100000; // Сдвигаем на 5 знаков
        int fractional = (int)(absFraction + 0.5); // Округление до ближайшего целого

        // Выводим пять знаков, добавляя ведущие нули
        unsigned char *frac = int_to_char(fractional);
        for (int k = 5; k > strlen(frac); k--) out_word(currentColour, "0");
        out_word(currentColour, (const char*)frac);

        // Корректируем вывод, если результат отрицательный и целая часть нулевая
        if (isNegative && whole == 0 && fractional > 0) {
            global_pos = 0;
            global_str--;
            out_word(currentColour, "-");
            out_word(currentColour, "0");
            out_word(currentColour, ".");
            for (int k = 5; k > strlen(frac); k--) out_word(currentColour, "0");
            out_word(currentColour, (const char*)frac);
        }
    }
}

int nod(int m, int n) {
    // Проверка входных данных с помощью check_of
    if (!check_of(m, int_to_char(m)) || !check_of(n, int_to_char(n))) {
	global_pos = 0;
	global_str++;
	out_word(currentColour, "error_nod");
        return 0; // Возвращаем 0 как индикатор ошибки
    }
    
    // Алгоритм Евклида
    return n ? nod(n, m % n) : m;
}

void gcd(unsigned char *str) {
    int i = 0, j = 0;
    unsigned char gcd1[35] = {0}, gcd2[35] = {0};

    // 1) Парсим строку
    while (str[i*2] != ' ' && str[i*2]) {	
        gcd1[i] = str[i*2];
        i++;
    }
    i++;
    while (str[i*2]) {
        gcd2[j] = str[i*2];
        i++;
        j++;
    }
/*
// Дебаг-вывод полученных символов
    global_pos = 0;
    global_str++;
    out_word(currentColour, "Debug: Parsed gcd1=");
    out_word(currentColour, (const char*)gcd1);
    out_word(currentColour, ", gcd2=");
    out_word(currentColour, (const char*)gcd2);
    out_word(currentColour, "\n");
*/

    // 2) Проверка на лишние символы
    int len1 = strlen(gcd1);
    int len2 = strlen(gcd2);

// Дебаг-вывод len
/*
    global_pos = 0;
    global_str++;
    out_word(currentColour, "Debug: Parsed len1=");
    out_word(currentColour, (const char*)int_to_char(len1));
    out_word(currentColour, ", len2=");
    out_word(currentColour, (const char*)int_to_char(len2));
    out_word(currentColour, "\n");
*/
    unsigned char allowed_gcd[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    int allowed_gcd_length = 10;

    if (!check_string(gcd1, len1, allowed_gcd, allowed_gcd_length) || !check_string(gcd2, len2, allowed_gcd, allowed_gcd_length)) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect", ++global_str);
        return;
    }

    // 3) Проверка ведущих нулей
    if (len1 > 1 && gcd1[0] == '0') {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect (vedush 0-1)", ++global_str);
        return;
    }
    if (len2 > 1 && gcd2[0] == '0') {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect (vedush 0-2)", ++global_str);
        return;
    }

    // 4) Парсим числа из строки
    int dig1 = char_to_int(gcd1);
    int dig2 = char_to_int(gcd2);

    // 5) Проверка <=0
    if (dig1 == 0 || dig2 == 0) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect", ++global_str);
        return;
    }

    // 6) Проверка переполнения
    if (!check_of(dig1, gcd1)) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: Integer overflow", ++global_str);
        return;
    }
    if (!check_of(dig2, gcd2)) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: Integer overflow", ++global_str);
        return;
    }

    // 7) Операция
    int gcd_res = nod(dig1, dig2);
    unsigned char *chr = int_to_char(gcd_res);
    global_pos = 0;
    global_str++;
    out_word(currentColour, "Result: ");
    out_word(currentColour, (const char*)chr);
}

int nok(int m, int n) {
    // Проверка входных данных с помощью check_of
    if (!check_of(m, int_to_char(m)) || !check_of(n, int_to_char(n))) {
	global_pos = 0;
	global_str++;
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: Integer overflow (m or n OF)", ++global_str);
        return -1;
    }

    int gcd = nod(m, n);

    if (gcd == 0) {
	global_pos = 0;
	global_str++;
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: Integer overflow (nod=0)", ++global_str);
        return -1;
    }
    
    // Проверка переполнения при умножении m * n
    if (m > MAX_INT / n) {
        out_word(currentColour, " ");
        out_str(ERR_CLR, "Error: Integer overflow (m*n OF)", ++global_str);
	return -1;
    }

    // Вычисление НОК
    return (m / gcd) * n;
}

void lcm(unsigned char *str) {
    int i = 0, j = 0;
    unsigned char lcm1[35] = {0}, lcm2[35] = {0};

    // 1) Парсим строку
    while (str[i*2] != ' ' && str[i*2]) {	
        lcm1[i] = str[i*2];
        i++;
    }
    i++;
    while (str[i*2]) {
        lcm2[j] = str[i*2];
        i++;
        j++;
    }

// Дебаг-вывод полученных символов
/*
    global_pos = 0;
    global_str++;
    out_word(currentColour, "Debug: Parsed lcm1=");
    out_word(currentColour, (const char*)lcm1);
    out_word(currentColour, ", lcm2=");
    out_word(currentColour, (const char*)lcm2);
    out_word(currentColour, "\n");
*/

    // 2) Проверка на лишние символы
    int len1 = strlen(lcm1);
    int len2 = strlen(lcm2);

// Дебаг-вывод len
/*
    global_pos = 0;
    global_str++;
    out_word(currentColour, "Debug: Parsed len1=");
    out_word(currentColour, (const char*)int_to_char(len1));
    out_word(currentColour, ", len2=");
    out_word(currentColour, (const char*)int_to_char(len2));
    out_word(currentColour, "\n");
*/
    unsigned char allowed_lcm[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    int allowed_lcm_length = 10;

// check_string also checks for <0
    if (!check_string(lcm1, len1, allowed_lcm, allowed_lcm_length) || !check_string(lcm2, len2, allowed_lcm, allowed_lcm_length)) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect (check_string)", ++global_str);
        return;
    }

    // 3) Проверка ведущих нулей
    if (len1 > 1 && lcm1[0] == '0') {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect (vedush 0-1)", ++global_str);
        return;
    }
    if (len2 > 1 && lcm2[0] == '0') {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect (vedush 0-2)", ++global_str);
        return;
    }

    // 4) Парсим числа из строки
    int dig1 = char_to_int(lcm1);
    int dig2 = char_to_int(lcm2);

    // 5) Проверка <=0
    if (dig1 == 0 || dig2 == 0) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect", ++global_str);
        return;
    }

    // 6) Проверка переполнения
    if (!check_of(dig1, lcm1)) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: Integer overflow (dig1)", ++global_str);
        return;
    }
    if (!check_of(dig2, lcm2)) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: Integer overflow (dig2)", ++global_str);
        return;
    }

    // 7) Операция
// int -> uint
    int lcm_res = nok(dig1, dig2);
    if (lcm_res == -1) return;
    unsigned char *chr = int_to_char(lcm_res);
    global_pos = 0;
    global_str++;
    out_word(currentColour, "Result: ");
    out_word(currentColour, (const char*)chr);
}

// Вспомогательная функция для валидации всей строки
int validate_str_div(unsigned char *s) {
    for (int k = 0; s[k*2]; k++) {
        unsigned char c = s[k*2];
        if (!(c >= '0' && c <= '9') && c != '-' && c != ' ') return 0;
    }
    return 1;
}

void div(unsigned char *str){
    int i = 0, j = 0;
    unsigned char div1[35] = {0}, div2[35] = {0};

    // 0) Проверка исходной строки на допустимые символы
    if (!validate_str_div(str)) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect ( 0) )", ++global_str);
        return;
    }

    // 1) Парсим строку
    while (str[i*2] != ' ' && str[i*2]) {	
        div1[i] = str[i*2];
        i++;
    }
    i++;
    while (str[i*2]) {
        div2[j] = str[i*2];
        i++;
        j++;
    }

    // Дебаг-вывод полученных символов
    global_pos = 0;
    global_str++;
    out_word(currentColour, "Debug: Parsed div1=");
    out_word(currentColour, (const char*)div1);
    out_word(currentColour, ", div2=");
    out_word(currentColour, (const char*)div2);
    out_word(currentColour, "\n");

    // 2) Проверка на лишние символы
    int len1 = strlen(div1);
    int len2 = strlen(div2);

    // Дебаг-вывод len
    global_pos = 0;
    global_str++;
    out_word(currentColour, "Debug: Parsed len1=");
    out_word(currentColour, (const char*)int_to_char(len1));
    out_word(currentColour, ", len2=");
    out_word(currentColour, (const char*)int_to_char(len2));
    out_word(currentColour, "\n");

    unsigned char allowed_div[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-' };
    int allowed_div_length = 11;

// check_string also checks for <0
    if (!check_string(div1, len1, allowed_div, allowed_div_length) || !check_string(div2, len2, allowed_div, allowed_div_length)) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect (check_string)", ++global_str);
        return;
    }
    // 3) Проверка ведущих нулей
    int start1 = (div1[0] == '-' || div1[0] == '+') ? 1 : 0; // Пропускаем знак для div1
    if (len1 > start1 + 1 && div1[start1] == '0') { // Ведущие нули после знака
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect (vedush 0-1)", ++global_str);
        return;
    }
    int start2 = (div2[0] == '-' || div2[0] == '+') ? 1 : 0; // Пропускаем знак для div2
    if (len2 > start2 + 1 && div2[start2] == '0') { // Ведущие нули после знака
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: command incorrect (vedush 0-2)", ++global_str);
        return;
    }

    // 4) Преобразование строк в числа
    int dig1 = (*div1) ? (validate_num(div1) ? char_to_int(div1) : 0) : 1;
    if (dig1 == 1) div1[0] = '1';
    if (dig1 == -1) { div1[0] = '-'; div1[1] = '1'; }

    int dig2 = (*div2) ? (validate_num(div2) ? char_to_int(div2) : 0) : 0;
    if (dig2 == 0) {
	out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: Division by zero", ++global_str);
	return;
    }

//debug digits
    global_pos = 0;
    global_str++;
    out_word(currentColour, "Debug: Parsed dig1=");
    out_word(currentColour, (const char*)int_to_char(dig1));
    out_word(currentColour, ", dig2=");
    out_word(currentColour, (const char*)int_to_char(dig2));
    out_word(currentColour, "\n");

    // 5) Проверка переполнения
    if (!check_of_signed(dig1, div1)) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: Integer overflow (dig1)", ++global_str);
        return;
    }

    if (!check_of_signed(dig2, div2)) {
        out_word(ERR_CLR, " ");
        out_str(ERR_CLR, "Error: Integer overflow (dig2)", ++global_str);
        return;
    }

    // 6) Вычисление результата: x = a/b
    global_pos = 0;
    global_str++;
    out_word(currentColour, "Result: x=");

    float resfl = (float)dig1 / (float)dig2;
    bool isNegative = (dig1 * dig2 < 0); // Определяем знак результата

    // Целая часть
    int whole = (int)resfl;
    out_word(currentColour, (const char*)int_to_char(whole));

    // Дробная часть с пятью знаками
    out_word(currentColour, ".");
    float absFraction = (resfl < 0 ? -resfl : resfl) - (float)(whole < 0 ? -whole : whole); // Абсолютная дробная часть
    absFraction *= 100000; // Сдвигаем на 5 знаков
    int fractional = (int)(absFraction + 0.5); // Округление до ближайшего целого

    // Выводим пять знаков, добавляя ведущие нули
    unsigned char *frac = int_to_char(fractional);
    for (int k = 5; k > strlen(frac); k--) out_word(currentColour, "0");
    out_word(currentColour, (const char*)frac);

    // Корректируем вывод, если результат отрицательный и целая часть нулевая
    if (isNegative && whole == 0 && fractional > 0) {
        global_pos = 0;
        global_str--;
        out_word(currentColour, "-");
        out_word(currentColour, "0");
        out_word(currentColour, ".");
        for (int k = 5; k > strlen(frac); k--) out_word(currentColour, "0");
        out_word(currentColour, (const char*)frac);
    }
}

