call "G:\Program Files\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"

sdk\bin\Flex++.exe -oss.yy.cpp SkinScript.l

sdk\bin\bison.exe -d SkinScript.y

del SkinScript.tab.cpp
rename SkinScript.tab.c SkinScript.tab.cpp

del ss.exe

cl /D UNICODE /D _UNICODE /D _DEBUG ss.yy.cpp SkinScript.tab.cpp SkinScript.cpp -o ss.exe

ss.exe < wmp11-script-s1.txt

pause
