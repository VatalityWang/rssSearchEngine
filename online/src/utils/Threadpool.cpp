 ///
 /// @file    Threadpool.cc
 /// @author  lemon(haohb13@gmail.com)
 /// @date    2015-11-03 15:53:35
 ///

#include "Threadpool.h"
#include "Thread.h"
#include <stdio.h>

namespace wd
{
Threadpool::Threadpool(size_t threadsNum, size_t bufNum)
	: threadsNum_(threadsNum),
	  buf_(bufNum),
	  isExit_(false)
{
	printf("Threadpool()\n");
}

Threadpool::~Threadpool()
{
	if(!isExit_)
	{
		stop();
	}
}

void Threadpool::start()
{
	for(size_t idx = 0; idx != threadsNum_; ++idx)
	{
		Thread * pThread = new Thread(
			std::bind(&Threadpool::threadFunc, this));//将该函数注册给线程回调函数，线程注册成功则调用该函数。
		vecThreads_.push_back(pThread);
	}

	std::vector<Thread *>::iterator it;
	for(it = vecThreads_.begin(); it != vecThreads_.end(); ++it)
	{
		(*it)->start();//见thread.cpp 创建线程，并执行回调函数。
	}
	printf("Threadpool::start()\n");
}


void Threadpool::stop()
{
	if(!isExit_)
	{
		isExit_ = true;
		buf_.set_flag(false);
		buf_.wakeup_empty();//通知所有线程任务队列空了

		std::vector<Thread *>::iterator it;
		for(it = vecThreads_.begin(); it != vecThreads_.end(); ++it)
		{
			(*it)->join();//销毁该线程
			delete (*it);//删除指向该线程的指针
		}
		vecThreads_.clear();//清空线程队列
	}
}

void Threadpool::addTask(Task task)
{
	printf("Threadpool::addTask()\n");
	buf_.push(task);
}

Threadpool::Task Threadpool::getTask()
{
	return buf_.pop();
}

void Threadpool::threadFunc()
{
	while(!isExit_)
	{
		Task task = buf_.pop();
		if(task)
		{
			task();//线程任务函数处理任务
		}
	}
}


}// end of namespace wd
