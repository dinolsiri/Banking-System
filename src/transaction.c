#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "account.h"
#include "transaction.h"
#include "ui.h"
#include "utils.h"

static long next_txn_id(void) {
    FILE *fp = fopen(TRANSACTIONS_FILE, "rb");
    Transaction txn;
    long maxId = 0;

    if (fp == NULL) {
        return 1;
    }
    while (fread(&txn, sizeof(Transaction), 1U, fp) == 1U) {
        if (txn.txnId > maxId) {
            maxId = txn.txnId;
        }
    }
    fclose(fp);
    return maxId + 1;
}

static int append_transaction(int fromAccount, int toAccount, double amount,
                              const char *type, const char *note) {
    FILE *fp = fopen(TRANSACTIONS_FILE, "ab");
    Transaction txn;

    if (fp == NULL) {
        ui_print_error("Unable to open transactions file.");
        return 0;
    }

    memset(&txn, 0, sizeof(txn));
    txn.txnId = next_txn_id();
    txn.fromAccount = fromAccount;
    txn.toAccount = toAccount;
    txn.amount = amount;
    strncpy(txn.type, type, sizeof(txn.type) - 1U);
    strncpy(txn.note, note, sizeof(txn.note) - 1U);
    txn.timestamp = time(NULL);

    if (fwrite(&txn, sizeof(Transaction), 1U, fp) != 1U) {
        fclose(fp);
        ui_print_error("Failed to save transaction.");
        return 0;
    }

    fclose(fp);
    return 1;
}

static void print_txn_row(const Transaction *txn) {
    char when[32];
    format_time_str(txn->timestamp, when, sizeof(when));
    printf("%-8ld %-10d %-10d %-12.2f %-12s %-19s %s\n",
           txn->txnId,
           txn->fromAccount,
           txn->toAccount,
           txn->amount,
           txn->type,
           when,
           txn->note);
}

int deposit_to_account(Account *account, long offset) {
    double amount;
    char note[TXN_NOTE_LEN];

    ui_header("Deposit Funds");
    amount = read_double_min("Deposit amount", 0.01);
    read_string("Note", note, sizeof(note));

    account->balance += amount;
    if (!rewrite_account_at(offset, account)) {
        return 0;
    }
    if (!append_transaction(0, account->accountNo, amount, "DEPOSIT", note)) {
        return 0;
    }
    ui_print_success("Deposit completed.");
    return 1;
}

int withdraw_from_account(Account *account, long offset) {
    double amount;
    char note[TXN_NOTE_LEN];

    ui_header("Withdraw Funds");
    amount = read_double_min("Withdraw amount", 0.01);
    if (amount > account->balance) {
        ui_print_error("Insufficient balance.");
        return 0;
    }
    read_string("Note", note, sizeof(note));

    account->balance -= amount;
    if (!rewrite_account_at(offset, account)) {
        return 0;
    }
    if (!append_transaction(account->accountNo, 0, amount, "WITHDRAW", note)) {
        return 0;
    }
    ui_print_success("Withdrawal completed.");
    return 1;
}

int transfer_between_accounts(Account *account, long offset) {
    int targetNo;
    Account target;
    long targetOffset;
    double amount;
    char note[TXN_NOTE_LEN];

    ui_header("Transfer Funds");
    targetNo = read_int_range("Target account number", 1, 999999999);
    if (targetNo == account->accountNo) {
        ui_print_error("Cannot transfer to the same account.");
        return 0;
    }
    if (!find_account_by_no(targetNo, &target, &targetOffset)) {
        ui_print_error("Target account not found.");
        return 0;
    }
    if (!target.isActive || target.isLocked) {
        ui_print_error("Target account is unavailable.");
        return 0;
    }

    amount = read_double_min("Transfer amount", 0.01);
    if (amount > account->balance) {
        ui_print_error("Insufficient balance.");
        return 0;
    }
    read_string("Note", note, sizeof(note));

    account->balance -= amount;
    target.balance += amount;

    if (!rewrite_account_at(offset, account)) {
        return 0;
    }
    if (!rewrite_account_at(targetOffset, &target)) {
        return 0;
    }
    if (!append_transaction(account->accountNo, target.accountNo, amount, "TRANSFER", note)) {
        return 0;
    }
    ui_print_success("Transfer completed.");
    return 1;
}

