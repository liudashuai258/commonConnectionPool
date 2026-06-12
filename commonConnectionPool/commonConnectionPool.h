// 实现的是连接池功能模块
#pragma once
#include<string>
#include<queue>
#include<mutex>
#include<atomic>
#include<thread>
#include<memory>
#include<functional>
#include"Connection.h"
#include<condition_variable>
using namespace std;
class ConnectionPool {
public:
	//获取连接池对象实例
	static ConnectionPool* getConnectionPool();
	//给外部提供接口，从连接池中获取一个可用的数据库连接
	shared_ptr<Connection>getConnection();
	//ps:shared_ptr是C++11引入的智能指针，能够自动管理内存，避免内存泄漏。
	// 当shared_ptr对象被销毁时，它所管理的资源（在这里是Connection对象）也会被自动释放。
private:
	//单例#1  构造函数私有化
	ConnectionPool();
public:
	//从配置文件中加载配置项
	bool loadConfigFile();

	//运行在独立的线程中的，专门负责成产新的链接
	void produceConnectionTask();

	//启动一个新的定时线程，扫描超过maxidletime时间的空闲连接，进行对于的链接回收
	void scannerConnectionTask();


	string _ip;//mysql的ip地址
	unsigned short _port;//mysql的端口号3306
	string _username;//mysql的登录用户名
	string _password;//mysql的密码
	string _dbname;//mysql的数据库名称
	int _initsize;//连接池的初始连接量
	int _maxsize;//连接池的最大连接量
	int _maxidletime;//连接池的最大空闲时间
	int _connectiontimeout;//连接池获取链接的超时时间
	
	queue<Connection*> _connectionQue;//存储MySQL链接的队列
	mutex _queueMutex;//维护链接队列的线程安全互斥锁
	atomic_int _connectionCnt;//记录连接池创建的connection连接的总数量
	//ps:atomic_int是C++11引入的原子类型,计算的是所有线程创建的connection数量，
	// 因为一个服务器可能有多个线程池，每个线程池都创建connection所以用atomic_int来记录总数量，保证线程安全。

	
	//条件变量，配合互斥锁实现线程间的等待和通知机制,
	//用于连接生产者和连接消费线程的通信
	condition_variable _cv;
	


};