
import sys
reload(sys)
sys.setdefaultencoding('utf8')

import subprocess
import os
import commands

#os.system('cmd1 && cmd2')
cmd1 = "cd ../"
cmd2 = "ls"
cmd = cmd1 + " && " + cmd2

#如下两种都可以运行
subprocess.Popen(cmd, shell=True)
subprocess.call(cmd,shell=True)