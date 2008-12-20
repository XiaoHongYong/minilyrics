call "G:\Program Files\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"

sdk\bin\Flex++.exe -oss.yy.cpp SkinScript.l

sdk\bin\bison.exe -d -oSkinScript.tab.cpp SkinScript.y

cl ss.yy.cpp SkinScript.tab.cpp ss.exe

ss.exe < wmp11-script.txt

pause
