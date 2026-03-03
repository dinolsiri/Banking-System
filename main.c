#include <stdio.h>

#include "account.h"
#include "auth.h"
#include "models.h"
#include "transaction.h"
#include "ui.h"
#include "utils.h"

static void user_session(Account *account, long *offset) {
    int choice = 0;

    while (choice != 8) {
        ui_header("User Console");
        printf("Welcome, %s | Account: %d | Balance: %.2f\n\n",
               account->fullName, account->accountNo, account->balance);
        printf("1. View Balance\n");
        printf("2. Deposit\n");
        printf("3. Withdraw\n");
        printf("4. Transfer\n");
        printf("5. Transaction History (last N)\n");
        printf("6. Export Statement\n");
        printf("7. Change PIN\n");
        printf("8. Logout\n");
        choice = read_int_range("Select option", 1, 8);

        switch (choice) {
            case 1:
                ui_header("Balance Snapshot");
                print_account_summary(account);
                break;
            case 2:
                deposit_to_account(account, *offset);
                break;
            case 3:
                withdraw_from_account(account, *offset);
                break;
            case 4:
                transfer_between_accounts(account, *offset);
                break;
            case 5: {
                int limit = read_int_range("How many recent transactions", 1, 100);
                show_account_transactions(account->accountNo, limit);
                break;
            }
            case 6:
                export_statement_for_account(account->accountNo);
                break;
            case 7:
                change_user_pin(account, *offset);
                break;
            case 8:
                ui_print_success("User logged out.");
                break;
            default:
                break;
        }
        if (choice != 8) {
            ui_pause();
        }
    }
}

static void admin_session(void) {
    int choice = 0;

    while (choice != 9) {
        ui_header("Admin Console");
        printf("1. Create Account\n");
        printf("2. View All Accounts\n");
        printf("3. Search Account\n");
        printf("4. Update Account\n");
        printf("5. Close Account\n");
        printf("6. Unlock Account\n");
        printf("7. View All Transactions\n");
        printf("8. Export Any Account Statement\n");
        printf("9. Logout\n");
        choice = read_int_range("Select option", 1, 9);

        switch (choice) {
            case 1:
                create_account();
                break;
            case 2:
                list_all_accounts();
                break;
            case 3:
                search_account_interactive(NULL, NULL);
                break;
            case 4:
                update_account_interactive();
                break;
            case 5:
                close_account_interactive();
                break;
            case 6:
                unlock_account_interactive();
                break;
            case 7:
                list_all_transactions();
                break;
            case 8: {
                int accountNo = read_int_range("Account number", 1, 999999999);
                ui_header("Export Statement");
                export_statement_for_account(accountNo);
                break;
            }
            case 9:
                ui_print_success("Admin logged out.");
                break;
            default:
                break;
        }
        if (choice != 9) {
            ui_pause();
        }
    }
}

int main(void) {
    int choice = 0;

    ensure_storage();
    ui_loading("Initializing secure banking grid");

    while (choice != 3) {
        Account account;
        long offset = 0L;

        ui_header("Main Gateway");
        ui_box("Cyber Banking System");
        printf("1. Admin Login\n");
        printf("2. User Login\n");
        printf("3. Exit\n");
        choice = read_int_range("Select option", 1, 3);

        switch (choice) {
            case 1:
                if (admin_login()) {
                    ui_pause();
                    admin_session();
                } else {
                    ui_pause();
                }
                break;
            case 2:
                if (user_login(&account, &offset)) {
                    ui_pause();
                    user_session(&account, &offset);
                } else {
                    ui_pause();
                }
                break;
            case 3:
                ui_header("Shutdown");
                ui_print_success("Bank system terminated.");
                break;
            default:
                break;
        }
    }

    return 0;
}
