#ifndef FRAMBUFFER_H
#define FRAMBUFFER_H

#include <stdint.h>
#include <grub/multiboot2.h>

#include <utils/Optional.hpp>

enum class FrameBufferColor
{
  BLACK         = 0,
  BLUE          = 1,
  GREEN         = 2,
  CYAN          = 3,
  RED           = 4,
  MAGENTA       = 5,
  BROWN         = 6,
  LIGHT_GREY    = 7,
  DARK_GREY     = 8,
  LIGHT_BLUE    = 9,
  LIGHT_GREEN   = 10,
  LIGHT_CYAN    = 11,
  LIGHT_RED     = 12,
  LIGHT_MAGENTA = 13,
  LIGHT_BROWN   = 14,
  WHITE         = 15
};

struct FrameBufferCursor
{
  uint32_t x, y;
};

struct FrameBufferSettings
{
};

class FrameBuffer
{
public:
  FrameBuffer() = default;
  FrameBuffer(uint16_t* cells, uint32_t width, uint32_t height);

public:
  static utils::Optional<FrameBuffer> create(struct multiboot_tag_framebuffer *multiboot_tag_framebuffer);
  static FrameBuffer createDefault();

public:
  int write(FrameBufferCursor cursor, char c, FrameBufferColor fg, FrameBufferColor bg) const;
  int write(FrameBufferCursor cursor, const char* buf, uint32_t len, FrameBufferColor fg, FrameBufferColor bg) const;

private:
  uint16_t* m_cells;
  uint32_t  m_width, m_height;
  FrameBufferSettings m_settings;
};

///** fb_create
// *  Create a framebuffer object from multiboot framebuffer tag.
// *
// *  @param fb                        Pointer to memory to store created framebuffer object
// *  @param multiboot_tag_framebuffer Multuboot framebuffer tag to create
// *                                   framebuffer from
// *
// *  @return 0 on success, negative value on failure
// */
//int fb_create(fb_t *fb, struct multiboot_tag_framebuffer* mutiboot_tag_framebuffer);
//
///** fb_create
// *  Create a default framebuffer object
// *
// *  @param fb                        Pointer to memory to store created framebuffer object
// *
// *  @return 0 on success, negative value on failure
// */
//int fb_create_default(fb_t *fb);
//
///** fb_write_cell
// *  Writes a character with the given foreground and background to position i in
// *  the framebuffer.
// *
// *  @param fb     The framebuffer object
// *  @param cursor The coordinate to write to
// *  @param c      The character to write
// *  @param fg     Foreground color
// *  @param bg     Background color
// *
// *  @return 0 on success, negative value on failure
// */
//int fb_write_cell(fb_t *fb, fb_cursor_t cursor, char c, fb_color_t fg, fb_color_t bg);
//
///** fb_move_cursor
// *  Moves the cursor of the frambuffer to the given position
// *
// *  @param fb     The framebuffer object
// *  @param cursor The new cursor
// *
// *  @return 0 on success, negative value on failure
// */
//int fb_set_cursor(fb_t *fb, fb_cursor_t cursor);
//
///** fb_write
// *  Writes a string to the framebuffer with given foreground and background
// *
// *  @param fb     The framebuffer object
// *  @param cursor The coordinate to write to
// *  @param buf    The buffer pointing to the string to be written
// *  @param len    The length of the buffer to be written
// *  @param fg     The foreground color
// *  @param bg     The background color
// *
// *  @return If succeeded, number of character written. Otherwise, a negative
// *          value
// */
//int fb_write(fb_t *fb, fb_cursor_t cursor, const char* buf, unsigned int len, unsigned char fg, unsigned char bg);

#endif // FRAMBUFFER_H
