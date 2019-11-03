#ifndef SERIAL_H
#define SERIAL_H

#ifdef __cplusplus
extern "C"
{
#endif

#define SERIAL_COM1_BASE                0x3F8      /* COM1 base port */

void serial_configure(unsigned short com, unsigned short divisor);
int serial_write(unsigned short com, const char* buf, unsigned int len);
int serial_read(unsigned short com, char* buf, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif // SERIAL_H
