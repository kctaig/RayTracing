#pragma once

#include <atomic>
#include <thread>

// зда§Ыј
class Spin_Lock {
public:
	void acquire() {
		while (flag.test_and_set(std::memory_order_acquire)) {
			std::this_thread::yield();
		}
	}
	void release() {
		flag.clear(std::memory_order_release);
	}

private:
	std::atomic_flag flag{};
};

class Guard {
public:
	Guard(Spin_Lock& spin_lock) :spin_lock(spin_lock) {
		spin_lock.acquire();
	}
	~Guard() { spin_lock.release(); }

private:
	Spin_Lock& spin_lock;
};