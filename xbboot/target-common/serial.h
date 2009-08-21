//
// Authors: Wolfgang Spraul <wolfgang@qi-hardware.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 3 of the License, or (at your option) any later version.
//

#include "common-types.h"

extern u32 UART_BASE;

void serial_putc(char c);
void serial_puts(const char *s);
void serial_put_hex(unsigned int v);
int serial_getc();
int serial_tstc();
