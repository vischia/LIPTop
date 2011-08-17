#!/usr/bin/env python

import os,sys
import json
import getopt
from math import sqrt,pow

"""
Gets the value of a given item
(if not available a default value is returned)
"""
def getByLabel(desc,key,defaultVal=None) :
    try :
        return desc[key]
    except KeyError:
        return defaultVal
    
#print usage
def usage() :
    print ' '
    print 'runHFCMeasurement.py [options]'
    print '  -l : luminosity (pb)'
    print '  -j : json file containing the samples'
    print '  -i : event summary file'
    print '  -n : number of ensembles to test'
    print '  -b : jet bin (2=2 jets exclusive, 3=3 jets inclusive, otherwise all inclusive)' 
    print '  -p : plot only'
    exit(-1)

#parse the options
try:
    # retrive command line options
    shortopts  = "b:l:j:i:n:p:h?"
    opts, args = getopt.getopt( sys.argv[1:], shortopts )
except getopt.GetoptError:
    # print help information and exit:
    print "ERROR: unknown options in argument %s" % sys.argv[1:]
    usage()
    sys.exit(1)

#configure
lumi=215.3
samplesDB=''
ifile=''
nensemble=1
plotOnly=False
jetbin=0
for o,a in opts:
    if o in("-?", "-h"):
        usage()
        sys.exit(0)
    elif o in('-l'): lumi=float(a)
    elif o in('-j'): samplesDB = a
    elif o in('-i'): ifile=a
    elif o in('-n'): nensemble=int(a)
    elif o in('-p'): plotOnly = (a=='True')
    elif o in('-b'): jetbin=int(a)
    
# load macros
import ROOT
ROOT.gSystem.Load('${CMSSW_BASE}/lib/${SCRAM_ARCH}/libLIPTop.so')
from ROOT import HFCMeasurement, MisassignmentMeasurement, EventSummaryHandler, getNewCanvas, showPlotsAndMCtoDataComparison, setStyle, formatForCmsPublic, formatPlot

