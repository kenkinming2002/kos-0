#pragma once

#define SERIAL_COM1_BASE                0x3F8      /* COM1 base port */

void serial_configure(unsigned short com, unsigned short divisor);
int serial_write(unsigned short com, const char* buf, unsigned int len);
int serial_read(unsigned short com, char* buf, unsigned int len);
