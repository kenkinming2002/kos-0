#include <syscall.h>

#include <stdint.h>
#include <limits>

extern "C" void _start()
{
  write("PROG1\n", 6);
  int service = create_service("SERVICE");
  if(service<0)
  {
    write("Service Creation Failed\n", 24);
    return;
  }

  yield();

  char buf[6];

  if(recv(std::numeric_limits<tid_t>::max(), buf, 6)<0)
  {
    write("Message Receive Failed\n", 23);
    return;
  }
  write(buf, 6);

  if(recv(std::numeric_limits<tid_t>::max(), buf, 6)<0)
  {
    write("Message Receive Failed\n", 23);
    return;
  }
  write(buf, 6);

  if(destroy_service(service) != 0)
  {
    write("Service Destruction Failed\n", 27);
    return;
  }

}
