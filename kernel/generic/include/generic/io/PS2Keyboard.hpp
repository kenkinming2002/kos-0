#pragma once

#include <generic/utils/RingBuffer.hpp>

#include <stdint.h>
#include <optional>

namespace io
{
  class PS2Keyboard
  {
  public:
    PS2Keyboard();

  public:
    void push(uint8_t scanCode);

  public:
    enum class KeyCode : uint8_t
    {
      KEY_0 = '0',
      KEY_1 = '1',
      KEY_2 = '2',
      KEY_3 = '3',
      KEY_4 = '4',
      KEY_5 = '5',
      KEY_6 = '6',
      KEY_7 = '7',
      KEY_8 = '8',
      KEY_9 = '9', 
      KEY_a = 'a', 
      KEY_b = 'b',
      KEY_c = 'c',
      KEY_d = 'd',
      KEY_e = 'e',
      KEY_f = 'f',
      KEY_g = 'g',
      KEY_h = 'h',
      KEY_i = 'i',
      KEY_j = 'j',
      KEY_k = 'k',
      KEY_l = 'l',
      KEY_m = 'm',
      KEY_n = 'n',
      KEY_o = 'o',
      KEY_p = 'p',
      KEY_q = 'q',
      KEY_r = 'r',
      KEY_s = 's',
      KEY_t = 't',
      KEY_u = 'u',
      KEY_v = 'v',
      KEY_w = 'w',
      KEY_x = 'x',
      KEY_y = 'y',
      KEY_z = 'z',

      KEY_A = 'A',
      KEY_B = 'B',
      KEY_C = 'C',
      KEY_D = 'D',
      KEY_E = 'E',
      KEY_F = 'F',
      KEY_G = 'G',
      KEY_H = 'H',
      KEY_I = 'I',
      KEY_J = 'J',
      KEY_K = 'K',
      KEY_L = 'L',
      KEY_M = 'M',
      KEY_N = 'N',
      KEY_O = 'O',
      KEY_P = 'P',
      KEY_Q = 'Q',
      KEY_R = 'R',
      KEY_S = 'S',
      KEY_T = 'T',
      KEY_U = 'U',
      KEY_V = 'V',
      KEY_W = 'W',
      KEY_X = 'X',
      KEY_Y = 'Y',
      KEY_Z = 'Z',

      KEY_EXCLAIMATION_MARK = '!',
      KEY_AT = '@',
      KEY_HASH = '#',
      KEY_DOLLAR = '$',
      KEY_PERCENT = '%',
      KEY_CARET = '^',
      KEY_AMPERSAND = '&',
      KEY_ASTERISK = '*',
      KEY_LPARENTHESIS = '(',
      KEY_RPARENTHESIS = ')',
      KEY_MINUS = '-',
      KEY_UNDERSCORE = '_',
      KEY_PLUS = '+',
      KEY_EQUAL = '=',
      KEY_LBRACKET = '[',
      KEY_RBRACKET = ']',
      KEY_LBRACE = '{',
      KEY_RBRACE = '}',
      KEY_VERTICAL_BAR = '|',
      KEY_BACKSLASH = '\\',
      KEY_FORWARDSLASH = '/',
      KEY_LESSTHAN = '<',
      KEY_GREATERTHAN = '>',
      KEY_COMMA = ',',
      KEY_FULLSTOP = '.',
      KEY_QUESTION_MARK = '?',
      KEY_BACKTICK = '`',
      KEY_TILDE = '~',
      KEY_COLON = ':',
      KEY_SEMICOLON = ';',
      KEY_SINGLEQUOTE = '\'',
      KEY_DOUBLEQUOTE = '"',
      KEY_SPACE = ' ',
      KEY_TAB = '\t',
      KEY_BACKSPACE = '\b',

      // Keys without ascii value
      KEY_ENTER = 0x80, // Enter maps to \r\n
      KEY_LALT,
      KEY_RALT,
      KEY_LSHIFT,
      KEY_RSHIFT,
      KEY_LCTRL,
      KEY_RCTRL,
      KEY_CAPSLOCK,

      KEY_KEYPAD_0,
      KEY_KEYPAD_1,
      KEY_KEYPAD_2,
      KEY_KEYPAD_3,
      KEY_KEYPAD_4,
      KEY_KEYPAD_5,
      KEY_KEYPAD_6,
      KEY_KEYPAD_7,
      KEY_KEYPAD_8,
      KEY_KEYPAD_9,
      KEY_KEYPAD_PLUS,
      KEY_KEYPAD_MINUS,
      KEY_KEYPAD_MULTIPLY,
      KEY_KEYPAD_DIVIDE,
      KEY_KEYPAD_DOT,

      KEY_NUMERLOCK,
      KEY_SCROLLLOCK,
      KEY_ESCAPE,

      KEY_F1,
      KEY_F2,
      KEY_F3,
      KEY_F4,
      KEY_F5,
      KEY_F6,
      KEY_F7,
      KEY_F8,
      KEY_F9,
      KEY_F10,
      KEY_F11,
      KEY_F12,

      KEY_UNSUPPORTED = 0xFE,
      KEY_NONE = 0xFF
    };

    enum class KeyState
    {
      PRESSED,
      RELEASED
    };

    struct Modifier
    {
      uint8_t ctrl : 1;
      uint8_t shift : 1;
      uint8_t alt : 1;
    };

    struct Event
    {
      KeyCode keyCode;
      KeyState keyState;
      Modifier modifier;

      // Note: this is not thread-safe and not reentrant
      std::optional<const char*> toAscii() const
      {
        static char buf[2] = {'\0', '\0'};

        if(static_cast<uint8_t>(keyCode)<0x80)
        {
          buf[0] = static_cast<uint8_t>(keyCode);
          return buf;
        }

        if(static_cast<uint8_t>(keyCode) >= static_cast<uint8_t>(KeyCode::KEY_KEYPAD_0) &&
           static_cast<uint8_t>(keyCode) <= static_cast<uint8_t>(KeyCode::KEY_KEYPAD_9))
        {
          buf[0] = '0' + (static_cast<uint8_t>(keyCode) - static_cast<uint8_t>(KeyCode::KEY_KEYPAD_0));
          return buf;
        }

        switch(keyCode)
        {
        case KeyCode::KEY_ENTER:
          return "\r\n";
        case KeyCode::KEY_KEYPAD_PLUS:
          return "+";
        case KeyCode::KEY_KEYPAD_MINUS:
          return "-";
        case KeyCode::KEY_KEYPAD_MULTIPLY:
          return "*";
        case KeyCode::KEY_KEYPAD_DIVIDE:
          return "/";
        case KeyCode::KEY_KEYPAD_DOT:
          return ".";
        default:
          return std::nullopt;
        }
      }
    };

  public:
    std::optional<Event> poll();

  private:
    KeyState m_nextKeyState = KeyState::PRESSED;
    Modifier m_currentModifer = {0,0,0};

  private:
    utils::RingBuffer<uint8_t, 64> m_buffer;
  };

  extern PS2Keyboard ps2Keyboard;
}
