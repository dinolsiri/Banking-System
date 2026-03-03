#ifndef AUTH_H
#define AUTH_H

#include "models.h"

int admin_login(void);
int user_login(Account *outAccount, long *outOffset);
int change_user_pin(Account *account, long offset);

#endif
