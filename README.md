# 💳 NEON BANK CORE
Advanced Modular Banking System in C

A futuristic terminal-based banking management system built using C (C11) with modular architecture, binary file storage, authentication system, and transaction tracking.

## 🚀 Features
### 🔐 Authentication

- Admin login
- User login with PIN
- Account lock after 3 failed attempts
- Change PIN functionality

### 👤 Account Management (Admin)

- Create new accounts
- View all accounts
- Search by account number / username / NIC
- Update account details
- Close (soft delete) account
- Unlock locked accounts

### 💰 User Operations

- View balance
- Deposit funds
- Withdraw funds
- Transfer between accounts
- View transaction history
- Export account statement (.txt)

### 📁 Data Storage

Persistent storage using binary files:

- `data/accounts.dat`
- `data/transactions.dat`

Auto-creates required folders and files

Update-in-place record editing

### 🎨 UI

- Futuristic neon terminal interface
- ANSI color styling
- Structured menus and headers
- Loading animations

## 🛠 Project Structure

```text
include/
 ├── account.h
 ├── auth.h
 ├── models.h
 ├── transaction.h
 ├── ui.h
 └── utils.h

src/
 ├── account.c
 ├── auth.c
 ├── transaction.c
 ├── ui.c
 └── utils.c

main.c
```

## ⚙️ Requirements

- Windows OS
- MSYS2 with MinGW GCC
- C11 compatible compiler

## 🧰 Installation (Windows + MSYS2)

Install MSYS2

Open MSYS2 UCRT64 terminal

Install GCC:

```bash
pacman -Syu
pacman -S --needed mingw-w64-ucrt-x86_64-gcc
```

Add to PATH:

`C:\msys64\ucrt64\bin`

Restart VS Code.

## 🏗 Build

From project root directory:

```bash
gcc -std=c11 -Wall -Wextra -O2 main.c src/ui.c src/utils.c src/auth.c src/account.c src/transaction.c -Iinclude -o bank.exe
```

## ▶️ Run

```powershell
.\bank.exe
```

## 🔑 Default Admin Credentials

Username: `root.admin`  
Password: `admin123`

(Password may differ if modified in `auth.c`.)

## 📄 Statement Export

Exported account statements are saved in:

`statements/statement_<accountNumber>.txt`

## 🧠 Technical Highlights

- Modular architecture (separation of concerns)
- Struct-based data modeling
- File handling with `fseek()` for in-place updates
- Custom salted multi-round hash function for PIN security
- Input validation and error handling
- Circular buffer for transaction history display

## 📚 Learning Outcomes

This project demonstrates:

- Advanced C programming
- File I/O in binary mode
- Authentication logic
- Data persistence
- Structured project organization
- Terminal UI design

## 👨‍💻 Author
Dinol siriwardena
https://github.com/dinolsiri
Developed as an advanced C systems programming project.