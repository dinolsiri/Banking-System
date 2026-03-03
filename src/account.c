#include <stdio.h>
#include <string.h>
#include <time.h>
#include "account.h"
#include "ui.h"
#include "utils.h"

static int username_exists(const char *username) {
    return find_account_by_username(username, NULL, NULL);
}

int next_account_number(void) {
    FILE *fp = fopen(ACCOUNTS_FILE, "rb");
    Account account;
    int maxNo = 100000;

    if (fp == NULL) {
        return maxNo + 1;
    }
    while (fread(&account, sizeof(Account), 1U, fp) == 1U) {
        if (account.accountNo > maxNo) {
            maxNo = account.accountNo;
        }
    }
    fclose(fp);
    return maxNo + 1;
}

int rewrite_account_at(long offset, const Account *account) {
    FILE *fp = fopen(ACCOUNTS_FILE, "r+b");
    if (fp == NULL) {
        ui_print_error("Unable to open accounts file for update.");
        return 0;
    }
    if (fseek(fp, offset, SEEK_SET) != 0) {
        fclose(fp);
        ui_print_error("Unable to seek to account record.");
        return 0;
    }
    if (fwrite(account, sizeof(Account), 1U, fp) != 1U) {
        fclose(fp);
        ui_print_error("Failed to rewrite account record.");
        return 0;
    }
    fclose(fp);
    return 1;
}

static int find_account(FILE *fp, int mode, const char *textKey, int intKey,
                        Account *outAccount, long *outOffset) {
    Account account;
    while (1) {
        long offset = ftell(fp);
        if (fread(&account, sizeof(Account), 1U, fp) != 1U) {
            break;
        }
        if ((mode == 0 && account.accountNo == intKey) ||
            (mode == 1 && strcmp(account.username, textKey) == 0) ||
            (mode == 2 && strcmp(account.nic, textKey) == 0)) {
            if (outAccount != NULL) {
                *outAccount = account;
            }
            if (outOffset != NULL) {
                *outOffset = offset;
            }
            return 1;
        }
    }
    return 0;
}

int find_account_by_no(int accountNo, Account *outAccount, long *outOffset) {
    FILE *fp = fopen(ACCOUNTS_FILE, "rb");
    int found = 0;
    if (fp == NULL) {
        return 0;
    }
    found = find_account(fp, 0, NULL, accountNo, outAccount, outOffset);
    fclose(fp);
    return found;
}

int find_account_by_username(const char *username, Account *outAccount, long *outOffset) {
    FILE *fp = fopen(ACCOUNTS_FILE, "rb");
    int found = 0;
    if (fp == NULL) {
        return 0;
    }
    found = find_account(fp, 1, username, 0, outAccount, outOffset);
    fclose(fp);
    return found;
}

int find_account_by_nic(const char *nic, Account *outAccount, long *outOffset) {
    FILE *fp = fopen(ACCOUNTS_FILE, "rb");
    int found = 0;
    if (fp == NULL) {
        return 0;
    }
    found = find_account(fp, 2, nic, 0, outAccount, outOffset);
    fclose(fp);
    return found;
}

static void input_pin_hash(char outHash[PIN_HASH_LEN]) {
    char pin[16];
    for (;;) {
        read_string("Set PIN (4-6 digits)", pin, sizeof(pin));
        if (!validate_pin_format(pin)) {
            ui_print_error("PIN must be 4-6 digits.");
            continue;
        }
        hash_pin(pin, outHash);
        return;
    }
}

void print_account_summary(const Account *account) {
    char created[32];
    format_time_str(account->createdAt, created, sizeof(created));
    printf("%sAccount No:%s %d\n", ANSI_CYAN, ANSI_RESET, account->accountNo);
    printf("%sName:%s %s\n", ANSI_CYAN, ANSI_RESET, account->fullName);
    printf("%sUsername:%s %s\n", ANSI_CYAN, ANSI_RESET, account->username);
    printf("%sNIC:%s %s\n", ANSI_CYAN, ANSI_RESET, account->nic);
    printf("%sPhone:%s %s\n", ANSI_CYAN, ANSI_RESET, account->phone);
    printf("%sAddress:%s %s\n", ANSI_CYAN, ANSI_RESET, account->address);
    printf("%sBalance:%s %.2f\n", ANSI_CYAN, ANSI_RESET, account->balance);
    printf("%sStatus:%s %s | Locked: %s | Failed Attempts: %d\n",
           ANSI_CYAN, ANSI_RESET, account->isActive ? "ACTIVE" : "CLOSED",
           account->isLocked ? "YES" : "NO",
           account->failedAttempts);
    printf("%sCreated:%s %s\n", ANSI_CYAN, ANSI_RESET, created);
}

