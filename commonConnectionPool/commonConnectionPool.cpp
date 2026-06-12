#pragma once
#include<queue>
#include"public.h"
#include"commonConnectionPool.h"

//线程安全的懒汉单例函数接口
ConnectionPool* ConnectionPool::getConnectionPool() {
	static ConnectionPool pool;//lock和unlock
	return &pool;
}

//从配置文件中加载配置项
bool ConnectionPool::loadConfigFile() {
	FILE* pf = nullptr;
	if (fopen_s(&pf, "mysql.ini", "r") != 0 || pf == nullptr) {
		LOG("mysql.ini file is not exist!");
		return false;
	}

	while (!feof(pf)) {
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		string str = line;
		int idx = str.find('=', 0);
		if (idx == -1) {
			continue;
		}
		int endidx = str.find('\n', idx);
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endidx - idx - 1);

		if (key == "ip") {
			_ip = value;

		}
		else if (key == "port") {
			_port = atoi(value.c_str());
		}
		else if (key == "username") {
			_username = value;
		}
		else if (key == "password") {
			_password = value;
		}
		else if (key == "dbname") {
			_dbname = value;
		}
		else if (key == "initsize") {
			_initsize = atoi(value.c_str());
		}
		else if (key == "maxsize") {
			_maxsize = atoi(value.c_str());
		}
		else if (key == "maxidletime") {
			_maxidletime = atoi(value.c_str());
		}
		else if (key == "connectiontimeout") {
			_connectiontimeout = atoi(value.c_str());
		}
		
	}
	return true;
}

//连接池的构造
ConnectionPool::ConnectionPool() {
	if (!loadConfigFile()) {
		LOG("加载配置文件失败!");
		return;
	}
	//创建初始数量的链接
	for (int i = 0;i < _initsize;++i) {
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime();//刷新一下连接的起始的空闲时间点
		_connectionQue.push(p);
		_connectionCnt++;
	}

	//启动一个新的线程，作为连接的生产者
	thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
	produce.detach();
	//启动一个新的定时线程，扫描超过maxidletime时间的空闲连接，进行对于的链接回收

	thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();
}

//运行在独立的线程中的，专门负责成产新的链接
void ConnectionPool::produceConnectionTask() {
	while (1) {
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectionQue.empty()) {
			_cv.wait(lock);//队列不空，此生产线程进入等待状态，等待消费者线程消费连接后通知生产线程继续生产
		}
		if (Connection* p = new Connection()) {
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime();//刷新一下连接的起始的空闲时间点
			_connectionQue.push(p);
			_connectionCnt++;
		}

		//通知消费者线程可以消费连接了
		_cv.notify_all();


	}
}


//给外部提供一个接口，从连接池中获取一个可用的空闲连接
shared_ptr<Connection>ConnectionPool::getConnection() {
	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty()) {
		if (cv_status::timeout==_cv.wait_for(lock, chrono::milliseconds(_connectiontimeout))) {
			if (_connectionQue.empty()) {
				LOG("获取连接超时了，获取连接失败!");
				return nullptr;
			}
		}
		/*
		shared_ptr智能指针析构时，会把connection资源直接释放掉，相当于调用connection的析构函数
		connection就被close掉了
		这里需要自定义shatred_ptr的释放资源的方式，把connection直接归还到queue当中
		*/

		shared_ptr<Connection>sp(_connectionQue.front(),[&](Connection *pcon){
			//这里是在服务器应用线程中调用的，所以一定要考虑队列的线程安全操作
			unique_lock<mutex> lock(_queueMutex);
			pcon->refreshAliveTime();//刷新一下连接的起始的空闲时间点
			_connectionQue.push(pcon);
		});
		_connectionQue.pop();
			_cv.notify_all();//
		return sp;
	}



}

//扫描超过maxidletime时间的空闲连接，进行对于的链接回收
void ConnectionPool::scannerConnectionTask() {
	while (1) {
		//通过sleep模拟定时效果
		this_thread::sleep_for(chrono::seconds(_maxidletime));

		//扫描整个队列，释放多余的链接
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt >_initsize) {
			Connection* p = _connectionQue.front();
			if (p->getAliceTime() >= (_maxidletime * 1000)) {
				_connectionQue.pop();
				_connectionCnt--;
				delete p;//调用connection的析构函数，释放连接资源
			}
			else {
				break;//队头的连接没有超过最大空闲时间，后面的连接更不可能超过了，所以直接退出扫描
			}
		}
	}
}

