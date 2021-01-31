#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include "common.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

struct terminal
{
    size_t row;
    size_t column;
    uint8_t color;
    uint16_t* buffer;
};

extern struct terminal stdout;


/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};
 
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}
  

void terminal_init(struct terminal* term); 
void terminal_setcolor(struct terminal* term, uint8_t color);
void terminal_putentryat(struct terminal* term, char c, uint8_t color, size_t x, size_t y);
void terminal_putchar(struct terminal* term, char c);
void terminal_putcharn(struct terminal* term, char c, size_t repeat);
void terminal_newline(struct terminal* term);
void terminal_write(struct terminal* term, const char* data, size_t size);
void terminal_writestring(struct terminal* term, const char* data);
void terminal_scrollline(struct terminal* term);
void terminal_clearline(struct terminal* term);
void terminal_movecursor(struct terminal* term, size_t x, size_t y);

#endif