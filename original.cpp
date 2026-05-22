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

