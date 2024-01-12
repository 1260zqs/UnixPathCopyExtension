# Windows右键菜单扩建
## 复制为路径 (/) Unix格式
系统自带的复制为路径为"\"方式, 粘贴到代码中非常烦人, 在此特别引入Unix路径格式("/")复制
eg.
C:\Windows\System32\kernel32.dll
C:/Windows/System32/kernel32.dll

##安装
regsvr32 CopyPath_win_x64.dll

##卸载
regsvr32 /u CopyPath.dll 
