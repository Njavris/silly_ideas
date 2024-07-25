#include <thread>
#include <cstdint>
#include <iostream>
#include <vector>
#include <cstdlib>

#include <unistd.h>
#include <fcntl.h>

using namespace std;

class Worker {
public:
	int fd;
	pthread_t ptid;
	Worker(int id) : wid(id), done(false) {};
	int arg;
	int result;
	int wid;
	bool done;
	virtual void doWork() = 0;
	virtual void doCallback() = 0;
};

class Test1 : public Worker {
public:
	Test1(int id) : Worker(id) {};
	void doWork() {
		pthread_detach(pthread_self());
		for (int i = 0; i < arg; i++) {
			printf("%d: thread loop: %d -> %d\n", wid, i, arg);
			sleep(1);
		}
		printf("%d done!\n", wid);
	};
	void doCallback() {
	};
};

class Test2 : public Worker {
public:
	Test2(int id) : Worker(id) {};
	void doWork() {
		result = 0;
		pthread_detach(pthread_self());
		for (int i = 0; i < arg; i++) {
			result += i;
			printf("%d: thread loop: %d -> %d\n", wid, i, arg);
			sleep(1);
		}
		printf("%d done! result=%d\n", wid, result);
	};
	void doCallback() {
	};
};

class Manager {
	int children;
	int fd[2];
	static void *thread_fn(void *arg) {
		Worker *w = (Worker *)arg;
		printf("Child: %d [%lx]\n", w->wid, pthread_self());
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
	Manager() : children(0) {
		int r;
		pipe(fd);
		r = fcntl(fd[0], F_GETFL);
		r |= O_NONBLOCK;
		fcntl(fd[0], F_SETFL, r);
	};
	void spawnWorker(Worker *w) {
		w->fd = fd[1];
		spawn(w);
		children ++;
	};
	bool checkWorkers() {
		pthread_t tid;
		if (read(fd[0], &tid, sizeof(tid)) > 0) {
			printf("Joining %lx\n", tid);
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

int main()
{
	int worker = 0;
	int children = 4;
	int fd[2];

	std::vector<Worker *> workers;
	Manager m;
	srand(time(0));
	pipe(fd);

	printf("Parent: %lx\n", pthread_self());
	for (int i = 0; i < children; i++) {
		Worker *w;
		if (i < children / 2)
			w = new Test1(i);
		else
			w = new Test2(i);
		w->arg = rand() % 16;
		workers.push_back(w);
		m.spawnWorker(w);
	}

	while (!m.allDone()) {
		m.checkWorkers();
	}
	printf("All children joined\n");
	for (auto &w: workers)
		delete w;
	return 0;
}
