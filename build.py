################################################################################
#
# Build MiniLyrics and ZikiPlayer
#
# Cmd line:
# build.py [product] [action]
# action = /clean, /rebuild, or /build, default for /build
# product = MiniLyrics or ZikiPlayer, if none product is specified, targets are all product
#
################################################################################

import sys, os, subprocess, time, shutil, re


def ErrExit(errStr):
	print("Error occured, while execute the following command")
	print(errStr)
	sys.exit(-1)


def Execute(cmd):
	print(cmd)
	p = subprocess.Popen(cmd)
	p.wait()


def ExecuteOnErrQuit(cmd):
	print(cmd)
	p = subprocess.Popen(cmd)
	p.wait()
	if p.returncode:
		ErrExit(cmd)

def GetFeildIntValueInFile(fileName, fieldPattern):
	fi = open(fileName)
	ftext = fi.read();
	ptn = re.compile(fieldPattern)
	m = ptn.search(ftext)
	if m is None:
		ErrExit(r"Can't find pattern " + fieldName + " in file: " + fileName)
	return int(m.groups()[0])


# Update SVN build number
ExecuteOnErrQuit("SubWCRev.exe . MPShared\\AutoBuild.in MPShared\\AutoBuild.h")

# Read Version info and build number
MLMajorVer = GetFeildIntValueInFile("MPShared\\MLVersion.cpp", r"ML_MAJOR_VERSION\s*=\s*(\d+)")
MLMinorVer = GetFeildIntValueInFile("MPShared\\MLVersion.cpp", "ML_MINOR_VERSION\s*=\s*(\d+)")
BuildNum = GetFeildIntValueInFile("MPShared\\AutoBuild.h", "#define\s*BUILD_NUM\s*(\d+)")

PackReleaseDir = "..\\Release\\" + str(BuildNum) + "\\"
print(r"Release dir: " + PackReleaseDir)

#
# Build solution
#
DevEnvExe = "devenv.com"
BuildAction = "/build"
TargetML = 0
TargetZP = 0

cmdLines = set(sys.argv)

# Get action from command line
if "/rebuild" in cmdLines: BuildAction = "/rebuild"
elif "/clean" in cmdLines: BuildAction = "/clean"

# Get target product from command line
if "MiniLyrics" in cmdLines: TargetML = 1
if "ZikiPlayer" in cmdLines: TargetZP = 1
if TargetML == 0 and TargetZP == 0:
	TargetML = 1
	TargetZP = 1

# Build MiniLyrics
if TargetML:
	MLSolutions = [
		[ " ZikiPlayer.sln", "Unicode Release|Win32", " /project MiniLyrics6" ],
		[ " PluginAP\\PluginAP.sln", "Release|Win32", "" ],
		[ " PluginBsp\\PluginBsp.sln", "Release|Win32", "" ],
		[ " PluginHMM\\PluginHMM.sln", "Release|Win32", "" ],
		[ " pluginiTunes\\pluginiTunes.sln", "Release|Win32", "" ],
		[ " PluginJetaudio\\PluginJetaudio.sln", "Release|Win32", "" ],
		[ " PluginMJ\\PluginMJ.sln", "Release|Win32", "" ],
		[ " PluginQCDPlayer\\PluginQCDPlayer.sln", "Release|Win32", "" ],
		[ " PluginRealOne\\PluginRealOne.sln", "Release|Win32", "" ],
		[ " PluginVLC\\PluginVLC.sln", "Release|Win32", "" ],
		[ " PluginWinamp5\\PluginWinamp5.sln", "Release|Win32", "" ],
		[ " PluginWMP\\PluginWMP.sln", "Release|Win32", "" ],
		[ " PluginWMPVis\\PluginWMPVis.sln", "Release|Win32", "" ],
		[ " PluginXMP\\PluginXMP.sln", "Release|Win32", "" ],
		[ " PluginYme\\PluginYme.sln", "Release|Win32", "" ],
		[ " MLInstallOptions\\io.sln", "Release|Win32", "" ]
	]

	for solution in MLSolutions:
		cmd = DevEnvExe + solution[0] + " " + BuildAction + " \"" + solution[1] + "\"" + solution[2]
		#ExecuteOnErrQuit(cmd)


# Build ZikiPlayer
if TargetZP:
	ZPSolutions = [
		[ " ZikiPlayer.sln", "Unicode Release|Win32", " /project MPlayer" ],
	]

	for solution in ZPSolutions:
		cmd = DevEnvExe + solution[0] + " " + BuildAction + " \"" + solution[1] + "\"" + solution[2]
		#ExecuteOnErrQuit(cmd)




#
# Create installation pack
#
shutil.copy("bin\\MiniLyrics.dll", "Install_ML\\Share\\MiniLyrics.dll")


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
#ExecuteOnErrQuit(ZipSrcCmd)
print(ZipSrcCmd)

# Compress ZikiPlayer plugin source code to MPlayer-Plugins.7z
ZipIncludeExt = [ "", "cpp", "h", "vcproj", "sln", "c" ]
ZipPluginCmd = "7z a -t7z " + "MPlayer-Plugins.7z -r " + " MPlayer-Plugins\*.".join(ZipIncludeExt)
#ExecuteOnErrQuit(ZipPluginCmd)
print(ZipPluginCmd)


try:
	shutil.copy("bin\\MiniLyrics.pdb", PackReleaseDir)
	shutil.copy("Inst-ML\\MiniLyrics.exe", PackReleaseDir)
	shutil.move("src.7z", PackReleaseDir)
	shutil.move("MPlayer-Plugins.7z", PackReleaseDir)
except (IOError, os.error) as why:
	ErrExit(str(why))
