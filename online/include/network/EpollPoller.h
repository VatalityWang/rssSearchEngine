 ///
 /// @file    EpollPoller.h
 /// @author  Damon(1225228598@qq.com)
 /// @date    2015-11-06 16:12:11
 ///


#ifndef __WD_EPOLLPOLLER_H
#define __WD_EPOLLPOLLER_H

#include "Noncopyable.h"
#include "TcpConnection.h"
#include "MutexLock.h"
#include <sys/epoll.h>
#include <vector>
#include <map>
#include <functional>

namespace wd
{
class EpollPoller : Noncopyable
{
public:
	typedef TcpConnection::TcpConnectionCallback EpollCallback;
	typedef std::function<void()> Functor;//定义一系列模板函数

	EpollPoller(int listenfd);
	~EpollPoller();

	void loop();
	void unloop();
	void runInLoop(const Functor & cb);
	void doPendingFunctors();//执行模板函数

	void wakeup();
	void handleRead();

	void setConnectionCallback(EpollCallback cb);//设置连接回调 信息处理回调 关闭连接回调
	void setMessageCallback(EpollCallback cb);
	void setCloseCallback(EpollCallback cb);

private:
	void waitEpollfd();
	void handleConnection();
	void handleMessage(int peerfd);
	
private:
	int epollfd_;
	int listenfd_;
	int wakeupfd_;
	bool isLooping_;

	MutexLock mutex_;
	std::vector<Functor> pendingFunctors_;//任务处理函数

	typedef std::vector<struct epoll_event> EventList;//epoll 监听的事件容器。
	EventList eventsList_;

	typedef std::map<int, TcpConnectionPtr> ConnectionMap;//文件描述符，智能指针
	ConnectionMap connMap_;

	EpollCallback onConnectionCb_;
	EpollCallback onMessageCb_;
	EpollCallback onCloseCb_;
};


}//end of namespace wd

#endif
