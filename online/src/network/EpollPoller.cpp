 ///
 /// @file    EpollPoller.cc
 /// @author  lemon(haohb13@gmail.com)
 /// @date    2015-11-06 16:18:29
 ///

#include "EpollPoller.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/eventfd.h>


namespace wd
{

int createEpollFd()
{
	int efd = ::epoll_create1(0);
	if(-1 == efd)
	{
		perror("epoll_create1 error");
		exit(EXIT_FAILURE);
	}
	return efd;
}

int createEventFd()
{
	int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);//非阻塞方式,类似于open的O_CLOEXEC,执行exec（）时，之前通过open打开的文件描述符会自动关闭测试，open之后，调用exec（），在新的进程中检测描述符是否已经关闭，初始化计数器的值为0，改值保存在内核中
	if(-1 == evtfd)
	{
		perror("eventfd create error");
	}	
	return evtfd;
}

void addEpollFdRead(int efd, int fd)//将fd注册到efd(epoll)所监听的事件列表中
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN;
	int ret = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);
	if(-1 == ret)
	{
		perror("epoll_ctl add error");
		exit(EXIT_FAILURE);
	}
}

void delEpollReadFd(int efd, int fd)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	int ret = epoll_ctl(efd, EPOLL_CTL_DEL, fd, &ev);
	if(-1 == ret)
	{
		perror("epoll_ctl del error");
		exit(EXIT_FAILURE);
	}
}

int acceptConnFd(int listenfd)
{
	int peerfd = ::accept(listenfd, NULL, NULL);
	if(peerfd == -1)
	{
		perror("accept error");
		exit(EXIT_FAILURE);
	}
	return peerfd;
}


//预览数据
size_t recvPeek(int sockfd, void * buf, size_t len)
{
	int nread;
	do
	{
		nread = ::recv(sockfd, buf, len, MSG_PEEK);
	}while(nread == -1 && errno == EINTR);
	return nread;
}

//通过预览数据，判断conn是否关闭
bool isConnectionClosed(int sockfd)
{
	char buf[1024];
	int nread = recvPeek(sockfd, buf, sizeof(buf));
	if(-1 == nread)
	{
		perror("recvPeek--- ");
		return true;
		//exit(EXIT_FAILURE);//若peer端已关闭连接，会导致server端崩溃
	}
	return (0 == nread);
}

//==========================================


EpollPoller::EpollPoller(int listenfd)
: epollfd_(createEpollFd())
, listenfd_(listenfd)
, wakeupfd_(createEventFd())// 进程，线程间通信
, isLooping_(false)
, eventsList_(1024)
{
	addEpollFdRead(epollfd_, listenfd_);
	addEpollFdRead(epollfd_, wakeupfd_);
}


EpollPoller::~EpollPoller()
{
	::close(epollfd_);
}

void EpollPoller::loop()
{
	isLooping_ = true;
	while(isLooping_)
	{
		waitEpollfd();
	}
}

void EpollPoller::unloop()
{
	if(isLooping_)
		isLooping_ = false;
}


void EpollPoller::runInLoop(const Functor & cb)
{
	{
	MutexLockGuard mlg(mutex_);
	pendingFunctors_.push_back(cb);
	}
	wakeup();
}

void EpollPoller::doPendingFunctors()
{
	printf("doPendingFunctors()\n");
	std::vector<Functor> functors;//处理回调函数的容器
	{
	MutexLockGuard mlg(mutex_);
	functors.swap(pendingFunctors_);//将pendingFunctors中的任务交换到functor中去
	}
	
	for(size_t i = 0; i < functors.size(); ++i)
	{
		functors[i]();//执行该函数
	}
}


void EpollPoller::wakeup()//任务函数插入任务队列的通知机制
{
	uint64_t one = 1;
	ssize_t n = ::write(wakeupfd_, &one, sizeof(one));
	if(n != sizeof(one))
	{
		perror("EpollPoller::wakeup() n != 8");
	}
}

