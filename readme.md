简介：
	db代理服务器,给业务服务器读写db mysql, （将来扩展mongodb,redis，只需要dbproxy_svr 新增实现就可以）。
	特点：
		ORM技术（对象-关系映射）。
		表结构用C++ struct表达。用户不需要写sql语句，不需要写db 对象和 c++ 之间的转换代码。
		首次连接db,会自动在db创建表。
		dbproxy_svr不算完整组件，只要table结构变化，需要重新编译。 参考“用户使用结构描述”

详细功能说明参考： doc.lua文件	

编译方法：
	整个文件夹放到linux目录，安装cmake gcc git等。

	git submodule init      --更新子模块
	git submodule update			
	git checkout -f	master		--强制删掉本地分支， track远程master分支
	去目录 external里面，参考说明编译每个文件夹，生成依赖库。
	
	sh run.sh  --CppDbProxy/dbproxy_svr/DynamicCfg 目录执行 sh run.sh 生成配置 h文件
	主目录执行：sh clearBuild.sh 完成编译


目录结构：
	dbproxy_svr              ==db代理服务器
	bin			             ==执行文件
	test 		             ==测试
	proto					 ==业务服务器 和 dbproxy_svr 公用的源码

使用方法：
	
	编译通过本项目。

	dbproxy_svr table结构变化，需要重新编译。
	业务服务器。 加入库db_driver（目录db_driver里面有库和头文件），就可以通过db_driver读写db了。
	详情看：doc.lua
