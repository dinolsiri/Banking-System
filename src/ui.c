#include <stdio.h>
#include <string.h>
#include <time.h>

#include "ui.h"

void ui_clear(void) {
    printf("\x1b[2J\x1b[H");
}

static void ui_line(char ch, int width, const char *color) {
    int i;
    printf("%s", color);
    for (i = 0; i < width; ++i) {
        putchar(ch);
    }
    printf("%s\n", ANSI_RESET);
}

void ui_header(const char *title) {
    ui_clear();
    ui_line('=', 74, ANSI_CYAN);
    printf("%s%s                  N E O N   B A N K   C O R E%s\n",
           ANSI_BOLD, ANSI_MAGENTA, ANSI_RESET);
    ui_line('-', 74, ANSI_BLUE);
    printf("%s[%s%s%s]%s %s%s%s\n",
           ANSI_CYAN, ANSI_GREEN, "SYSTEM", ANSI_CYAN, ANSI_RESET,
           ANSI_BOLD, title, ANSI_RESET);
    ui_line('=', 74, ANSI_CYAN);
}

void ui_box(const char *text) {
    size_t len = strlen(text);
    size_t width = len + 4U;
    size_t i;

    printf("%s+", ANSI_BLUE);
    for (i = 0; i < width; ++i) {
        putchar('-');
    }
    printf("+%s\n", ANSI_RESET);
    printf("%s|  %s%s%s  |\n", ANSI_BLUE, ANSI_GREEN, text, ANSI_BLUE);
    printf("+");
    for (i = 0; i < width; ++i) {
        putchar('-');
    }
    printf("+%s\n", ANSI_RESET);
}

void ui_pause(void) {
    char tmp[8];
    printf("%sPress Enter to continue...%s", ANSI_DIM, ANSI_RESET);
    fgets(tmp, sizeof(tmp), stdin);
}

void ui_loading(const char *msg) {
    const char spinner[] = {'|', '/', '-', '\\'};
    int i;

    printf("%s", ANSI_CYAN);
    for (i = 0; i < 8; ++i) {
        printf("\r[%c] %s", spinner[i % 4], msg);
        fflush(stdout);
        clock_t start = clock();
        while ((clock() - start) < CLOCKS_PER_SEC / 10) {
        }
    }
    printf("\r[+] %s%s\n", msg, ANSI_RESET);
}

void ui_input_prompt(const char *label) {
    printf("%s>%s %s: ", ANSI_CYAN, ANSI_RESET, label);
}

void ui_print_error(const char *msg) {
    printf("%s[ERROR]%s %s\n", ANSI_RED, ANSI_RESET, msg);
}

void ui_print_success(const char *msg) {
    printf("%s[OK]%s %s\n", ANSI_GREEN, ANSI_RESET, msg);
}