void EpollPoller::handleRead()//处理evenfd,执行读操作。
{
	uint64_t one = 1;
	ssize_t n = ::read(wakeupfd_, &one, sizeof(one));
	if(n != sizeof(one))
	{
		perror("EpollPoller::handleRead() n != 8");
	}
}


void EpollPoller::setConnectionCallback(EpollCallback cb)
{
	onConnectionCb_ = cb;
}

void EpollPoller::setMessageCallback(EpollCallback cb)
{
	onMessageCb_ = cb;
}

void EpollPoller::setCloseCallback(EpollCallback cb)
{
	onCloseCb_ = cb;
}

void EpollPoller::waitEpollfd()
{
	int nready;
	do
	{
		nready = ::epoll_wait(epollfd_, 
							  &(*eventsList_.begin()),
							  eventsList_.size(),
							  5000);//epoll文件描述符，监听事件的结构体的指针，事件列表的大小，阻塞时间5000毫秒
	}while(nready == -1 && errno == EINTR);//返回准备好I/o的文件描述的个数

	if(nready == -1)
	{
		perror("epoll_wait error");
		exit(EXIT_FAILURE);
	}
	else if(nready == 0)
	{
		printf("epoll_wait timeout\n");	
	}
	else
	{
		//做一个扩容的操作
		if(nready == static_cast<int>(eventsList_.size()))
		{
			eventsList_.resize(eventsList_.size() * 2);
		}
		
		//遍历每一个激活的文件描述符
		for(int idx = 0; idx != nready; ++idx)
		{
			if(eventsList_[idx].data.fd == listenfd_)//事件集合文件描述符等于监听的文件描述符，有新的连接加进来
			{
				if(eventsList_[idx].events & EPOLLIN)//普通或者是优先级数据可读
				{
					handleConnection();
				}
			}
			else if(eventsList_[idx].data.fd == wakeupfd_)//一个连接任务处理完毕
			{
				printf("wakeupfd light\n");
				if(eventsList_[idx].events & EPOLLIN)
				{
					handleRead();
					doPendingFunctors();
				}
			}
			else
			{
				if(eventsList_[idx].events & EPOLLIN)//信息处理完毕，直接
				{
					handleMessage(eventsList_[idx].data.fd);
				}
			}
		}//end for
	}//end else
}

void EpollPoller::handleConnection()
{
	int peerfd = acceptConnFd(listenfd_);
	addEpollFdRead(epollfd_, peerfd);//将该连接的描述符加入epoll 监听队列。

	TcpConnectionPtr conn(new TcpConnection(peerfd, this));//每来一个连接创建一个TcpConnection连接的对象
	//...给客户端发一个欢迎信息==> 挖一个空: 等...
	//conn->send("welcome to server.\n");//创建好对象之后，用该对象去掉回调设置函数。
	conn->setConnectionCallback(onConnectionCb_);
	conn->setMessageCallback(onMessageCb_);
	conn->setCloseCallback(onCloseCb_);

	std::pair<ConnectionMap::iterator, bool> ret;//EpollePoller.h 中，有ConnectionMap的定义
	ret = connMap_.insert(std::make_pair(peerfd, conn));
	assert(ret.second == true);
	(void)ret;
	//connMap_[peerfd] = conn;

	conn->handleConnectionCallback();//调用回调函数，将TcpConnection本身的指针传给回调函数。完成连接成功的初始化工作
}

void EpollPoller::handleMessage(int peerfd)
{
	bool isClosed = isConnectionClosed(peerfd);
	ConnectionMap::iterator it = connMap_.find(peerfd);//从当前所有的连接中去找当前有数据可读的连接
	assert(it != connMap_.end());
//找到后检查是否关闭
	if(isClosed)
	{
		it->second->handleCloseCallback();
		delEpollReadFd(epollfd_, peerfd);
		connMap_.erase(it);
	}
	else
	{
		it->second->handleMessageCallback();
	}
}

}// end of namespace wd
