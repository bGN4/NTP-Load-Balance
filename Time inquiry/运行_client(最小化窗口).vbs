MsgBox """Release\client.exe"&VBcrlf&_
         "[client_id] [usr_id] [LB_id] [count] [LB_ip] [LB_port]"",WindowStyle"&VBcrlf&_
         VBcrlf&"WindowStyle:"&VBcrlf&VBTab&_
         "0"&VBTab&"Hide the window"&VBcrlf&VBTab&_
         "1"&VBTab&"Display the window"&VBcrlf&VBTab&_
         "2"&VBTab&"Minimize the window",0,"Usage"
for i = 6 to 9
createobject("wscript.shell").run "Release\client.exe "&i&" "&i&" 1 10 127.0.0.1 123",2
Next