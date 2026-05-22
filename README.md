# Deadlock Fixes — Try-Lock Example (Group5)

## Overview
This README focuses only on the original deadlocking program in `original.cpp` and the fixed version in `trylock_gr5.cpp`.

## What `try_lock` does
`try_lock()` is a non-blocking mutex call. It tries to acquire a lock and immediately returns `true` or `false`.

The pattern used here is simple:

1. Try to lock `accountLock1`.
2. If that works, try to lock `accountLock2`.
3. If the second lock fails, release the first lock and retry.

This avoids the hold-and-wait condition that causes deadlock.

## Why `original.cpp` deadlocked
The original code used conflicting lock orders:

```cpp
void transferAtoB(int amount) {
    accountLock1.lock();
    accountLock2.lock();
    logTransaction("Thread A: transferred money");
}

void transferBtoA(int amount) {
    accountLock2.lock();
    accountLock1.lock();
    logTransaction("Thread B: transferred money");
}

void auditAccounts() {
    logLock.lock();
    accountLock1.lock();
    accountLock2.lock();
}
```

- `transferAtoB()` locked the accounts in one order.
- `transferBtoA()` locked the same accounts in the opposite order.
- `auditAccounts()` locked `logLock` first, then tried to lock the accounts.

That created a circular wait.

## How `trylock_gr5.cpp` fixes it
`trylock_gr5.cpp` uses a helper that only continues when both account locks are available:

```cpp
void lockAccountsWithTryLock() {
    while (true) {
        if (accountLock1.try_lock()) {
            if (accountLock2.try_lock()) return;
            accountLock1.unlock();
        }
        this_thread::yield();
    }
}
```

The audit path also follows the safe order by locking the accounts first and `logLock` second. That removes the circular wait, so the program completes instead of freezing.

## `trylock_gr5.cpp` Explanation

- starts trying to lock the accounts,
- acquires both locks,
- releases both locks,
- and prints the final balances.

## Example output
```text
Thread A: trying to lock accounts
Thread B: trying to lock accounts
Acquired both account locks (thread ...)
Thread B: transferred money
Released both account locks (thread ...)
Final Balances: A=950, B=2050
```

