简介：
	db代理服务器,给业务服务器读写db（mysql,mongodb,redis）用。
	特点：
		利用protobuf的ORM技术（对象-关系映射）。
		表结构用proto文件表达。用户不需要写db读写的语句（比如sql语句），不需要写db数据和对象之间的读写代码。
		首次连接db,会自动在db创建表。
		dbproxy_svr不算完整组件，只是个组件模板，需要根据业务修改重新编译。 参考“用户使用结构描述”

详细功能说明参考： doc.lua文件	

编译方法：
	整个文件夹放到linux目录，安装cmake gcc git等。

	git submodule init      --更新子模块
	git submodule update			
	git checkout -f	master		--强制删掉本地分支， track远程master分支
	去目录 external里面，参考说明编译每个文件夹，生成依赖库。
	主目录执行：sh clearBuild.sh 完成编译

protobuf-3.10.1源码编译方法：
	apt-get install autoconf automake libtool curl make g++ unzip    --安装相关工具
	./autogen.sh
	./configure --prefix=/home/xiaopao/test/dbproxy/external/protobuf   --设置安装目录
	make check								--这部可能会出错，继续安装。用着没发现问题不用管了，有问题查官网
    make install				
	
vs浏览代码：
	执行.\vs\run.bat,生成sln文件

	
目录结构：
	dbproxy_svr              ==db代理服务器
	bin			             ==执行文件
	test 		             ==测试
	external/protobuf        ==protobuf头文件，库，工具
	external/protobuf-3.10.1 ==protobuf源码

使用方法：
	编译通过本项目。
	根据你的业务，用proto文件定义存库结构，导出 proto cpp
	dbproxy_svr 不需要修改代码，但需要加入proto cpp重新编译。
	svr,就是你的业务服务器。 加入库db_driver（目录db_driver里面有库和头文件），就可以通过请求dbproxy_svr读写db了。
	详情看：doc.lua
