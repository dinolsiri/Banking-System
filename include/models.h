#ifndef MODELS_H
#define MODELS_H

#include <time.h>

#define DATA_DIR "data"
#define STATEMENTS_DIR "statements"
#define ACCOUNTS_FILE "data/accounts.dat"
#define TRANSACTIONS_FILE "data/transactions.dat"

#define NAME_LEN 60
#define NIC_LEN 20
#define PHONE_LEN 20
#define ADDRESS_LEN 120
#define USERNAME_LEN 30
#define PIN_HASH_LEN 65
#define TXN_TYPE_LEN 20
#define TXN_NOTE_LEN 80

typedef struct {
    int accountNo;
    char fullName[NAME_LEN];
    char nic[NIC_LEN];
    char phone[PHONE_LEN];
    char address[ADDRESS_LEN];
    char username[USERNAME_LEN];
    char pinHash[PIN_HASH_LEN];
    double balance;
    int isActive;
    int failedAttempts;
    int isLocked;
    time_t createdAt;
} Account;

typedef struct {
    long txnId;
    int fromAccount;
    int toAccount;
    double amount;
    char type[TXN_TYPE_LEN];
    time_t timestamp;
    char note[TXN_NOTE_LEN];
} Transaction;

#endif
