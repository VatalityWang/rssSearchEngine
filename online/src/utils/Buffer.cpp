 ///
 /// @file    Buffer.cc
 /// @author  Damon(1225228598@qq.com)
 /// @data    2017-11-02 11:31:01
 ///
//��������������������ͬ�����������
#include "Buffer.h"

namespace wd
{
Buffer::Buffer(size_t size)
	:	mutex_(),
		notFull_(mutex_),// ���������������ĳ�ֵ
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
	MutexLockGuard guard(mutex_);//�ֲ�����
	while(full())//ʹ��while��Ϊ�˷�ֹ���쳣���ѣ�����������һֱ�����ģ�һֱ�ȴ���ֱ������Ϊֹ��
	{
		notFull_.wait();
	}
	que_.push(task);
	
	notEmpty_.notify();//���ȴ��ڸ������ϵı����̷߳��ź�

}


Buffer::Task Buffer::pop()//ȡ����
{
	MutexLockGuard guard(mutex_);//����
	//mutex_.lock();

	while(empty() && flag_)//�п�
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
	notEmpty_.notifyall();//Ϊ����֪ͨ�����̡߳�
}

void Buffer::set_flag(bool flag)
{
	flag_ = flag;
}

}//end of namespace wd
