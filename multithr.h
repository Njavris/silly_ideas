#pragma once
#include <thread>
#include <cstdint>
#include <iostream>

#include <unistd.h>
#include <fcntl.h>

#ifdef DEBUG
#define tdebug(...)	printf(__VA_ARGS__)
#else
#define tdebug(...)
#endif

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
	static void *thread_fn(void *arg);
	void spawn(Worker *w);
public:
	TaskManager();
	~TaskManager();
	void spawnWorker(Worker *w);
	bool checkWorkers();
	bool allDone();
};