if(not plotOnly) :
    inF=ROOT.TFile.Open(ifile)

    jsonFile = open(samplesDB,'r')
    procList=json.load(jsonFile,encoding='utf-8').items()

    #run over sample
    evHandler       = EventSummaryHandler()
    misMeasurement  = MisassignmentMeasurement()
    ensembleInfo = '<big><b>Summary of ensemble tests (data/MC) for events with'
    if(jetbin==2):   ensembleInfo += '= 2 jets'
    elif(jetbin==3): ensembleInfo += '&gt;= 3 jets'
    else :           ensembleInfo += '&gt;= 2 jets'
    ensembleInfo += '</b></big>'
    print '[HFCmeasurement]',
    for ipe in xrange(0,nensemble+1) :
        
        #progress bar
        print '.',
        sys.stdout.flush()
        
        ensembleHandler = EventSummaryHandler()
        ensembleInfo  += '<table>'
        ensembleInfo += '<tr><th><b>Info for '
        if(ipe==0) : ensembleInfo += 'data'
        else       : ensembleInfo += ' ensemble #' + str(ipe)
        ensembleInfo += '</b></th></tr>'
        
        for proc in procList :

            #run over processes
            id=0
            for desc in proc[1] :
                isdata = getByLabel(desc,'isdata',False)
                
                if(ipe>0 and isdata) : continue
                if(ipe==0 and not isdata) : continue
            
                #run over items in process
                data = desc['data']
                for d in data :
                    tag = getByLabel(d,'dtag','')
 
                    #get tree of events from file
                    t=inF.Get(tag+'/data')
                    
                    try :
                        t.GetEntriesFast()
                    except:
                        continue
            
                    attResult=evHandler.attachToTree(t)
                    nevtsSel = evHandler.getEntries()
                    if(attResult is False) : continue
                    
                    #clone (will use the same address as the original tree)
                    id=id+1
                    if(id==1):
                        ROOT.gROOT.cd()
                        ensembleHandler.initTree(t.CloneTree(0), False)
                        ensembleHandler.getTree().SetDirectory(0)
                    
                    #generate number of events for ensemble
                    nevts=nevtsSel
                    nevtsExpected=nevts
                    if(ipe>0):
                        evHandler.getEntry(0)
                        nevtsExpected=lumi*(evHandler.evSummary_.weight)*nevtsSel
                        nevts = int(ROOT.gRandom.Poisson( nevtsExpected ))

                    genEvts=[]
                    for ievt in xrange(0,nevts) :
                        rndEvt=ievt
                        if(ipe>0):
                            rndEvt = int(ROOT.gRandom.Uniform(0,nevtsSel))
                            if( rndEvt in genEvts ) : continue
                        evHandler.getEntry(rndEvt)
                        ensembleHandler.fillTree()
                        genEvts.append(rndEvt)

            ensembleInfo += '<tr><td><b>' + tag + '<b></td></tr>'
            if(ipe>0) : ensembleInfo += '<tr><td>&lt;N<sub>expected</sub>&gt;=' + str(nevtsExpected) + '</td></tr>'
            ensembleInfo += '<tr><td>&lt;N<sub>generated</sub>&gt;=' + str(nevts) + '</td></tr>'
            #if(ipe>0) : ensembleInfo += '<tr><td><small>' + str(genEvts) + '</small></td></tr>'
                
            #take control of the filled tree now
            ensembleHandler.attachToTree( ensembleHandler.getTree() )
            misMeasurement.measureMisassignments( ensembleHandler, 180, 40, (ipe==0), jetbin )

            #print info
            categories=['all', 'emu','ll']
            for cat in categories : 
                fcorrectTrue  = misMeasurement.getTrueCorrectPairsFraction(cat)
                fcorrectEst   = misMeasurement.getCorrectPairsFraction(cat)
                fcorrect      = fcorrectEst[0]
                fcorrectErr   = fcorrectEst[1]
                kNorm         = misMeasurement.getNorm(cat)
                ensembleInfo += '<tr><td><i>' + cat + ' events</i></td></tr>'
                ensembleInfo += '<tr><td>k-factor=' + str(kNorm) + '</td></tr>'
                ensembleInfo += '<tr><td>f<sub>correct</sub>=' + str(fcorrect) + "+/-" + str(fcorrectErr)
                if(ipe>0) : ensembleInfo += ' (MC truth=' + str(fcorrectTrue)+ ')'
                ensembleInfo += '</td><tr>'
                ensembleInfo += '<tr><td></td></tr>'
            
            ensembleHandler.getTree().Delete("all")
        
    misMeasurement.saveMonitoringHistograms()
    inF.Close()

    #save ensemble info to file
    fout = open('ensemble_info.html', 'w')
    fout.write(ensembleInfo)
    fout.close()
                

