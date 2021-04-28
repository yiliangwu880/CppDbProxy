配置工具
使用：
	）导出配置相关的h文件：
		运行export_cpp_cfg。比如：./export_cpp_cfg ../export_cpp/test_cfg.txt
		成功会生成 CC_test_cfg.h文件
	）你的项目加入导出文件 CC_test_cfg.h,加入com目录的cpp文件。
	）你的项目代码包含 CC_test_cfg.h，初始化配置和读取配置。例子如下：
		test_cfg cfg;			//CC_test_cfg.h里面导出的类
		cfg.LoadFile();			//解析配置文件
		uint64 read = cfg.a1;	//读取配置使用
		
详情参考：https://github.com/yiliangwu880/cpp_cfg.git