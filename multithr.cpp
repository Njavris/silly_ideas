#include "multithr.h"

void *TaskManager::thread_fn(void *arg) {
	Worker *w = (Worker *)arg;
	tdebug("Child: %d [%lx]\n", w->wid, pthread_self());
	w->doWork();
	w->done = true;
	w->doCallback();
	if (write(w->fd, &w->ptid, sizeof(w->ptid)) == -1)
		exit(-1);
	pthread_exit(NULL);
};

void TaskManager::spawn(Worker *w) {
	pthread_create(&w->ptid, NULL, &thread_fn, (void *)w);
	pthread_detach(w->ptid);
};

TaskManager::TaskManager() : children(0) {
	if (pipe2(fd, O_NONBLOCK) == -1)
		exit(-1);
};

TaskManager::~TaskManager() {
	close(fd[0]);
	close(fd[1]);
};

void TaskManager::spawnWorker(Worker *w) {
	w->fd = fd[1];
	spawn(w);
	children ++;
};

bool TaskManager::checkWorkers() {
	pthread_t tid;
	if (read(fd[0], &tid, sizeof(tid)) > 0) {
		tdebug("Child [%lx] finished\n", tid);
		children --;
		return true;
	}
	return false;
};
bool TaskManager::allDone() {
	return children == 0;
};

