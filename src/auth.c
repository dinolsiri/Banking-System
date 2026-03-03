#include <stdio.h>
#include <string.h>

#include "account.h"
#include "auth.h"
#include "ui.h"
#include "utils.h"

static const char ADMIN_USERNAME[] = "root.admin";
static const char ADMIN_PASS_HASH[65] = "82f7dc6ec82c6ef6c7b12530078d10f93db29f6abe6ad70ce5fc8cc3deee657f";

static int update_login_state(const Account *account, long offset) {
    return rewrite_account_at(offset, account);
}

int admin_login(void) {
    char username[USERNAME_LEN];
    char password[64];
    char hash[65];

    ui_header("Admin Login");
    read_string("Admin username", username, sizeof(username));
    read_string("Admin password", password, sizeof(password));
    hash_pin(password, hash);

    if (strcmp(username, ADMIN_USERNAME) == 0 && strcmp(hash, ADMIN_PASS_HASH) == 0) {
        ui_print_success("Admin authentication successful.");
        return 1;
    }

    ui_print_error("Invalid admin credentials.");
    return 0;
}

int user_login(Account *outAccount, long *outOffset) {
    Account account;
    long offset;
    char username[USERNAME_LEN];
    char pin[16];
    char hash[65];

    ui_header("User Login");
    read_string("Username", username, sizeof(username));
    read_string("PIN", pin, sizeof(pin));

    if (!find_account_by_username(username, &account, &offset)) {
        ui_print_error("User not found.");
        return 0;
    }
    if (!account.isActive) {
        ui_print_error("Account is closed.");
        return 0;
    }
    if (account.isLocked) {
        ui_print_error("Account is locked. Contact admin.");
        return 0;
    }
    if (!validate_pin_format(pin)) {
        ui_print_error("Invalid PIN format.");
        return 0;
    }

    hash_pin(pin, hash);
    if (strcmp(hash, account.pinHash) != 0) {
        account.failedAttempts += 1;
        if (account.failedAttempts >= 3) {
            account.isLocked = 1;
            ui_print_error("Account locked after 3 failed attempts.");
        } else {
            ui_print_error("Incorrect PIN.");
        }
        update_login_state(&account, offset);
        return 0;
    }

    account.failedAttempts = 0;
    account.isLocked = 0;
    if (!update_login_state(&account, offset)) {
        return 0;
    }
    if (outAccount != NULL) {
        *outAccount = account;
    }
    if (outOffset != NULL) {
        *outOffset = offset;
    }
    ui_print_success("Login successful.");
    return 1;
}

int change_user_pin(Account *account, long offset) {
    char currentPin[16];
    char newPin[16];
    char confirmPin[16];
    char currentHash[65];

    ui_header("Change PIN");
    read_string("Current PIN", currentPin, sizeof(currentPin));
    hash_pin(currentPin, currentHash);
    if (strcmp(currentHash, account->pinHash) != 0) {
        ui_print_error("Current PIN is incorrect.");
        return 0;
    }

    read_string("New PIN", newPin, sizeof(newPin));
    if (!validate_pin_format(newPin)) {
        ui_print_error("New PIN must be 4-6 digits.");
        return 0;
    }
    read_string("Confirm new PIN", confirmPin, sizeof(confirmPin));
    if (strcmp(newPin, confirmPin) != 0) {
        ui_print_error("PIN confirmation mismatch.");
        return 0;
    }

    hash_pin(newPin, account->pinHash);
    if (!rewrite_account_at(offset, account)) {
        return 0;
    }
    ui_print_success("PIN changed successfully.");
    return 1;
}
