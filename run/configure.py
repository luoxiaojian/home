import os

tsDir="/home/makai/algorithm/itachi/taskset_300_2/"
fileList=os.listdir(tsDir)
lDir="/home/makai/algorithm/m2/run300_2/log300_2/"
#fileList.remove(".")
#fileList.remove("..")

exe="./run"

fout=open("go.sh", "w")

for fname in fileList:
    lfname=fname+".log"
    cmd=exe+" "+tsDir+fname+" "+lDir+lfname+"\n"
    fout.write(cmd)


fout.flush()
fout.close()