void show_account_transactions(int accountNo, int limit) {
    FILE *fp = fopen(TRANSACTIONS_FILE, "rb");
    Transaction *items;
    Transaction txn;
    int count = 0;
    int i;
    int shown;

    ui_header("Transaction History");
    if (fp == NULL) {
        ui_print_error("Unable to open transactions file.");
        return;
    }

    items = (Transaction *)calloc((size_t)limit, sizeof(Transaction));
    if (items == NULL) {
        fclose(fp);
        ui_print_error("Memory allocation failed.");
        return;
    }

    while (fread(&txn, sizeof(Transaction), 1U, fp) == 1U) {
        if (txn.fromAccount == accountNo || txn.toAccount == accountNo) {
            items[count % limit] = txn;
            count++;
        }
    }
    fclose(fp);

    printf("%-8s %-10s %-10s %-12s %-12s %-19s %s\n",
           "TxnID", "From", "To", "Amount", "Type", "Timestamp", "Note");
    shown = count < limit ? count : limit;
    for (i = 0; i < shown; ++i) {
        int idx = (count > limit) ? ((count - shown + i) % limit) : i;
        print_txn_row(&items[idx]);
    }
    if (shown == 0) {
        ui_box("No transactions for this account.");
    }
    free(items);
}

void list_all_transactions(void) {
    FILE *fp = fopen(TRANSACTIONS_FILE, "rb");
    Transaction txn;
    int count = 0;

    ui_header("All Transactions");
    if (fp == NULL) {
        ui_print_error("Unable to open transactions file.");
        return;
    }

    printf("%-8s %-10s %-10s %-12s %-12s %-19s %s\n",
           "TxnID", "From", "To", "Amount", "Type", "Timestamp", "Note");
    while (fread(&txn, sizeof(Transaction), 1U, fp) == 1U) {
        print_txn_row(&txn);
        count++;
    }
    if (count == 0) {
        ui_box("No transactions available.");
    }
    fclose(fp);
}

int export_statement_for_account(int accountNo) {
    FILE *fp = fopen(TRANSACTIONS_FILE, "rb");
    FILE *out;
    Transaction txn;
    Account account;
    char path[256];
    char when[32];
    double incoming = 0.0;
    double outgoing = 0.0;
    int rows = 0;

    if (!find_account_by_no(accountNo, &account, NULL)) {
        ui_print_error("Account not found for statement export.");
        return 0;
    }
    if (fp == NULL) {
        ui_print_error("Unable to open transactions file.");
        return 0;
    }

    snprintf(path, sizeof(path), "%s/statement_%d.txt", STATEMENTS_DIR, accountNo);
    out = fopen(path, "w");
    if (out == NULL) {
        fclose(fp);
        ui_print_error("Unable to create statement file.");
        return 0;
    }

    format_time_str(time(NULL), when, sizeof(when));
    fprintf(out, "NEON BANK ACCOUNT STATEMENT\n");
    fprintf(out, "Generated: %s\n", when);
    fprintf(out, "Account No: %d\n", account.accountNo);
    fprintf(out, "Name: %s\n", account.fullName);
    fprintf(out, "Username: %s\n", account.username);
    fprintf(out, "Current Balance: %.2f\n", account.balance);
    fprintf(out, "-------------------------------------------------------------------------------\n");
    fprintf(out, "%-8s %-10s %-10s %-12s %-12s %-19s %s\n",
            "TxnID", "From", "To", "Amount", "Type", "Timestamp", "Note");

    while (fread(&txn, sizeof(Transaction), 1U, fp) == 1U) {
        if (txn.fromAccount == accountNo || txn.toAccount == accountNo) {
            char ts[32];
            format_time_str(txn.timestamp, ts, sizeof(ts));
            fprintf(out, "%-8ld %-10d %-10d %-12.2f %-12s %-19s %s\n",
                    txn.txnId, txn.fromAccount, txn.toAccount, txn.amount, txn.type, ts, txn.note);
            if (txn.toAccount == accountNo) {
                incoming += txn.amount;
            }
            if (txn.fromAccount == accountNo) {
                outgoing += txn.amount;
            }
            rows++;
        }
    }

    fprintf(out, "-------------------------------------------------------------------------------\n");
    fprintf(out, "Incoming Total: %.2f\n", incoming);
    fprintf(out, "Outgoing Total: %.2f\n", outgoing);
    fprintf(out, "Net Flow: %.2f\n", incoming - outgoing);
    fprintf(out, "Rows Exported: %d\n", rows);

    fclose(fp);
    fclose(out);
    ui_print_success("Statement exported.");
    return 1;
}
