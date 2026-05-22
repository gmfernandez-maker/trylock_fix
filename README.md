# Deadlock Fixes — Try-Lock Example (Group5)

Overview
--------
This project demonstrates several strategies to resolve a deadlock in a simple bank transfer example. Two files show the try-lock approach:

- `trylock_gr5.cpp` — presentation-friendly try-lock variant with diagnostic output (acquire/release messages and final balances).
- `original_trylock_fixed.cpp` — minimal patched copy of the original program where transfers and the audit use a try-lock helper and audit is reordered to acquire account locks before the log lock.

What is `try_lock`?
--------------------
`try_lock()` is a non-blocking mutex operation: it attempts to acquire a mutex and returns immediately with a boolean result indicating success or failure. The typical pattern used here is:

1. Repeatedly attempt to acquire a first mutex with `try_lock()`.
2. If successful, attempt to acquire the second mutex with `try_lock()`.
3. If the second attempt fails, release the first mutex and retry (optionally yield).

This avoids the "hold-and-wait" condition where a thread holds one lock while waiting indefinitely for another, which is one of the classic necessary conditions for deadlock.

How this fixed the original program
----------------------------------
Original problem summary:

- `transferAtoB()` locked `accountLock1` then `accountLock2`, then called `logTransaction()` which locks `logLock`.
- `transferBtoA()` locked `accountLock2` then `accountLock1` (opposite order).
- `auditAccounts()` locked `logLock` first then tried to lock the account locks.

This created a circular wait: a transfer could hold account locks and wait for `logLock`, while audit held `logLock` and waited for account locks.

What the try-lock fix does:

- The helper `lockAccountsWithTryLock()` only returns once a thread holds both account locks simultaneously; if it cannot get both it releases any partial locks and retries. That removes the possibility of two transfer threads each holding one account lock while waiting for the other.
- Equally important: `auditAccounts()` was reordered to acquire the account locks (using the same try-lock helper) before acquiring `logLock`. This prevents the audit thread from holding `logLock` while waiting for account locks — breaking the circular wait involving `logLock`.

Result: no thread holds one resource while waiting for another, circular wait is removed and the program completes.

Example output (trimmed)
------------------------
When running `trylock_gr5.exe` you should see lines similar to:

```
Thread A: trying to lock accounts
Thread B: trying to lock accounts
Acquired both account locks (thread 3)
Thread B: transferred money
Released both account locks (thread 3)
Acquired both account locks (thread 2)
Thread A: transferred money
Released both account locks (thread 2)
Final Balances: A=950, B=2050
```

- `recursive_mutex.cpp` — single `recursive_mutex` protecting all critical sections
- `try_lock.cpp`, `trylock_gr5.cpp`, `original_trylock_fixed.cpp` — variants using try-lock; `trylock_gr5.cpp` is the demo-friendly one
- `shared_mutex.cpp` — read/write lock (shared_mutex) strategy

If you want, I can also add a short slide or a one-page PDF comparing all five strategies and the trade-offs to include in your Canvas submission.
