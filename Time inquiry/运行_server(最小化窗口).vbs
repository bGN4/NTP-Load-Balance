MsgBox """Release\server.exe [server_id] [LB_id] [port]"",WindowStyle"&VBcrlf&_
         VBcrlf&"WindowStyle:"&VBcrlf&VBTab&_
         "0"&VBTab&"Hide the window"&VBcrlf&VBTab&_
         "1"&VBTab&"Display the window"&VBcrlf&VBTab&_
         "2"&VBTab&"Minimize the window",0,"Usage"
for i = 5000 to 5002
createobject("wscript.shell").run "Release\server.exe "&i&" 1 "&i,2
Next