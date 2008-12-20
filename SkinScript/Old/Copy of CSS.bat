call "G:\Program Files\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"

sdk\bin\Flex++.exe -oss.yy.cpp SkinScript.l

cl ss.yy.cpp

ss.yy.exe < wmp11-script.txt > 1.txt

pause
