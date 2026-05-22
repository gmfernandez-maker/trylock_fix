# Deadlock Fixes — Try-Lock Example (Group5)

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
Thread B: trying to lock accounts
Acquired both account locks (thread 4)
Audit: locking log
Audit Report: A=1000, B=2000
Released both account locks (thread 4)
Acquired both account locks (thread 2)
Thread A: transferred money
Released both account locks (thread 2)
Acquired both account locks (thread 3)
Thread B: transferred money
Released both account locks (thread 3)
Final Balances: A=950, B=2050
```

- `Thread B: trying to lock accounts` means one transfer started first and entered the try-lock loop.
- `Acquired both account locks (thread 4)` means that thread successfully got both account mutexes at the same time.
- `Audit: locking log` means the audit thread moved on to the log mutex after the account locks were available.
- `Audit Report: A=1000, B=2000` means the audit ran before either transfer changed the balances, so it read the starting values.
- `Released both account locks (thread 4)` means that thread finished its critical section and unlocked both accounts.
- `Acquired both account locks (thread 2)` and `Thread A: transferred money` mean the A transfer ran next and updated the balances safely.
- `Acquired both account locks (thread 3)` and `Thread B: transferred money` mean the B transfer ran after that and also completed safely.
- `Final Balances: A=950, B=2050` confirms both transfers were applied and the program ended without deadlock.