#display results
plotF = ROOT.TFile.Open("MisassignmentMeasurement.root")
cats=['all','emu','ll']
setStyle()
cnv = getNewCanvas('misc','misc',False)
cnv.SetWindowSize(1500,500)
cnv.Divide(3,1)
cnv2 =getNewCanvas('misresc','misresc',False)
cnv2.SetWindowSize(1500,500)
cnv2.Divide(3,1)
icnv=0
for c in cats:
    prefix=''
    if(c!='all') : prefix=c+'_'
    icnv = icnv+1

    #inclusive distributions
    cnv.cd(icnv)
    spimpose = ROOT.TList()
    mcmodelH=plotF.Get('localAnalysis/'+c+'/'+prefix+'avgwrongmodelmlj')
    mcmodelH.SetTitle('MC model')
    formatPlot(mcmodelH,8,9,2,1,0,True,False,8,8,8)
    datamodelH =plotF.Get('localAnalysis/'+c+'/'+prefix+'datawrongmodelmlj')
    datamodelH.SetTitle('data model')
    formatPlot(datamodelH,1,9,2,1,0,True,False,1,1,1)
    spimpose.Add(mcmodelH)
    spimpose.Add(datamodelH)
    
    stack    = ROOT.TList()
    mcWrongH=plotF.Get('localAnalysis/'+c+'/'+prefix+'avgwrongmlj')
    formatPlot(mcWrongH,809,1,1,0,1001,True,False,1,809,809)
    mcWrongH.SetTitle('Wrong assignments')
    mcCorrectH=plotF.Get('localAnalysis/'+c+'/'+prefix+'avgcorrectmlj')
    formatPlot(mcCorrectH,614,1,1,0,1001,True,False,1,614,614)
    mcCorrectH.SetTitle('Correct assignments')
    stack   .Add(mcWrongH)
    stack   .Add(mcCorrectH)

    data    = ROOT.TList()
    dataH=plotF.Get('localAnalysis/'+c+'/'+prefix+'datainclusivemlj')
    dataH.SetDirectory(0)
    dataH.SetTitle('data')
    data    .Add(dataH)
    
    pad=cnv.cd(icnv)
    leg=showPlotsAndMCtoDataComparison(pad,stack,spimpose,data)
    subpad1=pad.cd(1)    
    subpad1.SetLogx()
    subpad2=pad.cd(2)
    subpad2.SetLogx()
    if(icnv==1) : formatForCmsPublic(pad.cd(1),leg,'CMS preliminary, #sqrt{s}=7 TeV, #int L=%3.0f pb^{-1}' % lumi ,2)
    else        : leg.Delete()


    #subtracted distributions
    cnv2.cd(icnv)

    spimpose2    = ROOT.TList()
    mcSubtractedH=plotF.Get('localAnalysis/'+c+'/'+prefix+'avginclusivemlj').Clone('mcres')
    mcSubtractedH.Add(mcmodelH,-1)
    mcSubtractedH.SetTitle('MC residual')
    formatPlot(mcSubtractedH,8,9,2,1,0,True,False,8,8,8)
    spimpose2.Add(mcSubtractedH)

    data2 = ROOT.TList()
    dataSubtractedH = dataH.Clone('datares')
    dataSubtractedH.Add(datamodelH,-1)
    dataSubtractedH.SetTitle('data residual')
    dataSubtractedH.SetDirectory(0)
    data2.Add(dataSubtractedH)
    
    stack2 = ROOT.TList()
    stack2.Add(mcCorrectH)
    
    pad2=cnv2.cd(icnv)
    leg=showPlotsAndMCtoDataComparison(pad2,stack2,spimpose2,data2)
    subpad12=pad2.cd(1)
    subpad12.SetLogx()
    subpad22=pad2.cd(2)
    subpad22.SetLogx()
    if(icnv==1) : formatForCmsPublic(pad2.cd(1),leg,'CMS preliminary, #sqrt{s}=7 TeV, #int L=%3.0f pb^{-1}' % lumi ,2)
    else        : leg.Delete()


    fcorrH=plotF.Get('localAnalysis/'+c+'/'+prefix+'fcorr')
    truefCorrH=plotF.Get('localAnalysis/'+c+'/'+prefix+'truefcorr')
    avgFcorr=fcorrH.GetMean()
    fCorrErr=fcorrH.GetRMS()
    avgFcorrTrue=truefCorrH.GetMean()
    fCorrTrueErr=truefCorrH.GetRMS()
    fCorrBias=avgFcorr-avgFcorrTrue
    fCorrBiasErr=sqrt(pow(fCorrErr,2)+pow(fCorrTrueErr,2))
    print c + ' f_{correct}=' + str(avgFcorr) + ' +/- ' + str(fCorrErr) + ' <f_{correct}^{MC}>=' + str(avgFcorrTrue) + ' +/- ' + str(fCorrTrueErr) + ' bias=' + str(fCorrBias) + ' +/- ' + str(fCorrBiasErr)
    
cnv.cd()
cnv.Draw()
cnv.Modified()
cnv.Update()
cnv.SaveAs('mljwithmodel.C')

cnv2.cd()
cnv2.Draw()
cnv2.Modified()
cnv2.Update()
cnv2.SaveAs('mljsubtracted.C')

    
raw_input(' *** Any key to end')    
plotF.Close()

