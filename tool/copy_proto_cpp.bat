REM 复制远程linux proto导出文件到win目录

xcopy Z:\test\dbproxy\proto\*.h E:\my\dbproxy\proto\ /sy
xcopy Z:\test\dbproxy\proto\*.cc E:\my\dbproxy\proto\ /sy
xcopy Z:\test\dbproxy\dbproxy_svr\cpp_cfg\*.h E:\my\dbproxy\dbproxy_svr\cpp_cfg\ /sy

pause