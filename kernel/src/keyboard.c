#include "../include/keyboard.h"

const char* char_table_de[] = {
//  0     1     2     3     4     5     6     7     8     9     A     B       C     D       E       F
	   0,    0, "1!","2\"", "3§", "4$", "5%", "6&", "7/", "8(", "9)", "0=",   "ß?", "´`", "\b\b", "\t\t",
	"qQ", "wW", "eE", "rR", "tT", "zZ", "uU", "iI", "oO", "pP", "üÜ", "+*", "\n\n",    0,   "aA",   "sS",
	"dD", "fF", "gG", "hH", "jJ", "kK", "lL", "öÖ", "äÄ", "^°",    0, "#'",   "yY", "xX",   "cC",   "vV",
	"bB", "nN", "mM", ",;", ".:", "-_",    0,    0,    0, "  "
};

uint8_t caps = 0;
uint8_t shift = 0;
uint8_t uppercase = 0;

void initKeyboard(){
	installIrqHandler(1, keyboardHandlerDE);
	biosTermPrintf("DBG: Keyboard init success\n");
	return;
}

void keyboardHandlerDE(intr_regs_t* regs){
	UNUSED(regs);

	uint8_t rawInput = inPortB(KEY_PORT);
	uint8_t released = rawInput & 0x80;
	uint8_t scancode = rawInput & 0x7F;

	switch(scancode){
		// Shift
		case 0x2A:
		case 0x36:
			if (released)
				shift = 0;
			else if (!released)
				shift = 1;

			if ((!shift && caps) || (shift && !caps))
				uppercase = 1;
			else
				uppercase = 0;
			break;

		// Capital Lock
		case 0x3A:
			if (!released && caps)
				caps = 0;
			else if (!released && !caps)
				caps = 1;

			if ((!shift && caps) || (shift && !caps))
				uppercase = 1;
			else
				uppercase = 0;
			break;

		// filter these out:
		case 0x1D:	// Control
		case 0x37:	// Error
		case 0x38:	// Alt
			break;
	
		default:
			if ((0x01 < scancode) && (scancode < 0x3A) && !released)
				biosTermPutc(char_table_de[scancode][uppercase]);
			break;
	}
	
	return;
}

