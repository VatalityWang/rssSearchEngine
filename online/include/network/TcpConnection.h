 ///
 /// @file    TcpConnection.h
 /// @author  lemon(haohb13@gmail.com)
 /// @date    2015-11-05 16:59:04
 ///

#ifndef _WD_TCPCONNCETION_H
#define _WD_TCPCONNCETION_H

#include "Noncopyable.h"
#include "InetAddress.h"
#include "Socket.h"
#include "SocketIO.h"

#include <string>
#include <memory>
#include <functional>

namespace wd
{

class EpollPoller;

class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;//用共享指针来监控自身的对象

class TcpConnection : Noncopyable,
	public std::enable_shared_from_this<TcpConnection>
{
public:
	typedef std::function<void(const TcpConnectionPtr &)> TcpConnectionCallback;// 注册指向自身的回调函数指针
	TcpConnection(int sockfd, EpollPoller * loop);
	~TcpConnection();

	std::string receive();
	void send(const std::string & msg);
	void sendAndClose(const std::string & msg);
	void sendInLoop(const std::string & msg);
	void shutdown();

	std::string toString();

	void setConnectionCallback(TcpConnectionCallback cb);
	void setMessageCallback(TcpConnectionCallback cb);
	void setCloseCallback(TcpConnectionCallback cb);

	void handleConnectionCallback();
	void handleMessageCallback();
	void handleCloseCallback();

private:
	Socket sockfd_;
	SocketIO sockIO_;
	const InetAddress localAddr_;
	const InetAddress peerAddr_;
	bool isShutdownWrite_;
	EpollPoller * loop_;

	TcpConnectionCallback onConnectionCb_;//初始化回调函数的三个变量
	TcpConnectionCallback onMessageCb_;
	TcpConnectionCallback onCloseCb_;

};

}//end of namespace wd

#endif
