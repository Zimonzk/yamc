#ifndef __GUI_H_INCLUDED__
#define __GUI_H_INCLUDED__

typedef void (*buttoncb)(void *userdata);

void init_gui(void);
void gui_render(void);

void gui_input(int x, int y, char down);

/* Size actually specifies a minimum here.
 * For good looking texturing the size_x is rounded up to fit whole middle sections. */
int gui_add_button(float x, float y, float size_x, float size_y, const char *label, buttoncb bcb, void *userdata);
void gui_clear(void);

#endif
