 ///
 /// @file    Buffer.cc
 /// @author  lemon(haohb13@gmail.com)
 /// @date    2015-11-02 11:31:01
 ///
//由任务条件变量和锁共同控制任务队列
#include "Buffer.h"

namespace wd
{
Buffer::Buffer(size_t size)
	:	mutex_(),
		notFull_(mutex_),// 给条件变量赋锁的初值
		notEmpty_(mutex_),//tongshang
		size_(size),
		flag_(true)
{
}

bool Buffer::full()
{
	return size_ == que_.size();
}

bool Buffer::empty()
{
	return 0 == que_.size();
}
	  
void Buffer::push(Task task)
{
	MutexLockGuard guard(mutex_);//局部对象
	while(full())//使用while是为了防止被异常唤醒，如果任务队列一直是满的，一直等待，直到不满为止。
	{
		notFull_.wait();
	}
	que_.push(task);
	
	notEmpty_.notify();//给等待在该条件上的变量线程发信号

}


Buffer::Task Buffer::pop()//取任务
{
	MutexLockGuard guard(mutex_);//加锁
	//mutex_.lock();

	while(empty() && flag_)//判空
	{
		notEmpty_.wait();
	}

	if(flag_)
	{
		Task task = que_.front();
		que_.pop();

		notFull_.notify();
		//mutex_.unlock();
		return task;
	}
	else
	{
		//mutex_.unlock();
		return NULL;
	}
}


void Buffer::wakeup_empty()
{
	notEmpty_.notifyall();//为空则通知所有线程。
}

void Buffer::set_flag(bool flag)
{
	flag_ = flag;
}

}//end of namespace wd
