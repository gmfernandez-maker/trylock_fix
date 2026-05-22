#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>

using namespace std;

mutex accountLock1;
mutex accountLock2;
mutex logLock;

int account1 = 1000;
int account2 = 2000;

void logTransaction(const string& msg) {
    lock_guard<mutex> g(logLock);
    cout << msg << endl;
    this_thread::sleep_for(chrono::milliseconds(50)); 
}


void lockAccountsWithTryLock() {
    while (true) {
        if (accountLock1.try_lock()) {
            if (accountLock2.try_lock()) {
                cout << "Acquired both account locks (thread " << this_thread::get_id() << ")" << endl;
                return; 
            }
            accountLock1.unlock();
        }
        this_thread::yield();
    }
}

void unlockAccounts() {
    accountLock2.unlock();
    accountLock1.unlock();
    cout << "Released both account locks (thread " << this_thread::get_id() << ")" << endl;
}

void transferAtoB(int amount) {
    cout << "Thread A: trying to lock accounts" << endl;
    lockAccountsWithTryLock();

    this_thread::sleep_for(chrono::milliseconds(100));

    account1 -= amount;
    account2 += amount;

    logTransaction("Thread A: transferred money");

    unlockAccounts();
}

void transferBtoA(int amount) {
    cout << "Thread B: trying to lock accounts" << endl;
    lockAccountsWithTryLock();

    this_thread::sleep_for(chrono::milliseconds(100));

    account2 -= amount;
    account1 += amount;

    logTransaction("Thread B: transferred money");

    unlockAccounts();
}

void auditAccounts() {
    cout << "Audit: trying to lock accounts" << endl;
    lockAccountsWithTryLock();

    cout << "Audit: locking log" << endl;
    lock_guard<mutex> g(logLock);

    cout << "Audit Report: "
         << "A=" << account1
         << ", B=" << account2 << endl;

    unlockAccounts();
}

int main() {
    thread t1(transferAtoB, 100);
    thread t2(transferBtoA, 50);
    thread t3(auditAccounts);

    t1.join();
    t2.join();
    t3.join();

    cout << "Final Balances: A=" << account1 << ", B=" << account2 << endl;
    return 0;
}
