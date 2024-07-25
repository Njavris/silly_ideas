#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <string>
#include <fstream>

#include "multithr.h"

using namespace std;

class execTime {
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime, stopTime;
public:
	void start() { startTime = chrono::high_resolution_clock::now(); };
	void stop() { stopTime = chrono::high_resolution_clock::now(); };
	int durationMs() {
		return chrono::duration_cast<chrono::microseconds>(stopTime - startTime).count();
	}
	int durationNs() {
		return chrono::duration_cast<chrono::nanoseconds>(stopTime - startTime).count();
	}
};

uint64_t sum_alg(uint64_t num) {
	uint64_t res = num * num + num;
	res /= 2;
	return res;
}

uint64_t sum_iter(uint64_t num) {
	uint64_t res = 0;
	for (int i = 1; i <= num; i++) {
		res += i;
	}
	return res;
}

class Summer : public Worker {
public:
	Summer(int id) : Worker(id) {};
	void doWork() {
		result = 0;
		pthread_detach(pthread_self());
		for (uint64_t i = arg + 1; i <= arg + arg1; i++) {
			result += i;
		}
		tdebug("Summing [%ld:%ld) = %ld\n", arg, arg + arg1, result);
	};
	void doCallback() {
	};
};

uint64_t sum_parallel(uint64_t num, int children) {
	uint64_t res = 0;
	uint64_t range = num / children;

	std::vector<Worker *> workers;
	TaskManager m;
//	srand(time(0));

	tdebug("Parent: %lx\n", pthread_self());
	for (int i = 0; i < children; i++) {
		Worker *w;
		w = new Summer(i);
		w->arg = range * i;
		w->arg1 = range;
		workers.push_back(w);
		m.spawnWorker(w);
	}

	while (!m.allDone()) {
		m.checkWorkers();
	}
	tdebug("All children joined\n");
	for (auto &w: workers) {
		res += w->result;
		delete w;
	}
	return res;
}

string point(int target, int children) {
	execTime tmr;
	string ret = to_string(target) + ",";
	uint64_t res1 = 0, res2 = 0, res3 = 0;
	tmr.start();
	res1 = sum_iter(target);
	tmr.stop();
	printf("iter sum = %ld (%lx) %d ns\n", res1, res1, tmr.durationNs());
	ret += to_string(tmr.durationNs()) + ",";

	tmr.start();
	res2 = sum_parallel(target, children);
	tmr.stop();
	printf("paralel sum = %ld (%lx) %d ns\n", res2, res2, tmr.durationNs());
	ret += to_string(tmr.durationNs()) + ",";

	tmr.start();
	res3 = sum_alg(target);
	tmr.stop();
	printf("algo sum = %ld (%lx) %d ns\n", res3, res3, tmr.durationNs());
	ret += to_string(tmr.durationNs());

	if (res1 != res2 || res2 != res3)
		exit(-1);
	return ret;
}

void test() {
	ofstream file;
//	file.open("test.csv");
	int children = 4;
	for (uint64_t i = children; i < 0x8000000000000000; i+= children * 10) {
		string ret = point(i, children);
//		file << ret << endl;
	}
//	file.close();
}

int main()
{
	test();
	return 0;
}
