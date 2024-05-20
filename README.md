# -
C++网络通信
### 概述

这是一个基于WinSock的多线程TCP网络通信模块

该模块包含IPv4/IPv6套接字的创建/连接以及关于取得数据/连接/断开连接的事件处理



### 函数介绍

##### 1.服务端

​	1.bool creatV4(int port, void(*getData_a)(server svr), void(*connect_a)(server svr), void(*disconnect_a)(server svr))

​		该函数将创建一个IPv4服务器套接字；port参数表示将要绑定的端口；void(*getData_a)(server svr)参数即为接收到数据后执行的函数指针，此后同理



​	2.bool creatV6(int port, void(*getData_a)(server svr), void(*connect_a)(server svr), void(*disconnect_a)(server svr))

​		与v4相同，不过创建的是IPv6套接字，这两个函数都将启动一个线程以监听连接，并在获取连接后再次启动新线程监听数据



​	3.bool sendCharArrToClient(int num, char* byte, int lenOfData)

​		此函数将一个字符指针所指向的数据发送至客户端；num参数表示客户端的编号，char*表示指针首地址，lenOfData表示数据长度



​	4.bool sendStringToClient(int num, std::vector<char> byte)

​		此函数将一个vector<char>的数据发送至客户；num表示客户端编号，byte表示数据



​	5.int serverGetLastRecv()

​		此函数返回最后一个向服务器发送数据的客户编号



​	6.int serverGetLastDisconnect()

​		此函数将返回最后一个断开连接的编号



​	7.int serverGetLastConnect()

​		此函数将返回最后一个连接的编号



​	8.std::vector<char> serverGetData()

​		此函数将返回最后一次接受的数据





##### 2.客户端

​	嗯......几乎与服务端同理（才不是我懒呢！）



总之，这个项目就是**

