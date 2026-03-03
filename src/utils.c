#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>
#endif

#include "models.h"
#include "ui.h"
#include "utils.h"

static int make_dir_if_missing(const char *path) {
#ifdef _WIN32
    if (_mkdir(path) == 0 || errno == EEXIST) {
        return 1;
    }
#else
    if (mkdir(path, 0777) == 0 || errno == EEXIST) {
        return 1;
    }
#endif
    return 0;
}

static void ensure_file(const char *path) {
    FILE *fp = fopen(path, "ab");
    if (fp != NULL) {
        fclose(fp);
    }
}

void ensure_storage(void) {
    if (!make_dir_if_missing(DATA_DIR)) {
        ui_print_error("Failed to create data directory.");
    }
    if (!make_dir_if_missing(STATEMENTS_DIR)) {
        ui_print_error("Failed to create statements directory.");
    }
    ensure_file(ACCOUNTS_FILE);
    ensure_file(TRANSACTIONS_FILE);
}

void trim_newline(char *s) {
    size_t len;
    if (s == NULL) {
        return;
    }
    len = strlen(s);
    while (len > 0U && (s[len - 1U] == '\n' || s[len - 1U] == '\r')) {
        s[len - 1U] = '\0';
        --len;
    }
}

static void read_line_raw(char *buf, size_t size) {
    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        clearerr(stdin);
        return;
    }
    if (strchr(buf, '\n') == NULL) {
        int ch;
        do {
            ch = getchar();
        } while (ch != '\n' && ch != EOF);
    }
    trim_newline(buf);
}

void read_string(const char *prompt, char *buf, size_t size) {
    for (;;) {
        ui_input_prompt(prompt);
        read_line_raw(buf, size);
        if (buf[0] != '\0') {
            return;
        }
        ui_print_error("Input cannot be empty.");
    }
}

int read_int_range(const char *prompt, int min, int max) {
    char buf[64];
    char *end = NULL;
    long value;

    for (;;) {
        ui_input_prompt(prompt);
        read_line_raw(buf, sizeof(buf));
        errno = 0;
        value = strtol(buf, &end, 10);
        if (errno == 0 && end != buf && *end == '\0' && value >= min && value <= max) {
            return (int)value;
        }
        ui_print_error("Enter a valid number within range.");
    }
}

double read_double_min(const char *prompt, double min) {
    char buf[64];
    char *end = NULL;
    double value;

    for (;;) {
        ui_input_prompt(prompt);
        read_line_raw(buf, sizeof(buf));
        errno = 0;
        value = strtod(buf, &end);
        if (errno == 0 && end != buf && *end == '\0' && value >= min) {
            return value;
        }
        ui_print_error("Enter a valid amount.");
    }
}

int confirm_yn(const char *prompt) {
    char buf[8];
    for (;;) {
        ui_input_prompt(prompt);
        read_line_raw(buf, sizeof(buf));
        if (buf[0] == 'y' || buf[0] == 'Y') {
            return 1;
        }
        if (buf[0] == 'n' || buf[0] == 'N') {
            return 0;
        }
        ui_print_error("Enter Y or N.");
    }
}

int validate_pin_format(const char *pinStr) {
    size_t len;
    size_t i;
    if (pinStr == NULL) {
        return 0;
    }
    len = strlen(pinStr);
    if (len < 4U || len > 6U) {
        return 0;
    }
    for (i = 0; i < len; ++i) {
        if (!isdigit((unsigned char)pinStr[i])) {
            return 0;
        }
    }
    return 1;
}

int validate_username_format(const char *u) {
    size_t len;
    size_t i;
    if (u == NULL) {
        return 0;
    }
    len = strlen(u);
    if (len < 3U || len > 20U) {
        return 0;
    }
    for (i = 0; i < len; ++i) {
        unsigned char ch = (unsigned char)u[i];
        if (!(isalnum(ch) || ch == '.' || ch == '_')) {
            return 0;
        }
    }
    return 1;
}

int validate_phone_format(const char *p) {
    size_t len;
    size_t i;
    if (p == NULL) {
        return 0;
    }
    len = strlen(p);
    if (len < 7U || len > 15U) {
        return 0;
    }
    for (i = 0; i < len; ++i) {
        unsigned char ch = (unsigned char)p[i];
        if (i == 0U && ch == '+') {
            continue;
        }
        if (!isdigit(ch)) {
            return 0;
        }
    }
    return 1;
}

void hash_pin(const char *pin, char outHex[65]) {
    static const char salt[] = "NEON_BANK_V1";
    unsigned long long state[4] = {
        1469598103934665603ULL,
        1099511628211ULL ^ 0x9e3779b97f4a7c15ULL,
        0xcbf29ce484222325ULL ^ 0xa0761d6478bd642FULL,
        0x84222325cbf29ce4ULL ^ 0xe7037ed1a0b428dbULL
    };
    const unsigned char *parts[2];
    size_t lens[2];
    int round;
    int part;
    int i;

    parts[0] = (const unsigned char *)salt;
    parts[1] = (const unsigned char *)pin;
    lens[0] = strlen(salt);
    lens[1] = strlen(pin);

    for (round = 0; round < 2048; ++round) {
        for (part = 0; part < 2; ++part) {
            size_t j;
            for (j = 0; j < lens[part]; ++j) {
                unsigned long long v = (unsigned long long)(parts[part][j] + round + (part * 17));
                state[part % 4] ^= v;
                state[part % 4] *= 1099511628211ULL;
                state[(part + 1) % 4] ^= (state[part % 4] >> 7);
                state[(part + 1) % 4] *= 1099511628211ULL;
            }
        }
        state[round % 4] ^= (unsigned long long)round * 0x100000001b3ULL;
        state[(round + 1) % 4] = (state[(round + 1) % 4] << 5) |
                                 (state[(round + 1) % 4] >> (64 - 5));
    }

    for (i = 0; i < 4; ++i) {
        sprintf(outHex + (i * 16), "%016llx", state[i]);
    }
    outHex[64] = '\0';
}

void format_time_str(time_t value, char *buf, size_t size) {
    struct tm *tmInfo = localtime(&value);
    if (tmInfo == NULL) {
        snprintf(buf, size, "N/A");
        return;
    }
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tmInfo);
}
