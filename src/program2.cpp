#include <syscall.h>

extern "C" void _start()
{
  write("PROG2\n", 6);
  int service = locate_service("SERVICE");
  if(service<0)
  {
    write("Service Location Failed\n", 24);
    return;
  }
  send(service, "SACRET", 6);
  send(service, "SBCRET", 6);

  yield();
}
