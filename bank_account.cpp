// ============================================================
//  Bank Account System — C++
//  Concepts: OOP, Inheritance, Polymorphism, Classes, Vectors
// ============================================================

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <sstream>

// ── Transaction: records a single deposit or withdrawal ──────
struct Transaction {
    std::string type;    // "Deposit" or "Withdrawal"
    double      amount;
    std::string note;

    // Get current timestamp as a string
    std::string getTimestamp() const {
        time_t now = time(0);
        std::string ts = ctime(&now);
        ts.pop_back(); // remove trailing newline
        return ts;
    }

    void print() const {
        std::cout << "  [" << type << "] $"
                  << std::fixed << std::setprecision(2) << amount
                  << "  — " << note << "\n";
    }
};

// ── Base class: BankAccount ──────────────────────────────────
class BankAccount {
protected:
    std::string accountHolder;
    std::string accountNumber;
    double      balance;
    std::vector<Transaction> history;

public:
    BankAccount(std::string holder, std::string accNum, double initialBalance)
        : accountHolder(holder), accountNumber(accNum), balance(initialBalance) {}

    // Virtual destructor — good practice for base classes
    virtual ~BankAccount() {}

    // Deposit funds into account
    bool deposit(double amount, const std::string& note = "Deposit") {
        if (amount <= 0) {
            std::cout << "  Error: Deposit amount must be positive.\n";
            return false;
        }
        balance += amount;
        history.push_back({"Deposit", amount, note});
        std::cout << "  ✓ Deposited $" << std::fixed << std::setprecision(2) << amount << "\n";
        return true;
    }

    // Withdraw funds — overridden by subclasses for different rules
    virtual bool withdraw(double amount, const std::string& note = "Withdrawal") {
        if (amount <= 0) {
            std::cout << "  Error: Withdrawal amount must be positive.\n";
            return false;
        }
        if (amount > balance) {
            std::cout << "  Error: Insufficient funds. Balance: $"
                      << std::fixed << std::setprecision(2) << balance << "\n";
            return false;
        }
        balance -= amount;
        history.push_back({"Withdrawal", amount, note});
        std::cout << "  ✓ Withdrew $" << std::fixed << std::setprecision(2) << amount << "\n";
        return true;
    }

    // Print transaction history
    void printHistory() const {
        if (history.empty()) {
            std::cout << "  No transactions yet.\n";
            return;
        }
        std::cout << "\n  --- Transaction History ---\n";
        for (const auto& t : history) t.print();
    }

    // Virtual summary — subclasses can override to add extra info
    virtual void printSummary() const {
        std::cout << "\n  Account Holder : " << accountHolder << "\n";
        std::cout << "  Account Number : " << accountNumber << "\n";
        std::cout << "  Balance        : $" << std::fixed << std::setprecision(2) << balance << "\n";
    }

    double      getBalance()  const { return balance; }
    std::string getHolder()   const { return accountHolder; }
    std::string getAccNum()   const { return accountNumber; }
};

// ── Derived class: SavingsAccount ───────────────────────────
//    Extra feature: interest rate, limited withdrawals per month
class SavingsAccount : public BankAccount {
private:
    double interestRate;         // annual rate, e.g. 0.035 = 3.5%
    int    withdrawalsThisMonth;
    int    withdrawalLimit;      // max per month

public:
    SavingsAccount(std::string holder, std::string accNum,
                   double initialBalance, double rate = 0.035)
        : BankAccount(holder, accNum, initialBalance),
          interestRate(rate), withdrawalsThisMonth(0), withdrawalLimit(3) {}

    // Override withdraw to enforce monthly limit
    bool withdraw(double amount, const std::string& note = "Withdrawal") override {
        if (withdrawalsThisMonth >= withdrawalLimit) {
            std::cout << "  Error: Monthly withdrawal limit (" << withdrawalLimit << ") reached.\n";
            return false;
        }
        bool success = BankAccount::withdraw(amount, note);
        if (success) withdrawalsThisMonth++;
        return success;
    }

    // Apply monthly interest to balance
    void applyInterest() {
        double monthly = balance * (interestRate / 12.0);
        balance += monthly;
        history.push_back({"Deposit", monthly, "Monthly Interest Applied"});
        std::cout << "  ✓ Interest applied: +$" << std::fixed << std::setprecision(2) << monthly << "\n";
        withdrawalsThisMonth = 0; // reset monthly counter
    }

