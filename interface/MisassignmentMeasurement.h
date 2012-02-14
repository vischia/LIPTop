#ifndef misassignmentmeasurement_hh
#define misassignmentmeasurement_hh

#include <iostream>
#include <string>
#include <vector>

#include "LIP/Top/interface/EventSummaryHandler.h"

#include "CMGTools/HtoZZ2l2nu/interface/setStyle.h"
#include "CMGTools/HtoZZ2l2nu/interface/SelectionMonitor.h"

#include "TFile.h"
#include "TRandom2.h"
#include "TH1D.h"

#include "CondFormats/JetMETObjects/interface/JetResolution.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"

/**
   @short 
   takes care of measuring the missassignments from the M_{lj} spectrum
   while bookeeping the statistics for the pseudo-experiments
 */
class MisassignmentMeasurement
{
 public:
  
  /**
     @short CTOR
   */
  MisassignmentMeasurement() : nMeasurements(0)  { bookMonitoringHistograms();  }
    
    void initJetUncertainties(TString uncFileName,  TString ptFileName, TString etaFileName,  TString phiFileName);

  
  /**
     @short DTOR
   */

  ~MisassignmentMeasurement()  { }

  TRandom2 rndGen;
  SelectionMonitor controlHistos;

  /**
     @short books the histograms
  */
  void bookMonitoringHistograms();

  /**
     @short fixes extremities, averages quantities, fits the bias and pulls
   */
  void finishMonitoringHistograms();

  /**
     @short dumps current version of the histograms to file
   */
  void saveMonitoringHistograms();

  /**
     @short resets the histogram contents
   */
  void resetHistograms(bool fullReset=false);

  /**
     @short returns a collection of randomly rotated leptons
  */
  top::PhysicsObjectLeptonCollection randomlyRotate( top::PhysicsObjectLeptonCollection &leptons, top::PhysicsObjectJetCollection &jets);

  /**
     @short runs the measurement
  */
  void measureMisassignments(top::EventSummaryHandler &evHandler, double mcut=190, double minMlj=40, bool isData=false, int jetBin=-1, TString syst="");
  
  /**
     @short getters
  */
  std::vector<double> getCorrectPairsFraction(TString cat="all")
    {
      std::vector<double> res(2,0);
      res[0]=fCorrectPairsEst[cat];
      res[1]=fCorrectPairsEstErr[cat];
      return res;
    }
  std::vector<double> getAlpha(TString cat="all") 
    {
      std::vector<double> res(2,0);
      res[0]=alphaEst[cat];
      res[1]=alphaEstErr[cat];
      return res;
    }
  void setBiasCorrections(TString cat,double val) {bias[cat]=val; }
  double getNorm(TString cat="all") { return kNorm[cat]; }
  std::vector<double> getTrueCorrectPairsFraction(TString cat="all",bool sigOnly=false) 
    {
      std::vector<double> res(2,0);
      TH1 *h=controlHistos.getHisto(sigOnly ? "sigonlytruefcorr" : "truefcorr",cat);
      res[0]=h->GetMean();
      res[1]=h->GetMeanError();
      return res;
   }

  void fitCurrentModelsToData();

 private:

  int nMeasurements;
  std::map<TString,double> kNorm;
  std::map<TString,double> fCorrectPairsEst, fCorrectPairsEstErr;
  std::map<TString,double> alphaEst,alphaEstErr;

  std::map<TString,double> bias;

  JetResolution *stdEtaResol_, *stdPhiResol_, *stdPtResol_;
  JetCorrectionUncertainty *jecUnc_;

};


#endif
