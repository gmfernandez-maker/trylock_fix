# Deadlock Fixes — Try-Lock Example (Group5)

Overview
--------
This project demonstrates several strategies to resolve a deadlock in a simple bank transfer example. Two files show the try-lock approach:

- `trylock_gr5.cpp` — presentation-friendly try-lock variant with diagnostic output (acquire/release messages and final balances).

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

```cpp
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
	logLock.lock();
	cout << msg << endl;
	this_thread::sleep_for(chrono::milliseconds(50)); // simulate delay
	logLock.unlock();
}

void transferAtoB(int amount) {
	cout << "Thread A: locking account1" << endl;
	accountLock1.lock();

	this_thread::sleep_for(chrono::milliseconds(100));

	cout << "Thread A: locking account2" << endl;
	accountLock2.lock();

	account1 -= amount;
	account2 += amount;

	logTransaction("Thread A: transferred money");

	accountLock2.unlock();
	accountLock1.unlock();
}

void transferBtoA(int amount) {
	cout << "Thread B: locking account2" << endl;
	accountLock2.lock();

	this_thread::sleep_for(chrono::milliseconds(100));

	cout << "Thread B: locking account1" << endl;
	accountLock1.lock();

	account2 -= amount;
	account1 += amount;

	logTransaction("Thread B: transferred money");

	accountLock1.unlock();
	accountLock2.unlock();
}

void auditAccounts() {
	cout << "Audit: locking log first" << endl;
	logLock.lock();

	this_thread::sleep_for(chrono::milliseconds(50));

	cout << "Audit: locking account1" << endl;
	accountLock1.lock();

	cout << "Audit: locking account2" << endl;
	accountLock2.lock();

	cout << "Audit Report: "
	     << "A=" << account1
	     << ", B=" << account2 << endl;

	accountLock2.unlock();
	accountLock1.unlock();
	logLock.unlock();
}

int main() {
	thread t1(transferAtoB, 100);
	thread t2(transferBtoA, 50);
	thread t3(auditAccounts);

	t1.join();
	t2.join();
	t3.join();

	return 0;
}
```

- `transferAtoB()` locked `accountLock1` then `accountLock2`, then called `logTransaction()` which locks `logLock`.
- `transferBtoA()` locked `accountLock2` then `accountLock1` (opposite order).
- `auditAccounts()` locked `logLock` first then tried to lock the account locks.

This created a circular wait: a transfer could hold account locks and wait for `logLock`, while audit held `logLock` and waited for account locks.

What the try-lock fix does:

- The helper `lockAccountsWithTryLock()` only returns once a thread holds both account locks simultaneously; if it cannot get both it releases any partial locks and retries. That removes the possibility of two transfer threads each holding one account lock while waiting for the other.
- Equally important: `auditAccounts()` was reordered to acquire the account locks (using the same try-lock helper) before acquiring `logLock`. This prevents the audit thread from holding `logLock` while waiting for account locks — breaking the circular wait involving `logLock`.

Result: no thread holds one resource while waiting for another; circular wait is removed and the program completes.


About `trylock_gr5.cpp`
-----------------------
`trylock_gr5.cpp` is the version meant for presenting and explaining the fix in class. It keeps the same try-lock logic, but adds small status messages so you can see when a thread:

- starts trying to lock the accounts,
- successfully acquires both account locks,
- releases both account locks,
- and prints the final balances after all threads finish.

That extra output makes it easier to show that the program no longer freezes and that each thread completes its work in a safe order.

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

