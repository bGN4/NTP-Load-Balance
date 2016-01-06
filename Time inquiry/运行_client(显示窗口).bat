@echo off
echo.
echo Usage:
echo     Release\client.exe [client_id] [usr_id] [LB_id] [count] [LB_ip] [LB_port]
echo     修改for函数中的值可以改变运行的Clinet的实例的个数
echo.
echo 例:
echo     命令:
echo         "for /l %%i in (6 1 10)do start Release\client.exe %%i %%i 1 10 192.168.1.1 123"
echo     含义:
echo         打开5个Client实例,
echo         Client ID 分别为6,7,8,9,10;
echo         usr_id 分别为6,7,8,9,10;
echo         LB的ID为1;
echo         共发送10个数据包;
echo         LB的IP为192.168.1.1;
echo         LB的端口为123;
echo.
for /l %%i in (6 1 10)do start Release\client.exe %%i %%i 1 10 127.0.0.1 123