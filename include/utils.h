#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <time.h>

void ensure_storage(void);
void hash_pin(const char *pin, char outHex[65]);
int read_int_range(const char *prompt, int min, int max);
double read_double_min(const char *prompt, double min);
void read_string(const char *prompt, char *buf, size_t size);
int confirm_yn(const char *prompt);
int validate_pin_format(const char *pinStr);
int validate_username_format(const char *u);
int validate_phone_format(const char *p);
void trim_newline(char *s);
void format_time_str(time_t value, char *buf, size_t size);

#endif
