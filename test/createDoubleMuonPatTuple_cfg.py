import os,sys
runOnMC=False
trigFilter='mumu'
cfgFile=os.path.expandvars('${CMSSW_BASE}/src/LIP/Top/test/createPatTuple_cfg.py')
from CMGTools.HtoZZ2l2nu.localPatTuples_cff import configureFromCommandLine
castorDir, outFile, inputList = configureFromCommandLine()
execfile(cfgFile)
