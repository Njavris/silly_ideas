#include <thread>
#include <cstdint>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <string>
#include <fstream>

#include <unistd.h>
#include <fcntl.h>

//#define DEBUG

#ifdef DEBUG
#define tdebug(...)	printf(__VA_ARGS__)
#else
#define tdebug(...)
#endif

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


class Worker {
public:
	int fd;
	pthread_t ptid;
	Worker(int id) : wid(id), done(false) {};
	uint64_t arg;
	uint64_t arg1;
	uint64_t result;
	int wid;
	bool done;
	virtual void doWork() = 0;
	virtual void doCallback() = 0;
};

class TaskManager {
	int children;
	int fd[2];
	static void *thread_fn(void *arg) {
		Worker *w = (Worker *)arg;
		tdebug("Child: %d [%lx]\n", w->wid, pthread_self());
		w->doWork();
		w->done = true;
		w->doCallback();
		write(w->fd, &w->ptid, sizeof(w->ptid));
		pthread_exit(NULL);
	};
	void spawn(Worker *w) {
		pthread_create(&w->ptid, NULL, &thread_fn, (void *)w);
		pthread_detach(w->ptid);
	};
public:
	TaskManager() : children(0) {
		int r;
		pipe(fd);
		r = fcntl(fd[0], F_GETFL);
		r |= O_NONBLOCK;
		fcntl(fd[0], F_SETFL, r);
	};
	~TaskManager() {
		close(fd[0]);
		close(fd[1]);
	};
	void spawnWorker(Worker *w) {
		w->fd = fd[1];
		spawn(w);
		children ++;
	};
	bool checkWorkers() {
		pthread_t tid;
		if (read(fd[0], &tid, sizeof(tid)) > 0) {
			tdebug("Joining %lx\n", tid);
			pthread_join(tid, NULL);
			children --;
			return true;
		}
		return false;
	};
	bool allDone() {
		return children == 0;
	};
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
/*
	sum = (max * (max + 1)) / 2
	2 * sum = max * max + max
	2 * sum = 18446744073709551614
	2 * 9223372036854775807 = max^2 + max
	
*/

string test(int target, int children) {
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

int main()
{
	ofstream file;
//	file.open("test.csv");
	int children = 4;
	for (uint64_t i = children; i < 0x8000000000000000; i+= children * 10) {
		string ret = test(i, children);
//		file << ret << endl;
	}
//	file.close();
	return 0;
}
