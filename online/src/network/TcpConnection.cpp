 ///
 /// @file    TcpConnection.cc
 /// @author  lemon(haohb13@gmail.com)
 /// @date    2015-11-05 17:02:41
 ///

#include "TcpConnection.h"
#include "EpollPoller.h"
#include <string.h>
#include <stdio.h>


namespace wd
{

TcpConnection::TcpConnection(int sockfd, EpollPoller * loop)
: sockfd_(sockfd)
, sockIO_(sockfd)
, localAddr_(wd::Socket::getLocalAddr(sockfd))
, peerAddr_(wd::Socket::getPeerAddr(sockfd))
, isShutdownWrite_(false)
, loop_(loop)
{}


TcpConnection::~TcpConnection()
{
	if(!isShutdownWrite_)
	{
		isShutdownWrite_ = true;
		shutdown();
	}
	printf("~TcpConnection()\n");
}

std::string TcpConnection::receive()//接收对端发送过来的连接。
{
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	size_t ret = sockIO_.readline(buf, sizeof(buf));
	if(ret == 0)
	{
		return std::string();
	}
	else
		return std::string(buf);
}

void TcpConnection::send(const std::string & msg)
{
	sockIO_.writen(msg.c_str(), msg.size());
}

//针对php服务器
void TcpConnection::sendAndClose(const std::string & msg)
{
	send(msg);//发送信息，文件描述符由Socket封装提供
	shutdown();//关闭Socket连接
}

void TcpConnection::sendInLoop(const std::string & msg)
{
	loop_->runInLoop(std::bind(&TcpConnection::sendAndClose, this, msg));
}

void TcpConnection::shutdown()
{
	if(!isShutdownWrite_)
		sockfd_.shutdownWrite();
	isShutdownWrite_ = true;
}

std::string TcpConnection::toString()
{
	char str[100];//字符串拼接
	snprintf(str, sizeof(str), "%s:%d -> %s:%d",
			 localAddr_.ip().c_str(),
			 localAddr_.port(),
			 peerAddr_.ip().c_str(),
			 peerAddr_.port());
	return std::string(str);
}

//初始化回调函数
void TcpConnection::setConnectionCallback(TcpConnectionCallback cb)
{
	onConnectionCb_ = cb;
}

void TcpConnection::setMessageCallback(TcpConnectionCallback cb)
{
	onMessageCb_ = cb;
}

void TcpConnection::setCloseCallback(TcpConnectionCallback cb)
{
	onCloseCb_ = cb;
}
//如果回调函数为空，初始化回调函数
void TcpConnection::handleConnectionCallback()
{
	if(onConnectionCb_)
		onConnectionCb_(shared_from_this());
}

void TcpConnection::handleMessageCallback()
{
	if(onMessageCb_)
		onMessageCb_(shared_from_this());//获取监控的对象知智能指针，以便在WordQuerySever里面调用该对象的相关成员函数。实现发送和接收
}

void TcpConnection::handleCloseCallback()
{
	if(onCloseCb_)
		onCloseCb_(shared_from_this());
}

}// end of namespace wd
