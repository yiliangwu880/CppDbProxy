简介：
	db代理服务器,给业务服务器读写db mysql, （将来扩展mongodb,redis，只需要dbproxy_svr 新增实现就可以）。
	特点：
		ORM技术（对象-关系映射）。
		表结构用C++ struct表达。用户不需要写sql语句，不需要写db 对象和 c++ 之间的转换代码。
		首次连接db,会自动在db创建表。
		dbproxy_svr不算完整组件，只要table结构变化，需要重新编译。 参考“用户使用结构描述”

术语:
	dbproxy	  	== db代理服务器，svr读写db都通过dbproxy转发。
	svr  	  	== 需要读写db的业务服务器。 只连接dbproxy, 不连接db。
	db	  		==  mysql, mongodb
	db_driver   == svr使用的驱动库，处理和dbproxy的交互.
	表			== mysql的表，或者mongodb的集合，这里统一叫表吧。
	ORM			==
					ORM 是"对象-关系映射"（Object/Relational Mapping）的缩写。
					ORM 把数据库映射成对象。
					数据库的表（table）	--> 类（class）
					记录（record，行数据）	--> 对象（object）

问题：
	传统项目，实现svr读写db, 直接用db提供的驱动库，学习，重复开发成本高。 

	举例子说明大多数项目遇到的问题：
	见过最差的做法例子，比如中青宝的MMO服务器框架，开发过程，给表加新字段需要做：
		*.sql脚本： 加字段
		dbproxy:   代码里修改sql生成，多个地方，包括 insert select， update。	
		dbproxy:   dbproxy读写到协议对象的代码片段，加字段。
		dbproxy-svr通讯协议结构： 加字段，
		svr：svr代码片段 加读写到协议对象

		效果：工作效率低，犯错机会大, 非常糟糕。

	 当然很多项目没做那么低级，会写重用代码，一般能做到修改表加新字段需要做：
		*.sql脚本:		加字段
		dbproxy:		某个函数加字段相关内容。	
		通讯协议结构 :	加字段
		svr    :		某个函数加字段相关内容。
		
	 相比之下，本设计可以更轻松，修改表加新字段需要做：
		struct:	加字段，额外宏定义新字段
		dbproxy:重新编译。
		svr:	不需要额外修改db读写相关，原来的读写方式对新字段自动生效。
		
设计目的：
	消灭重复劳动。
	struct + 宏 描述表结构。
	dbproxy_svr 不用人工修改代码。
	svr只需要用函数调用 struct 对象，就能实现所有db读写操作。
	方便转换到不同db。

缺点：
	纯C++,只支持C++的进程。

用户使用结构描述：
	dbproxy_svr 依赖: proto文件夹
	svr 依赖: db_driver proto文件夹
	其中 proto文件夹， 定义业务需求的表结构。
	
功能：
	dbproxy_svr:
	{
		创建db表。
		插入，更新，查询, 删除。
		用户连接限制. 指定svr地址。（可选用）
		兼容多种db. （先实现mysql）
		db没连接成功，拒绝svr连接.
	}
	db_driver:
	{
		插入，更新，查询, 删除。
		响应结果。
		断线重连。
	}


	

	

	