@echo off
echo.
echo Usage:
echo     Release\server.exe [server_id] [LB_id] [port]
echo     修改for函数中的值可以改变运行的Server的实例的个数
echo.
echo 例:
echo     命令:
echo         "for /l %%i in (5000 1 5002)do start Release\server.exe %%i 1 %%i"
echo     含义:
echo         打开3个Server实例,
echo         Server ID 分别为5000,5001,5002;
echo         LB的ID为1;
echo         端口分别为5000,5001,5002;
echo.
for /l %%i in (5000 1 5002)do start Release\server.exe %%i 1 %%i