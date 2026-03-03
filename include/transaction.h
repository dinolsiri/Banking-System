#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "models.h"

int deposit_to_account(Account *account, long offset);
int withdraw_from_account(Account *account, long offset);
int transfer_between_accounts(Account *account, long offset);
void show_account_transactions(int accountNo, int limit);
void list_all_transactions(void);
int export_statement_for_account(int accountNo);

#endif
