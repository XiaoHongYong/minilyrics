# Build MiniLyrics Solution 

import sys, os, subprocess, time, shutil 


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


#
# Update SVN build number
#

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
shutil.CopyFile

#
# Copy Zipped source code and .pdb, installation pack to a Release dir.
#