    void printSummary() const override {
        BankAccount::printSummary();
        std::cout << "  Account Type   : Savings\n";
        std::cout << "  Interest Rate  : " << (interestRate * 100) << "%\n";
        std::cout << "  Withdrawals    : " << withdrawalsThisMonth
                  << " / " << withdrawalLimit << " this month\n";
    }
};

// ── Derived class: CheckingAccount ──────────────────────────
//    Extra feature: overdraft protection up to a set limit
class CheckingAccount : public BankAccount {
private:
    double overdraftLimit; // how far below $0 the account can go

public:
    CheckingAccount(std::string holder, std::string accNum,
                    double initialBalance, double overdraft = 200.0)
        : BankAccount(holder, accNum, initialBalance), overdraftLimit(overdraft) {}

    // Override withdraw to allow overdraft
    bool withdraw(double amount, const std::string& note = "Withdrawal") override {
        if (amount <= 0) {
            std::cout << "  Error: Withdrawal must be positive.\n";
            return false;
        }
        if (amount > balance + overdraftLimit) {
            std::cout << "  Error: Exceeds overdraft limit. Max available: $"
                      << std::fixed << std::setprecision(2) << (balance + overdraftLimit) << "\n";
            return false;
        }
        balance -= amount;
        history.push_back({"Withdrawal", amount, note});
        std::cout << "  ✓ Withdrew $" << std::fixed << std::setprecision(2) << amount;
        if (balance < 0)
            std::cout << "  ⚠ Balance is negative (overdraft used)\n";
        else
            std::cout << "\n";
        return true;
    }

    void printSummary() const override {
        BankAccount::printSummary();
        std::cout << "  Account Type   : Checking\n";
        std::cout << "  Overdraft Limit: $" << std::fixed << std::setprecision(2) << overdraftLimit << "\n";
    }
};

// ── Transfer between two accounts ───────────────────────────
void transfer(BankAccount& from, BankAccount& to, double amount) {
    std::cout << "\n  Transferring $" << std::fixed << std::setprecision(2) << amount << "...\n";
    if (from.withdraw(amount, "Transfer Out to " + to.getAccNum()))
        to.deposit(amount, "Transfer In from " + from.getAccNum());
}

// ── Main menu ────────────────────────────────────────────────
int main() {
    std::cout << "====================================\n";
    std::cout << "   Bank Account System — C++\n";
    std::cout << "====================================\n";

    std::string name;
    std::cout << "Enter account holder name: ";
    std::getline(std::cin, name);

    // Create one checking and one savings account for the user
    CheckingAccount checking(name, "CHK-001", 1000.0);
    SavingsAccount  savings (name, "SAV-001",  500.0, 0.04); // 4% interest

    int choice = 0;
    while (choice != 7) {
        std::cout << "\n--- Menu ---\n";
        std::cout << "1. Deposit (Checking)\n";
        std::cout << "2. Withdraw (Checking)\n";
        std::cout << "3. Deposit (Savings)\n";
        std::cout << "4. Withdraw (Savings)\n";
        std::cout << "5. Transfer Checking → Savings\n";
        std::cout << "6. View summary & history\n";
        std::cout << "7. Exit\n";
        std::cout << "Choice: ";
        std::cin >> choice;
        std::cin.ignore(1000, '\n');

        double amount;

        if (choice == 1) {
            std::cout << "  Amount to deposit: $";
            std::cin >> amount; std::cin.ignore(1000, '\n');
            checking.deposit(amount);

        } else if (choice == 2) {
            std::cout << "  Amount to withdraw: $";
            std::cin >> amount; std::cin.ignore(1000, '\n');
            checking.withdraw(amount);

        } else if (choice == 3) {
            std::cout << "  Amount to deposit: $";
            std::cin >> amount; std::cin.ignore(1000, '\n');
            savings.deposit(amount);

        } else if (choice == 4) {
            std::cout << "  Amount to withdraw: $";
            std::cin >> amount; std::cin.ignore(1000, '\n');
            savings.withdraw(amount);

        } else if (choice == 5) {
            std::cout << "  Transfer amount: $";
            std::cin >> amount; std::cin.ignore(1000, '\n');
            transfer(checking, savings, amount);

        } else if (choice == 6) {
            std::cout << "\n======= CHECKING ACCOUNT =======";
            checking.printSummary();
            checking.printHistory();
            std::cout << "\n======= SAVINGS ACCOUNT ========";
            savings.printSummary();
            savings.printHistory();

        } else if (choice != 7) {
            std::cout << "  Invalid option.\n";
        }
    }

    std::cout << "\nThank you for banking with us, " << name << "!\n";
    return 0;
}
