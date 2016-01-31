import os

tsDir="/home/makai/algorithm/itachi/taskset_900/"
fileList=os.listdir(tsDir)
lDir="/home/makai/algorithm/m4/uedf/log900/"
#fileList.remove(".")
#fileList.remove("..")

exe="./algo"

fout=open("go.sh", "w")

for fname in fileList:
    lfname=fname+".log"
    cmd=exe+" "+tsDir+fname+" "+lDir+lfname+"\n"
    fout.write(cmd)


fout.flush()
fout.close()
