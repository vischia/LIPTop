import os,sys
runOnMC=True
trigFilter=''
cfgFile=os.path.expandvars('${CMSSW_BASE}/src/LIP/Top/test/createPatTuple_cfg.py')
from CMGTools.HtoZZ2l2nu.localPatTuples_cff import configureFromCommandLine
castorDir, outFile, inputList = configureFromCommandLine()
print castorDir
print outFile
print inputList
execfile(cfgFile)
