#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "models.h"

int create_account(void);
void list_all_accounts(void);
int search_account_interactive(Account *outAccount, long *outOffset);
int update_account_interactive(void);
int close_account_interactive(void);
int unlock_account_interactive(void);
void print_account_summary(const Account *account);
int find_account_by_no(int accountNo, Account *outAccount, long *outOffset);
int find_account_by_username(const char *username, Account *outAccount, long *outOffset);
int find_account_by_nic(const char *nic, Account *outAccount, long *outOffset);
int rewrite_account_at(long offset, const Account *account);
int next_account_number(void);

#endif
