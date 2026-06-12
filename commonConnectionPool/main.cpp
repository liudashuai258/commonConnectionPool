#include<mysql.h>
#include<iostream>
#include<string>
#include <cstdlib>
#include "Connection.h"
#include "commonConnectionPool.h"
using namespace std;
int main() {
	/*
	Connection conn;
	char sql[1024] = { 0 };
	sprintf_s(sql, "insert into user(name,age,sex) values('%s',%d,'%s')","zhang san",20, "male");

	conn.connect("127.0.0.1", 3306, "root", "837296", "chat");
    conn.update(sql);
	
	*/
	ConnectionPool* cp = ConnectionPool::getConnectionPool();

	system("pause");
	return 0;
}