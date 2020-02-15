#include <generic/io/PS2Controller.hpp>

#include <intel/asm/io.hpp>

namespace io
{
  namespace
  {
    constexpr uint16_t PS2_CONTROLLER_DATA_PORT = 0x60;
    constexpr uint16_t PS2_CONTROLLER_COMMAND_PORT = 0x64;
    constexpr uint16_t PS2_CONTROLLER_STATUS_PORT = 0x64;

    constexpr uint8_t PS2_CONTROLLER_COMMAND_DISABLE_SECOND_DEVICE = 0xA7;
    constexpr uint8_t PS2_CONTROLLER_COMMAND_ENABLE_SECOND_DEVICE = 0xA8;

    constexpr uint8_t PS2_CONTROLLER_COMMAND_DISABLE_FIRST_DEVICE = 0xAD;
    constexpr uint8_t PS2_CONTROLLER_COMMAND_ENABLE_FIRST_DEVICE = 0xAE;

    constexpr uint8_t PS2_CONTROLLER_COMMAND_READ_CONTROLLER_CONFIGURATION = 0x20;
    constexpr uint8_t PS2_CONTROLLER_COMMAND_WRITE_CONTROLLER_CONFIGURATION = 0x60;

    constexpr uint8_t PS2_CONTROLLER_COMMAND_CONTROLLER_SELF_TEST = 0xAA;
    constexpr uint8_t PS2_CONTROLLER_SELF_TEST_SUCCESS = 0x55;

    constexpr uint8_t PS2_CONTROLLER_COMMAND_TEST_FIRST_PORT = 0xAB;
    constexpr uint8_t PS2_CONTROLLER_COMMAND_TEST_SECOND_PORT = 0xA9;

    constexpr uint8_t PS2_CONTROLLER_COMMAND_WRITE_SECOND_PORT_INPUT = 0xD4;

    constexpr uint8_t PS2_DEVICE_COMMAND_RESET = 0xFF;
    constexpr uint8_t PS2_DEVICE_COMMAND_ACK = 0xFA;
    constexpr uint8_t PS2_DEVICE_RESET_SUCCESS = 0xAA;
  }

