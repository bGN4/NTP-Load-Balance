@echo off
echo.
echo Usage:
echo     Release\server.exe [server_id] [LB_id] [port]
echo     �޸�for�����е�ֵ���Ըı����е�Server��ʵ���ĸ���
echo.
echo ��:
echo     ����:
echo         "for /l %%i in (5000 1 5002)do start Release\server.exe %%i 1 %%i"
echo     ����:
echo         ��3��Serverʵ��,
echo         Server ID �ֱ�Ϊ5000,5001,5002;
echo         LB��IDΪ1;
echo         �˿ڷֱ�Ϊ5000,5001,5002;
echo.
for /l %%i in (5000 1 5002)do start Release\server.exe %%i 1 %%i