int create_account(void) {
    FILE *fp;
    Account account;
    char username[USERNAME_LEN];

    ui_header("Create Account");
    memset(&account, 0, sizeof(account));
    account.accountNo = next_account_number();
    read_string("Full name", account.fullName, sizeof(account.fullName));
    read_string("NIC", account.nic, sizeof(account.nic));
    do {
        read_string("Phone", account.phone, sizeof(account.phone));
        if (!validate_phone_format(account.phone)) {
            ui_print_error("Invalid phone format.");
        }
    } while (!validate_phone_format(account.phone));
    read_string("Address", account.address, sizeof(account.address));
    do {
        read_string("Username", username, sizeof(username));
        if (!validate_username_format(username)) {
            ui_print_error("Username must be 3-20 chars and only letters/digits/._");
            continue;
        }
        if (username_exists(username)) {
            ui_print_error("Username already exists.");
            continue;
        }
        strcpy(account.username, username);
        break;
    } while (1);
    input_pin_hash(account.pinHash);
    account.balance = read_double_min("Initial deposit", 0.0);
    account.isActive = 1;
    account.failedAttempts = 0;
    account.isLocked = 0;
    account.createdAt = time(NULL);

    fp = fopen(ACCOUNTS_FILE, "ab");
    if (fp == NULL) {
        ui_print_error("Unable to open accounts file.");
        return 0;
    }
    if (fwrite(&account, sizeof(Account), 1U, fp) != 1U) {
        fclose(fp);
        ui_print_error("Failed to save account.");
        return 0;
    }
    fclose(fp);
    ui_print_success("Account created successfully.");
    print_account_summary(&account);
    return 1;
}

void list_all_accounts(void) {
    FILE *fp = fopen(ACCOUNTS_FILE, "rb");
    Account account;
    int index = 0;

    ui_header("All Accounts");
    if (fp == NULL) {
        ui_print_error("Unable to open accounts file.");
        return;
    }
    while (fread(&account, sizeof(Account), 1U, fp) == 1U) {
        printf("%s[%d]%s --------------------------------------------\n", ANSI_MAGENTA, ++index, ANSI_RESET);
        print_account_summary(&account);
    }
    if (index == 0) {
        ui_box("No accounts found.");
    }
    fclose(fp);
}

int search_account_interactive(Account *outAccount, long *outOffset) {
    int choice;
    char key[64];
    Account account;
    long offset = 0L;
    int found = 0;

    ui_header("Search Account");
    printf("1. By Account Number\n");
    printf("2. By Username\n");
    printf("3. By NIC\n");
    choice = read_int_range("Select option", 1, 3);

    if (choice == 1) {
        int accountNo = read_int_range("Account number", 1, 999999999);
        found = find_account_by_no(accountNo, &account, &offset);
    } else if (choice == 2) {
        read_string("Username", key, sizeof(key));
        found = find_account_by_username(key, &account, &offset);
    } else {
        read_string("NIC", key, sizeof(key));
        found = find_account_by_nic(key, &account, &offset);
    }

    if (!found) {
        ui_print_error("Account not found.");
        return 0;
    }

    ui_print_success("Account found.");
    print_account_summary(&account);
    if (outAccount != NULL) {
        *outAccount = account;
    }
    if (outOffset != NULL) {
        *outOffset = offset;
    }
    return 1;
}

int update_account_interactive(void) {
    Account account;
    long offset;
    char buf[ADDRESS_LEN];

    if (!search_account_interactive(&account, &offset)) {
        return 0;
    }
    if (!account.isActive) {
        ui_print_error("Closed accounts cannot be updated.");
        return 0;
    }

    read_string("New full name", account.fullName, sizeof(account.fullName));
    read_string("New NIC", account.nic, sizeof(account.nic));
    do {
        read_string("New phone", buf, sizeof(buf));
        if (!validate_phone_format(buf)) {
            ui_print_error("Invalid phone format.");
        }
    } while (!validate_phone_format(buf));
    strcpy(account.phone, buf);
    read_string("New address", account.address, sizeof(account.address));
    if (!rewrite_account_at(offset, &account)) {
        return 0;
    }
    ui_print_success("Account updated.");
    return 1;
}

int close_account_interactive(void) {
    Account account;
    long offset;

    if (!search_account_interactive(&account, &offset)) {
        return 0;
    }
    if (!account.isActive) {
        ui_print_error("Account is already closed.");
        return 0;
    }
    if (!confirm_yn("Confirm close account (Y/N)")) {
        return 0;
    }
    account.isActive = 0;
    if (!rewrite_account_at(offset, &account)) {
        return 0;
    }
    ui_print_success("Account closed successfully.");
    return 1;
}

int unlock_account_interactive(void) {
    Account account;
    long offset;

    if (!search_account_interactive(&account, &offset)) {
        return 0;
    }
    account.isLocked = 0;
    account.failedAttempts = 0;
    if (!rewrite_account_at(offset, &account)) {
        return 0;
    }
    ui_print_success("Account unlocked.");
    return 1;
}