  PS2Controller::PS2Controller()
  {
    uint8_t configurationByte;
    // TODO: Determine if PS/2 Controller exist via ACPI.
    //       For now, we assume it exist.
    
    // 1: Disable all PS/2 devices
    this->writeToCommandPort(PS2_CONTROLLER_COMMAND_DISABLE_FIRST_DEVICE);
    this->writeToCommandPort(PS2_CONTROLLER_COMMAND_DISABLE_SECOND_DEVICE);

    // 2: Flush the output buffer
    this->flushDataPort();

    // 3: Get configuration of the PS/2 Controller
    this->writeToCommandPort(PS2_CONTROLLER_COMMAND_READ_CONTROLLER_CONFIGURATION);
    configurationByte = this->readFromDataPort();
    m_isDuelChannel = configurationByte & static_cast<uint8_t>(1 << 5);

    // 4: Configure the PS/2 Controller
    configurationByte = configurationByte & ~static_cast<uint8_t>(0x43); // Disable interrupts and translation

    this->writeToCommandPort(PS2_CONTROLLER_COMMAND_WRITE_CONTROLLER_CONFIGURATION);
    this->writeToDataPort(configurationByte);

    // 5: Perform controller self test
    this->writeToCommandPort(PS2_CONTROLLER_COMMAND_CONTROLLER_SELF_TEST);
    if(this->readFromDataPort() != PS2_CONTROLLER_SELF_TEST_SUCCESS)
    {
      m_firstChannelFunctioning = false;
      m_secondChannelFunctioning = false;
      return;
    }

    // Restore the configuration byte as this can be reset on some device after self test
    this->writeToCommandPort(PS2_CONTROLLER_COMMAND_WRITE_CONTROLLER_CONFIGURATION);
    this->writeToDataPort(configurationByte); 

    // 6: Test for second PS/2 device
    if(m_isDuelChannel)
    {
      this->writeToCommandPort(PS2_CONTROLLER_COMMAND_ENABLE_SECOND_DEVICE);
      this->writeToCommandPort(PS2_CONTROLLER_COMMAND_READ_CONTROLLER_CONFIGURATION);
      configurationByte = this->readFromDataPort();
      if(m_isDuelChannel)
        m_isDuelChannel = !(configurationByte & static_cast<uint8_t>(1 << 5));
      this->writeToCommandPort(PS2_CONTROLLER_COMMAND_DISABLE_SECOND_DEVICE);
    }

    // 7: Perform Inteface Tests
    this->writeToCommandPort(PS2_CONTROLLER_COMMAND_TEST_FIRST_PORT);
    m_firstChannelFunctioning = (this->readFromDataPort() == 0x00);

    if(m_isDuelChannel)
    {
      this->writeToCommandPort(PS2_CONTROLLER_COMMAND_TEST_SECOND_PORT);
      m_secondChannelFunctioning = (this->readFromDataPort() == 0x00);
    }
    else
      m_secondChannelFunctioning = false; // For consistency
    // 8: Enable devices 
    this->writeToCommandPort(PS2_CONTROLLER_COMMAND_ENABLE_FIRST_DEVICE);
    this->writeToCommandPort(PS2_CONTROLLER_COMMAND_ENABLE_SECOND_DEVICE);

    this->writeToCommandPort(PS2_CONTROLLER_COMMAND_READ_CONTROLLER_CONFIGURATION);
    configurationByte = this->readFromDataPort();
    configurationByte |= 0x03; // Enable interrupts

    this->writeToCommandPort(PS2_CONTROLLER_COMMAND_WRITE_CONTROLLER_CONFIGURATION);
    this->writeToDataPort(configurationByte); // Enable interupts and translation

    // 9: Reset devices
    this->writeToDataPort(PS2_DEVICE_COMMAND_RESET);
    if(this->readFromDataPort() == PS2_DEVICE_COMMAND_ACK)
      m_firstChannelFunctioning = (this->readFromDataPort() == PS2_DEVICE_RESET_SUCCESS);
    else
      m_firstChannelFunctioning = false;

    if(m_secondChannelFunctioning)
    {
      this->writeToCommandPort(PS2_CONTROLLER_COMMAND_WRITE_SECOND_PORT_INPUT);
      this->writeToDataPort(PS2_DEVICE_COMMAND_RESET);
      if(this->readFromDataPort() == PS2_DEVICE_COMMAND_ACK)
        m_firstChannelFunctioning = (this->readFromDataPort() == PS2_DEVICE_RESET_SUCCESS);
      else
        m_firstChannelFunctioning = false;
    }
  }

  void PS2Controller::writeToDataPort(uint8_t data)
  {
    while(assembly::inb(PS2_CONTROLLER_STATUS_PORT) & 0x02);
    assembly::outb(PS2_CONTROLLER_DATA_PORT, data);
  }

  void PS2Controller::writeToCommandPort(uint8_t data)
  {
    while(assembly::inb(PS2_CONTROLLER_STATUS_PORT) & 0x02);
    assembly::outb(PS2_CONTROLLER_COMMAND_PORT, data);
  }

  uint8_t PS2Controller::readFromDataPort()
  {
    while(!(assembly::inb(PS2_CONTROLLER_STATUS_PORT) & 0x01));
    return assembly::inb(PS2_CONTROLLER_DATA_PORT);
  }

  void PS2Controller::flushDataPort()
  {
    while((assembly::inb(PS2_CONTROLLER_STATUS_PORT) & 0x01))
      assembly::inb(PS2_CONTROLLER_DATA_PORT);
  }

  //PS2Controller ps2Controller;
}
