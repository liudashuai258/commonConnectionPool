# commonConnectionPool

`commonConnectionPool` 是一个基于 C++ 实现的 MySQL 数据库连接池项目。

本项目通过提前创建一定数量的数据库连接，并使用线程安全的连接池统一管理连接资源，减少频繁创建和释放数据库连接带来的性能开销，适用于高并发服务端程序中的数据库访问场景。

## 项目功能

* MySQL 数据库连接封装
* 数据库连接池统一管理
* 支持从配置文件读取数据库参数
* 支持连接池初始连接数配置
* 支持最大连接数配置
* 支持最大空闲时间配置
* 支持连接超时时间配置
* 使用队列管理空闲连接
* 使用互斥锁保证线程安全
* 使用条件变量实现连接等待与唤醒
* 使用智能指针自动归还连接
* 支持多线程环境下复用数据库连接

## 项目结构

```text
commonConnectionPool/
├── commonConnectionPool.sln
├── commonConnectionPool/
│   ├── Connection.h
│   ├── Connection.cpp
│   ├── commonConnectionPool.h
│   ├── commonConnectionPool.cpp
│   ├── main.cpp
│   ├── public.h
│   └── mysql.ini
└── .gitignore
```

## 核心模块说明

### 1. Connection

`Connection` 类主要负责对 MySQL 原生接口进行封装。

主要功能包括：

* 初始化数据库连接
* 执行 SQL 更新语句
* 执行 SQL 查询语句
* 记录连接空闲时间
* 封装 MySQL 连接对象

### 2. ConnectionPool

`ConnectionPool` 类是整个项目的核心模块，负责统一管理数据库连接。

主要功能包括：

* 创建数据库连接
* 管理空闲连接队列
* 向外提供可用连接
* 回收使用完的连接
* 动态创建新连接
* 清理长时间空闲连接
* 保证多线程访问安全

### 3. mysql.ini

`mysql.ini` 是数据库连接池配置文件，用于配置数据库连接参数。

常见配置包括：

```ini
ip=127.0.0.1
port=3306
username=root
password=123456
dbname=test
initSize=10
maxSize=1024
maxIdleTime=60
connectionTimeout=100
```

## 工作流程

连接池启动后，会先根据配置文件创建一定数量的初始连接。

当业务线程需要访问数据库时，会从连接池中获取一个可用连接。

如果空闲队列中有连接，则直接取出复用。

如果没有空闲连接，并且当前连接数未达到最大连接数，则创建新连接。

如果连接数已经达到上限，则当前线程会等待一段时间。

当连接使用结束后，不需要手动释放数据库连接，智能指针会自动将连接归还到连接池中。

整体流程如下：

```text
业务线程请求数据库连接
        ↓
ConnectionPool::getConnection()
        ↓
检查空闲连接队列
        ↓
有空闲连接：直接取出
        ↓
无空闲连接：判断是否可以创建新连接
        ↓
获得连接后执行业务 SQL
        ↓
shared_ptr 自动析构
        ↓
连接重新放回连接池
```

## 技术点

* C++
* MySQL C API
* 单例模式
* 生产者消费者模型
* 线程同步
* mutex 互斥锁
* condition_variable 条件变量
* queue 队列
* shared_ptr 智能指针
* RAII 资源管理
* 多线程数据库连接复用

## 项目亮点

* 使用连接池减少数据库连接频繁创建和销毁的开销
* 使用智能指针自定义删除器实现连接自动归还
* 使用条件变量实现连接不足时的线程等待
* 使用独立线程动态补充连接
* 使用独立线程扫描并释放空闲时间过长的连接
* 支持通过配置文件灵活调整连接池参数

## 编译环境

* Windows
* Visual Studio
* C++
* MySQL Server
* MySQL Connector/C 或 MySQL 开发库

## 使用方式

1. 配置 MySQL 环境。

2. 修改 `mysql.ini` 中的数据库连接信息。

3. 使用 Visual Studio 打开：

```text
commonConnectionPool.sln
```

4. 编译并运行项目。

## 测试说明

项目可以通过多线程模拟大量数据库插入操作，观察使用普通数据库连接和使用连接池后的执行耗时差异。

测试目标：

* 验证连接池功能是否正确
* 验证多线程环境下连接获取是否安全
* 验证连接复用是否减少数据库连接创建开销
* 对比普通连接方式和连接池方式的性能差异

## 当前状态

本项目已经完成数据库连接池的核心功能，实现了数据库连接封装、连接池管理、连接复用、线程安全访问和自动归还连接等功能。
