#include "terminal.h"
#include "mman.h"
#include "io.h"

struct terminal stdout;

void terminal_init(struct terminal* term) 
{
	//map vga buffer
	map_hardware_buffer((void*) VGA_BUFFER, (void*) VGA_BUFFER_PHYS, VGA_BUFFER_SIZE, PAGE_FLAG_USER | PAGE_FLAG_WRITE);

	term->row = 0;
	term->column = 0;
	term->color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	term->buffer = (uint16_t*) VGA_BUFFER;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			term->buffer[index] = vga_entry(' ', term->color);
		}
	}

	//disable cursor
	outb(0x0A, 0x3D4);
	outb(0x20, 0x3D5);

	// //enable cursor
 	// uint8_t cursor_start = 0xF;
	// uint8_t cursor_end = 0xF;

	// outb(0x0A, 0x3D4);
	// outb((inb(0x3D5) & 0xC0) | cursor_start, 0x3D5);
 
	// outb(0x0B, 0x3D4);
	// outb((inb(0x3D5) & 0xE0) | cursor_end, 0x3D5);
}
 
void terminal_setcolor(struct terminal* term, uint8_t color) 
{
	term->color = color;
}

uint8_t terminal_getcolor(struct terminal* term) 
{
	return term->color;
}
 
void terminal_putentryat(struct terminal* term, char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	term->buffer[index] = vga_entry(c, color);
	// terminal_movecursor(term, x, y);
}

void terminal_movecursor(struct terminal* term, size_t x, size_t y)
{
	(void) term; //we are directly manipulating curser, so were arean't using the term struct

	uint16_t pos = y * VGA_WIDTH + x;

	outb(0x0F, 0x3D4);
	outb((uint8_t) (pos & 0xFF), 0x3D5);
	outb(0x0E, 0x3D4);
	outb((uint8_t) ((pos >> 8) & 0xFF), 0x3D5);
}

 
void terminal_putchar(struct terminal* term, char c) 
{
	if(c == '\n')
	{
		terminal_newline(term);
		return;
	}

	terminal_putentryat(term, c, term->color, term->column, term->row);
	if (++term->column == VGA_WIDTH)
		terminal_newline(term);
}

void terminal_putcharn(struct terminal* term, char c, size_t repeat) 
{
	for (size_t i = 0; i < repeat; i++)
		terminal_putchar(term, c);
}

void terminal_newline(struct terminal* term)
{
    term->column = 0;
    if (++term->row == VGA_HEIGHT)
        terminal_scrollline(term);
}

void terminal_scrollline(struct terminal* term)
{
	// copy rows 2 though N to rows 1 through N-1
	memcpy(term->buffer, &term->buffer[VGA_WIDTH], VGA_WIDTH*(VGA_HEIGHT-1)*sizeof(term->buffer[0]));
	// clear last row
	term->row = VGA_HEIGHT-1;
	terminal_clearline(term);
}

void terminal_clearline(struct terminal* term)
{
	size_t row_start = term->row * VGA_WIDTH;
	for(size_t nn=0;nn<VGA_WIDTH;nn++)
		term->buffer[row_start+nn] = vga_entry(' ', term->color);

}


void terminal_write(struct terminal* term, const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(term, data[i]);
}
 
void terminal_writestring(struct terminal* term, const char* data) 
{
	terminal_write(term, data, strlen(data));
}

