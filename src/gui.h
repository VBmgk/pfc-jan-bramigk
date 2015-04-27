#ifndef GUI_H
#define GUI_H

void gui_init(void);
void gui_shutdown(void);

void gui_new_frame(void);
void gui_render(void);
void gui_sync(void);
bool gui_should_close(void);

#endif
