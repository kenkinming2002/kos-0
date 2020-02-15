#pragma once

#include <stdint.h>

namespace io
{
  class PS2Controller
  {
  public:
    PS2Controller();

  private:
    static void writeToDataPort(uint8_t data);
    static void writeToCommandPort(uint8_t data);
    static uint8_t readFromDataPort();

    void flushDataPort();

  private:
    bool m_firstChannelFunctioning, m_secondChannelFunctioning;
    bool m_isDuelChannel;
  };

  extern PS2Controller ps2Controller;
}
