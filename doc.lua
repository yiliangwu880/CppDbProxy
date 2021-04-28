简介：
	db代理服务器,给业务服务器读写db（mysql,mongodb）用。
	特点：
		利用protobuf的ORM技术（对象-关系映射）。
		表结构用proto文件表达。用户不需要写db读写的语句（比如sql语句），不需要写db数据和对象之间的读写代码。
		首次连接db,会自动在db创建表。
		dbproxy_svr不算完整组件，这是只是个组件模板，需要根据业务修改重新编译。 参考“用户使用结构描述”

术语:
	dbproxy	  	== db代理服务器，svr读写db都通过dbproxy转发。
	svr  	  	== 需要读写db的业务服务器。 只连接dbproxy, 不连接db。
	db	  		==  mysql, mongodb
	svr_driver  == svr使用的驱动库，处理和dbproxy的交互.
	表			== mysql的表，或者mongodb的集合，这里统一叫表吧。
	ORM			==
					ORM 是"对象-关系映射"（Object/Relational Mapping）的缩写。
					ORM 把数据库映射成对象。
					数据库的表（table）	--> 类（class）
					记录（record，行数据）	--> 对象（object）
					字段（field）			--> 对象的属性（attribute）

问题：
	传统项目，实现svr读写db, 直接用db提供的驱动库，学习成本高，容易写出内存泄漏。 （C++ Driver for MySQL (Connector/C++) 库的接口就是个例子，容易写出泄露内存的代码）。
	运行时架构都流行svr--dbproxy--db的的结构。

	举例子说明大多数项目遇到的问题：
	见过最差的做法例子，比如中青宝的MMO服务器框架，修改表加新字段需要做：
		*.sql脚本： 加字段
		dbproxy:   加sql insert 语句加字段内容。	
		dbproxy:   加sql select 语句加字段内容。	
		dbproxy:   加sql select 查询结果 获取字段代码。
		dbproxy:   加sql update 语句加字段内容。	
		dbproxy:   dbproxy读写到协议对象的代码片段，加字段。
		通讯协议结构： 加字段，
		svr：svr代码片段 加读写到协议对象

		效果：工作效率低，犯错机会大。（题外话：导致这样原因不是公司的大佬技术不行，是大佬技术管理不注重这些点）
		

	 当然很多项目没做那么低级，会写重用代码，一般能做到修改表加新字段需要做：
		*.sql脚本:		加字段
		dbproxy:		某个函数加字段相关内容。	
		通讯协议结构 :	加字段
		svr    :		某个函数加字段相关内容。
		
	 相比之下，dbproxy可以更轻松，修改表加新字段需要做：
		protobuf协议:	加字段
		dbproxy:		导出proto cpp, 重新编译。
		svr:			导出proto cpp文件就包含协议对象读写接口。不用手写。
		
设计目的：
	消灭重复劳动。
	protobuf文描述表结构。
	dbproxy_svr 不用人工修改代码。
	svr只需要读写protobuf对象，就能实现所有db读写操作。
	方便转换到不同db。

缺点：
	依赖protobuf3做协议通讯

用户使用结构描述：
	dbproxy_svr 依赖: proto_file
	svr 依赖: db_driver proto_file
	其中 proto_file， 需要用户自定义业务需求的表结构。
	
功能：
	dbproxy_svr:
	{
		用protobuf定义表字段。
		创建db表。
		初始化检查表和proto msg是否不匹配。
		插入，更新，查询, 删除。
		用户连接限制. 指定svr地址。（可选用）
		兼容多种db. （先实现mysql）
		db没连接成功，拒绝svr连接.
	}
	db_driver:
	{
		插入，更新，查询, 删除。
		响应结果。
		proto msg 不匹配 通知。
		断线重连。
	}

协议：
	svr和dbproxy通讯包结构：cmd,msg
	cmd 消息号
	msg 具体消息内容，protobuf结构。


mysql proto格式：
	不支持repeated字段，
	支持Message字段，数据库会转换成blob字段，不能直接用查看内容。

mongodb proto格式
	

	

	