#include <generic/io/PS2Keyboard.hpp>

#include <i686/core/Interrupt.hpp>

#include <intel/core/pic/8259.hpp>
#include <intel/asm/io.hpp>

#include <generic/io/Print.hpp>

#include <generic/io/PS2Controller.hpp>

namespace io
{
  namespace
  {
    [[gnu::interrupt]] [[gnu::no_caller_saved_registers]] void keyboard_interrupt_handler([[maybe_unused]]core::interrupt::frame* frame)
    {
      uint8_t scanCode = assembly::inb(0x60);
      ps2Keyboard.push(scanCode);

      core::pic::controller8259::acknowledge(0x1);
    }
  }

  PS2Keyboard::PS2Keyboard()
  {
    core::interrupt::install_handler(0x21, core::PrivillegeLevel::RING0, reinterpret_cast<uintptr_t>(&keyboard_interrupt_handler));
    // TODO: Verify that device 1 is keyboard
    core::pic::controller8259::clearMask(1); // Enable keyboard
  }

  void PS2Keyboard::push(uint8_t scanCode)
  {
    m_buffer.push(scanCode);
  }


  std::optional<PS2Keyboard::Event> PS2Keyboard::poll()
  {
    static constexpr KeyCode qwertyKeyCodeNormal[256] = 
    {
      KeyCode::KEY_NONE,            KeyCode::KEY_F9,          KeyCode::KEY_NONE,         KeyCode::KEY_F5, 
      KeyCode::KEY_F3,              KeyCode::KEY_F1,          KeyCode::KEY_F2,           KeyCode::KEY_F12,
      KeyCode::KEY_NONE,            KeyCode::KEY_F10,         KeyCode::KEY_F8,           KeyCode::KEY_F6,
      KeyCode::KEY_F4,              KeyCode::KEY_TAB,         KeyCode::KEY_BACKTICK,     KeyCode::KEY_NONE,
      KeyCode::KEY_NONE,            KeyCode::KEY_LALT,        KeyCode::KEY_LSHIFT,       KeyCode::KEY_NONE, 
      KeyCode::KEY_LCTRL,           KeyCode::KEY_q,           KeyCode::KEY_1,            KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_z,            KeyCode::KEY_s, 
      KeyCode::KEY_a,               KeyCode::KEY_w,           KeyCode::KEY_2,            KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_c,           KeyCode::KEY_x,            KeyCode::KEY_d, 
      KeyCode::KEY_e,               KeyCode::KEY_4,           KeyCode::KEY_3,            KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_SPACE,       KeyCode::KEY_v,            KeyCode::KEY_f, 
      KeyCode::KEY_t,               KeyCode::KEY_r,           KeyCode::KEY_5,            KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_n,           KeyCode::KEY_b,            KeyCode::KEY_h, 
      KeyCode::KEY_g,               KeyCode::KEY_y,           KeyCode::KEY_6,            KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_m,            KeyCode::KEY_j, 
      KeyCode::KEY_u,               KeyCode::KEY_7,           KeyCode::KEY_8,            KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_COMMA,       KeyCode::KEY_k,            KeyCode::KEY_i, 
      KeyCode::KEY_o,               KeyCode::KEY_0,           KeyCode::KEY_9,            KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_FULLSTOP,    KeyCode::KEY_FORWARDSLASH, KeyCode::KEY_l, 
      KeyCode::KEY_SEMICOLON,       KeyCode::KEY_p,           KeyCode::KEY_MINUS,        KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_SINGLEQUOTE,  KeyCode::KEY_NONE, 
      KeyCode::KEY_LBRACKET,        KeyCode::KEY_EQUAL,       KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_CAPSLOCK,        KeyCode::KEY_RSHIFT,      KeyCode::KEY_ENTER,        KeyCode::KEY_RBRACKET, 
      KeyCode::KEY_NONE,            KeyCode::KEY_BACKSLASH,   KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_BACKSPACE,    KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_KEYPAD_1,    KeyCode::KEY_NONE,         KeyCode::KEY_KEYPAD_4, 
      KeyCode::KEY_KEYPAD_7,        KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_KEYPAD_0,        KeyCode::KEY_KEYPAD_DOT,  KeyCode::KEY_KEYPAD_2,     KeyCode::KEY_KEYPAD_5, 
      KeyCode::KEY_KEYPAD_6,        KeyCode::KEY_KEYPAD_8,    KeyCode::KEY_ESCAPE,       KeyCode::KEY_NUMERLOCK, 
      KeyCode::KEY_F11,             KeyCode::KEY_KEYPAD_PLUS, KeyCode::KEY_KEYPAD_3,     KeyCode::KEY_KEYPAD_MINUS, 
      KeyCode::KEY_KEYPAD_MULTIPLY, KeyCode::KEY_KEYPAD_9,    KeyCode::KEY_SCROLLLOCK,   KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_F7, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
    };

    static constexpr KeyCode qwertyKeyCodeShift[256] = 
    {
      KeyCode::KEY_NONE,            KeyCode::KEY_F9,          KeyCode::KEY_NONE,         KeyCode::KEY_F5, 
      KeyCode::KEY_F3,              KeyCode::KEY_F1,          KeyCode::KEY_F2,           KeyCode::KEY_F12,
      KeyCode::KEY_NONE,            KeyCode::KEY_F10,         KeyCode::KEY_F8,           KeyCode::KEY_F6,
      KeyCode::KEY_F4,              KeyCode::KEY_TAB,         KeyCode::KEY_TILDE,        KeyCode::KEY_NONE,
      KeyCode::KEY_NONE,            KeyCode::KEY_LALT,        KeyCode::KEY_LSHIFT,       KeyCode::KEY_NONE, 
      KeyCode::KEY_LCTRL,           KeyCode::KEY_Q,           KeyCode::KEY_EXCLAIMATION_MARK,  KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_Z,            KeyCode::KEY_S, 
      KeyCode::KEY_A,               KeyCode::KEY_W,           KeyCode::KEY_AT,           KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_C,           KeyCode::KEY_X,            KeyCode::KEY_D, 
      KeyCode::KEY_E,               KeyCode::KEY_DOLLAR,      KeyCode::KEY_HASH,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_SPACE,       KeyCode::KEY_V,            KeyCode::KEY_F, 
      KeyCode::KEY_T,               KeyCode::KEY_R,           KeyCode::KEY_PERCENT,      KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_N,           KeyCode::KEY_B,            KeyCode::KEY_H, 
      KeyCode::KEY_G,               KeyCode::KEY_Y,           KeyCode::KEY_CARET,        KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_M,            KeyCode::KEY_J, 
      KeyCode::KEY_U,               KeyCode::KEY_AMPERSAND,   KeyCode::KEY_ASTERISK,     KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_LESSTHAN,    KeyCode::KEY_K,            KeyCode::KEY_I, 
      KeyCode::KEY_O,               KeyCode::KEY_LPARENTHESIS,KeyCode::KEY_RPARENTHESIS, KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_GREATERTHAN, KeyCode::KEY_QUESTION_MARK,KeyCode::KEY_L, 
      KeyCode::KEY_COLON,           KeyCode::KEY_P,           KeyCode::KEY_UNDERSCORE,   KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_DOUBLEQUOTE,  KeyCode::KEY_NONE, 
      KeyCode::KEY_LBRACE,          KeyCode::KEY_PLUS,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_CAPSLOCK,        KeyCode::KEY_RSHIFT,      KeyCode::KEY_ENTER,        KeyCode::KEY_RBRACE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_VERTICAL_BAR,KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_BACKSPACE,    KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_KEYPAD_1,    KeyCode::KEY_NONE,         KeyCode::KEY_KEYPAD_4, 
      KeyCode::KEY_KEYPAD_7,        KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_KEYPAD_0,        KeyCode::KEY_KEYPAD_DOT,  KeyCode::KEY_KEYPAD_2,     KeyCode::KEY_KEYPAD_5, 
      KeyCode::KEY_KEYPAD_6,        KeyCode::KEY_KEYPAD_8,    KeyCode::KEY_ESCAPE,       KeyCode::KEY_NUMERLOCK, 
      KeyCode::KEY_F11,             KeyCode::KEY_KEYPAD_PLUS, KeyCode::KEY_KEYPAD_3,     KeyCode::KEY_KEYPAD_MINUS, 
      KeyCode::KEY_KEYPAD_MULTIPLY, KeyCode::KEY_KEYPAD_9,    KeyCode::KEY_SCROLLLOCK,   KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_F7, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
      KeyCode::KEY_NONE,            KeyCode::KEY_NONE,        KeyCode::KEY_NONE,         KeyCode::KEY_NONE, 
    };

    auto getNextEvent = [this](KeyCode keyCode) {
        auto event = Event{keyCode, m_nextKeyState, m_currentModifer};
        m_nextKeyState = KeyState::PRESSED;
        return event;
    };

    while(!m_buffer.empty())
    {
      switch(auto scanCode = m_buffer.pop())
      {
      case 0xF0:
        m_nextKeyState = KeyState::RELEASED;
        break;
      default:
        if(m_currentModifer.shift)
        {
          switch(auto keyCode = qwertyKeyCodeShift[scanCode])
          {
          case KeyCode::KEY_NONE:
          case KeyCode::KEY_UNSUPPORTED:
            break;
          case KeyCode::KEY_LALT: 
          case KeyCode::KEY_RALT:
            m_currentModifer.alt = m_nextKeyState == KeyState::PRESSED;
            return getNextEvent(keyCode);
          case KeyCode::KEY_LSHIFT: 
          case KeyCode::KEY_RSHIFT:
            m_currentModifer.shift = m_nextKeyState == KeyState::PRESSED;
            return getNextEvent(keyCode);
          case KeyCode::KEY_LCTRL: 
          case KeyCode::KEY_RCTRL:
            m_currentModifer.ctrl = m_nextKeyState == KeyState::PRESSED;
            return getNextEvent(keyCode);
          default:
            return getNextEvent(keyCode);
          }
        }
        else
        {
          switch(auto keyCode = qwertyKeyCodeNormal[scanCode])
          {
          case KeyCode::KEY_NONE:
          case KeyCode::KEY_UNSUPPORTED:
            break;
          case KeyCode::KEY_LALT: 
          case KeyCode::KEY_RALT:
            m_currentModifer.alt = m_nextKeyState == KeyState::PRESSED;
            return getNextEvent(keyCode);
          case KeyCode::KEY_LSHIFT: 
          case KeyCode::KEY_RSHIFT:
            m_currentModifer.shift = m_nextKeyState == KeyState::PRESSED;
            return getNextEvent(keyCode);
          case KeyCode::KEY_LCTRL: 
          case KeyCode::KEY_RCTRL:
            m_currentModifer.ctrl = m_nextKeyState == KeyState::PRESSED;
            return getNextEvent(keyCode);
          default:
            return getNextEvent(keyCode);
          }
        }
        break;
      }
    }
    return std::nullopt;
  }

  PS2Keyboard ps2Keyboard;
}
