 ///
 /// @file    Threadpool.h
 /// @author  Damon(1225228598@qq.com)
 /// @date    2015-11-03 15:49:51
 ///


#ifndef __WD_THREADPOOL_H
#define __WD_THREADPOOL_H

#include "Buffer.h"
#include <vector>
#include <functional>

namespace wd
{
class Thread;

class Threadpool
{
public:
	typedef std::function<void()> Task;//一个任务即一个函数
	Threadpool(size_t threadsNum, size_t bufNum);
	~Threadpool();

	void start();
	void stop();

	void addTask(Task task);

private:
	void threadFunc();//写成私有，只希望内部访问。
	Task getTask();

private:
	size_t threadsNum_;
	std::vector<Thread *> vecThreads_;//定义一个容器来存线程指针。
	Buffer buf_;//定义一个队列来存任务，所有线程共用一个队列。
	bool isExit_;
};

}// end of namespace wd

#endif
