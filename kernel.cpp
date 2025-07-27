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
	out_str(currentColour, ".......##.##.....##.##........##...##..##.......##...##...##.....##.......##", ++global_str);

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

unsigned char *int_to_char(int integer){
	bool add = 0;
	unsigned char *ch;
	for (int l = 0; l < 11; l++) ch[l] = '\0';

	if (integer < 0) {
		integer = 0 - integer;
		add = 1;
	}

	if (!integer) return (unsigned char*)"0";

	int i = -1;
	int temp = integer;
	while (temp) {
		i++;
		temp /= 10;
	}
	if (add) i++;

	while (i >= 0) {
		ch[i--] = (integer % 10) + '0';
		integer /= 10;
	}
	if (add) ch[0] = '-';
	return ch;
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

bool check_of(int num, unsigned char* str_num){
	if (*str_num == '+') str_num++;
	unsigned char *num_str;
	for (int i = 0; i < 11; i++) num_str[i] = '\0';
	num_str = int_to_char(num);
	return !(strcmp_s(str_num, num_str));
}

bool check_of_sum(int num1, int num2){
	if (num1 >= 0 && num2 >= 0) return (num1 > MAX_INT - num2);
	if (num1 < 0 && num2 < 0) return (num1 < MIN_INT - num2);
	return 0;
}

void solve(unsigned char *str){
	int i = 0, j = 0;
	unsigned char num1[20], num2[20], num3[20];

// разве не просто задаю значение массиву?
	for (int k = 0; k < 20; k++){
		num1[k] = '\0';
		num2[k] = '\0';
		num3[k] = '\0';
	}
	
	// Заполнение чисел num1, num2, num3 через двойную индексацию
	while (str[i*2] != 'x' && str[i*2]){	
		num1[i] = str[i*2];
		i++;
	}
	i++;
	while(str[i*2] && str[i*2] != '='){
		num2[j] = str[i*2];
		i++;
		j++;
	}
	i++;
	j = 0;
	while(str[i*2]){
		num3[j] = str[i*2];
		i++;
		j++;
	}
	
	int dig1 = 1;
	if (*num1 != '\0') dig1 = char_to_int(num1);
	if (dig1 == 1)  num1[0] = '1';
	if (dig1 == -1){
		num1[0] = '-';
		num1[1] = '1';
	}
	int dig2 = 0;
	if (*num2 != '\0') dig2 = char_to_int(num2);
	else *num2 = '0';
	int dig3 = 0;
	if (*num3 != '\0') dig3 = char_to_int(num3);
	else *num3 = '0';
	out_word(ERR_CLR, " ");
	bool ZD = (dig1 == 0);
	bool WC = (dig2 == 0 && dig3 == 0);
	bool OF = (check_of(dig1, num1) || check_of(dig3, num3) || check_of(dig2, num2) || check_of_sum(dig3, -dig2));
	if (!ZD && !WC && !OF){
		int res = (dig3 - dig2) /dig1;
		global_pos = 0;
		global_str++;
		out_word(currentColour, "Result: x=");
		if (dig1 * res + dig2 != dig3){
			float resfl = ((float)dig3 - (float)dig2) / (float)dig1;
			unsigned char *chr1 = int_to_char((int)resfl);
			if (resfl < 0 && resfl > -1) out_word(currentColour, "-");
			out_word(currentColour, (const char*)chr1);
			out_word(currentColour, ".");
			resfl = (resfl - (int)resfl)*1000000;
			if (resfl < 0) resfl = resfl*(-1);
			if ((int)resfl % 10 >= 5) resfl = resfl + 10;
			resfl = resfl / 10;
			unsigned char *chr2 = int_to_char((int)resfl);
			for (int k = 5; k > strlen((unsigned char *)chr2); k--)
				out_word(currentColour, "0");
			out_word(currentColour, (const char*)chr2);
		}
		else {
		unsigned char *chr = int_to_char(res);
		out_word(currentColour, "Result: x=");
		out_word(currentColour, (const char*)chr);
		}
	}
	else {
		if (WC) out_str(ERR_CLR, "Error: command incorrect", ++global_str);
		if (ZD) out_str(ERR_CLR, "Error: division by zero", ++global_str);
		if (OF) out_str(ERR_CLR, "Error: Integer overflow", ++global_str);
	}
}

int nod(int m, int n){
    return n ? nod(n, m % n) : m;
}

void gcd(unsigned char *str){
	int i = 0, j = 0;
	unsigned char gcd1[20], gcd2[20];
	for (int k = 0; k < 20; k++){
		gcd1[k] = '\0';
		gcd2[k] = '\0';
	}
	while (str[i*2] != ' ' && str[i*2]){	
		gcd1[i] = str[i*2];
		i++;
	}
	i++;
	while(str[i*2]){
		gcd2[j] = str[i*2];
		i++;
		j++;
	}
	int dig1 = 0;
	if (*gcd1 != '\0') dig1 = char_to_int(gcd1);
	else *gcd1 = '0';
	int dig2 = 0;
	if (*gcd2 != '\0') dig2 = char_to_int(gcd2);
	else *gcd2 = '0';
	out_word(ERR_CLR, " ");
	bool WC = (*gcd1 == '\0' || *gcd2 == '\0' || dig1 == 0 || dig2 == 0);
	bool OF = (check_of(dig1, gcd1) || check_of(dig2, gcd2));
	if (!WC && !OF){
		int gcd_res = nod(dig1, dig2);
		unsigned char *chr = int_to_char(gcd_res);
		global_pos = 0;
		global_str++;
		out_word(currentColour, "Result: ");
		out_word(currentColour, (const char*)chr);
	}
	else {
		if (WC) out_str(ERR_CLR, "Error: command incorrect", ++global_str);
		if (OF) out_str(ERR_CLR, "Error: Integer overflow", ++global_str);
	}
}

void lcm(unsigned char *str){
	int i = 0, j = 0;
	unsigned char lcm1[20], lcm2[20];
	for (int k = 0; k < 20; k++){
		lcm1[k] = '\0';
		lcm2[k] = '\0';
	}
	while (str[i*2] != ' ' && str[i*2]){	
		lcm1[i] = str[i*2];
		i++;
	}
	i++;
	while(str[i*2]){
		lcm2[j] = str[i*2];
		i++;
		j++;
	}
	int dig1 = 0;
	if (*lcm1 != '\0') dig1 = char_to_int(lcm1);
	else *lcm1 = '0';
	int dig2 = 0;
	if (*lcm2 != '\0') dig2 = char_to_int(lcm2);
	else *lcm2 = '0';
	out_word(ERR_CLR, " ");
	bool WC = (*lcm1 == '\0' || *lcm2 == '\0' || dig1 == 0 || dig2 == 0);
	bool OF = (check_of(dig1,lcm1) || check_of(dig2, lcm2));
	if (!WC && !OF){
		
		int lcm_res = (dig1 * dig2) / nod(dig1, dig2);
		unsigned char *chr = int_to_char(lcm_res);
		global_pos = 0;
		global_str++;
		out_word(currentColour, "Result: ");
		out_word(currentColour, (const char*)chr);
	}
	else {
		if  (WC) out_str(ERR_CLR, "Error: command incorrect", ++global_str);
		if (OF) out_str(ERR_CLR, "Error: Integer overflow", ++global_str);
	}
}

void div(unsigned char *str){
	int i = 0, j = 0;
	unsigned char div1[20], div2[20];
	for (int k = 0; k < 20; k++){
		div1[k] = '\0';
		div2[k] = '\0';
	}
	while (str[i*2] != ' ' && str[i*2]){	
		div1[i] = str[i*2];
		i++;
	}
	i++;
	while(str[i*2]){
		div2[j] = str[i*2];
		i++;
		j++;
	}
	bool WC = (*div1 == '\0' || *div2 == '\0');
	int dig1 = 0;
	if (*div1 != '\0') dig1 = char_to_int(div1);
	else *div1 = '0';
	int dig2 = 0;
	if (*div2 != '\0') dig2 = char_to_int(div2);
	else *div2 = '0';
	out_word(ERR_CLR, " ");
	bool ZD = (dig2 == 0 && !WC);
	bool OF = (check_of(dig1, div1) || check_of(dig2, div2));
	if (!WC && !ZD && !OF){
		float div_resfl = (float)dig1 / (float)dig2;
		unsigned char *chr1 = int_to_char((int)div_resfl);
		global_pos = 0;
		global_str++;
		out_word(currentColour, "Result: ");
		if (div_resfl < 0 && div_resfl > -1) out_word(currentColour, "-");
		out_word(currentColour, (const char*)chr1);
		out_word(currentColour, ".");
		div_resfl = (div_resfl - (int)div_resfl) * 1000000;
		if (div_resfl < 0) div_resfl = div_resfl * (-1);
		if ((int)div_resfl % 10 >= 5) div_resfl = div_resfl + 10;
		div_resfl = div_resfl / 10;
		unsigned char *chr2 = int_to_char((int)div_resfl);
		for (int k = 5; k > strlen((unsigned char *)chr2); k--)
			out_word(currentColour, "0");
			out_word(currentColour, (const char*)chr2);
	}
	else {
		if (WC) out_str(ERR_CLR, "Error: command incorrect", ++global_str);
		if (ZD) out_str(ERR_CLR, "Error: Division by zero", ++global_str);
		if (OF) out_str(ERR_CLR, "Error: Integer overflow", ++global_str);
	}
}