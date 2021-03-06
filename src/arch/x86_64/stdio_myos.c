#include "stdio_myos.h"
#include "math.h"

uint16 vga_entry(uint8 ch, uint8 fore_color, uint8 back_color) {
    uint16 ax = 0;
    uint8 ah = 0, al = 0;

    ah = back_color;
    ah <<= 4;
    ah |= fore_color;
    ax = ah;
    ax <<= 8;
    al = ch;
    ax |= al;

    return ax;
}
void set_cursor(struct VGAInfo* i, uint32 index) {
    i->vga_index = index;
    i->next_line_index = i->vga_index / 80 + 1;
}
void set_cursor_coord(struct VGAInfo* i, uint32 x, uint32 y) {
    i->vga_index = x + y * 80;
    i->next_line_index = y + 1;
}
void clear_vga_buffer(struct VGAInfo* info, uint16** buffer, uint8 fore_color, uint8 back_color) {
    uint32 i;
    for(i = 0; i < BUFSIZE; i++) {
        (*buffer)[i] = vga_entry(NULL, fore_color, back_color);
    }
    info->next_line_index = 1;
    info->vga_index = 0;
}

void init_vga(struct VGAInfo* i, uint8 fore_color, uint8 back_color) {
    i->vga_buffer = (uint16*)VGA_ADDRESS;
    clear_vga_buffer(i, &i->vga_buffer, fore_color, back_color);
    i->g_fore_color = fore_color;
    i->g_back_color = back_color;
}
void print_new_line(struct VGAInfo* i) {
    if(i->next_line_index >= 55) {
        i->next_line_index = 0;
        clear_vga_buffer(i, &i->vga_buffer, i->g_fore_color, i->g_back_color);
    }
    i->vga_index = 80*i->next_line_index;
    i->next_line_index++;
}
void print_char(struct VGAInfo* i, char ch) {
    if(ch == '\n') {
        print_new_line(i);
        return;
    }
    i->vga_buffer[i->vga_index] = vga_entry(ch, i->g_fore_color, i->g_back_color);
    i->vga_index++;
}
uint32 strlen(const char* str) {
    uint32 length = 0;
    while(str[length])
        length++;
    return length;
}
uint32 digit_count( int num) {
    uint32 count = 0;
    if(num == 0)
        return 1;
    while(num > 0) {
        count++;
        num = num / 10;
    }
    return count;
}
uint32 digit_count64(uint64 num) {
  uint32 count = 0;
  if(num == 0)
    return 1;
  while(num > 0) {
    count++;
    num = num / 10;
  }
  return count;
}
void itoa(int num, char *number)
{
  int dgcount = digit_count(num);
  int index = dgcount - 1;
  char x;
  if(num == 0 && dgcount == 1){
    number[0] = '0';
    number[1] = '\0';
  }else{
    while(num != 0){
      x = num % 10;
      number[index] = x + '0';
      index--;
      num = num / 10;
    }
    number[dgcount] = '\0';
  }
}
void itoa64(uint64 num, char *number) {
  int dgcount = digit_count64(num);
  int index = dgcount - 1;
  char x;
  if(num == 0 && dgcount == 1){
    number[0] = '0';
    number[1] = '\0';
  }else{
    while(num != 0){
      x = num % 10;
      number[index] = x + '0';
      index--;
      num = num / 10;
    }
    number[dgcount] = '\0';
  }
}
void print_int(struct VGAInfo* i, uint32 num) {
  char str_num[digit_count(num)+1];
  itoa(num, str_num);
  print_string(i, str_num);
}
void print_long(struct VGAInfo* i, uint64 num) {
  char str_num[digit_count64(num)+1];
  itoa64(num, str_num);
  print_string(i, str_num);
}
void print_string(struct VGAInfo* i, char *str)
{
    uint32 index = 0;
    while(str[index]){
        print_char(i, str[index]);
        index++;
    }
}
uint8 inb(uint16 port)
{
  uint8 ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "d"(port));
  return ret;
}

void outb(uint16 port, uint8 data)
{
  asm volatile("outb %0, %1" : "=a"(data) : "d"(port));
}

char get_input_keycode()
{
  char ch = 0;
  while((ch = inb(KEYBOARD_PORT)) != 0){
    if(ch > 0)
      return ch;
  }
  return ch;
}

void wait_for_io(uint32 timer_count)
{
  while(1){
    asm volatile("nop");
    timer_count--;
    if(timer_count <= 0)
      break;
    }
}

void sleep(uint32 timer_count)
{
  wait_for_io(timer_count);
}

void test_input(struct VGAInfo* info)
{
  char ch = 0;
  char keycode = 0;
  bool first = false;
  char k = get_input_keycode();
  do{
    keycode = get_input_keycode();
    if(keycode == KEY_ENTER){
      print_new_line(info);
    } else{
      ch = get_ascii_char(keycode);
      print_char(info, ch);
    }
    if(first)
      sleep(0x009FFFFF);
    else {
      sleep(0x0FFFFFFF);
      first = true;
    }
  }while(ch > 0);
}