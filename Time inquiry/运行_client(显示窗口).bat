@echo off
echo.
echo Usage:
echo     Release\client.exe [client_id] [usr_id] [LB_id] [count] [LB_ip] [LB_port]
echo     �޸�for�����е�ֵ���Ըı����е�Clinet��ʵ���ĸ���
echo.
echo ��:
echo     ����:
echo         "for /l %%i in (6 1 10)do start Release\client.exe %%i %%i 1 10 192.168.1.1 123"
echo     ����:
echo         ��5��Clientʵ��,
echo         Client ID �ֱ�Ϊ6,7,8,9,10;
echo         usr_id �ֱ�Ϊ6,7,8,9,10;
echo         LB��IDΪ1;
echo         ������10�����ݰ�;
echo         LB��IPΪ192.168.1.1;
echo         LB�Ķ˿�Ϊ123;
echo.
for /l %%i in (6 1 10)do start Release\client.exe %%i %%i 1 10 127.0.0.1 123