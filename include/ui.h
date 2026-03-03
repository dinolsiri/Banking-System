#ifndef UI_H
#define UI_H

#define ANSI_RESET "\x1b[0m"
#define ANSI_BOLD "\x1b[1m"
#define ANSI_CYAN "\x1b[96m"
#define ANSI_GREEN "\x1b[92m"
#define ANSI_YELLOW "\x1b[93m"
#define ANSI_RED "\x1b[91m"
#define ANSI_MAGENTA "\x1b[95m"
#define ANSI_BLUE "\x1b[94m"
#define ANSI_DIM "\x1b[2m"

void ui_clear(void);
void ui_header(const char *title);
void ui_box(const char *text);
void ui_pause(void);
void ui_loading(const char *msg);
void ui_input_prompt(const char *label);
void ui_print_error(const char *msg);
void ui_print_success(const char *msg);

#endif
