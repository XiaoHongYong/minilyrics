# Build MiniLyrics Solution 

import sys, os, subprocess, time, shutil, re


def ErrExit(errStr):
	print("Error occurs, while execute the following command")
	print(errStr)
	sys.exit(-1)


def Execute(cmd):
	print(cmd)
	p = subprocess.Popen(cmd)
	p.wait()
	if p.returncode:
		ErrExit(cmd)


def ExecuteOnErrQuit(cmd):
	print(cmd)
	p = subprocess.Popen(cmd)
	p.wait()

def GetFeildIntValueInFile(fileName, fieldName):
	fi = open(fileName)
	ftext = fi.read();
	startPos = ftext.find(fieldName)
	if startPos == -1:
		ErrExit("Can't find field " + fieldName + " in file: " + fileName)
	numberRe = re.compile("[0-9]+")
	number = numberRe.findall(ftext[startPos : ])
	if len(number) == 0:
		ErrExit("A number should be followed after field " + fieldName + " in file: " + fileName)

	return int(number[0])


# Update SVN build number
#Execute("SubWCRev.exe . MPShared\\AutoBuild.in MPShared\\AutoBuild.h")

# Read Version info and build number
MLMajorVer = GetFeildIntValueInFile("MPShared\\MLVersion.cpp", "ML_MAJOR_VERSION = ")
MLMinorVer = GetFeildIntValueInFile("MPShared\\MLVersion.cpp", "ML_MINOR_VERSION = ")
BuildNum = GetFeildIntValueInFile("MPShared\\AutoBuild.h", "#define BUILD_NUM ")

PackReleaseDir = "..\\Release\\" + str(BuildNum) + "\\"

#
# Build solution
#
DevEnvExe = "devenv.com"
BuildConfig = os.environ["CFG"]

#cmd = DevEnvExe + " ZikiPlayer.sln /build " + BuildConfig + "|Win32\" /project ML6"
#Execute(cmd)



#
# Create installation pack
#
try:
	shutil.copy("bin\\MiniLyrics.dll", "Inst-ML\\Share\\MiniLyrics.dll")
except (IOError, os.error) as why:
	ErrExit(str(why))


#
# Copy Zipped source code and .pdb, installation pack to a Release dir.
#

# Compress source code to src.7z
ZipExcludeExt = [ "", "rar", "ncb", "svn", "aps", "opt", "plg", "user", "scc", 
	"tmp", "obj", "idb", "pdb", "lib", "pch", "res", "exp", "stt", "zip", 
	"gz", "suo" ]
ZipIncludeDir = ["",  "base", "gfx", "id3lib", "mllib", "mlprotocol", "mplayer",
	"mplayerlib", "ourmfclib", "skin", "sqlite_cpp", "utils", "widget", 
	"MPShared", "ML6", "ImageLib", "SearchArith", "MLyricsOutPlugin", 
	"MLPluginLib", "skinscript", "MLServer",
	"MPlayer-Plugins\\MDAPE", "MPlayer-Plugins\\MDFlac",
	"MPlayer-Plugins\\MDMP4", "MPlayer-Plugins\\MDVorbis", 
	"MPlayer-Plugins\\MDWMA2" ]
	#mpskineditor ourmfclib GridCtrl   Test        
ZipOutFile = "src.7z"
ZipSrcCmd = "7z a -t7z " + ZipOutFile + " -i!".join(ZipIncludeDir) + " -xr!*.".join(ZipExcludeExt)
#Execute(ZipSrcCmd)
print(ZipSrcCmd)

# Compress ZikiPlayer plugin source code to MPlayer-Plugins.7z
ZipIncludeExt = [ "", "cpp", "h", "vcproj", "sln", "c" ]
ZipPluginCmd = "7z a -t7z " + "MPlayer-Plugins.7z -r " + " MPlayer-Plugins\*.".join(ZipIncludeExt)
#Execute(ZipPluginCmd)
print(ZipPluginCmd)


try:
	shutil.copy("bin\\MiniLyrics.pdb", PackReleaseDir)
	shutil.copy("Inst-ML\\MiniLyrics.exe", PackReleaseDir)
	shutil.move("src.7z", PackReleaseDir)
	shutil.move("MPlayer-Plugins.7z", PackReleaseDir)
except (IOError, os.error) as why:
	ErrExit(str(why))
