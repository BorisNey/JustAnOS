
#include "../include/vga_terminal.h"

void kernel_main(){
	terminal_init(VGA_COLOR_BLACK, VGA_COLOR_CYAN);
	terminal_printf("Hello World!\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\nend");
	return;
}
