#ifndef hfcmeasurement_hh
#define hfcmeasurement_hh

#include "CMGTools/HtoZZ2l2nu/interface/setStyle.h"
#include "CMGTools/HtoZZ2l2nu/interface/SelectionMonitor.h"

#include "LIP/Top/interface/EventSummaryHandler.h"
#include "LIP/Top/interface/HeavyFlavorPDF.h"

#include "TFile.h"
#include "TRandom2.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLorentzVector.h"

#include "RooGaussian.h"
#include "RooRealVar.h"
#include "RooFormulaVar.h"
#include "RooProdPdf.h"
#include "RooDataHist.h"
#include "RooAddition.h"
#include "RooPlot.h"
#include "RooMinuit.h"
#include "RooAddPdf.h"
#include "RooFitResult.h"

//
#define MAXJETMULT 12
#define MAXCATEGORIES 12
enum JetMultBins{BIN_2=0,BIN_3, BIN_4};
struct CombinedHFCModel_t
{
  RooArgSet pdfSet,constrPDFSet;
  RooRealVar *bmult, *r, *lfacceptance;
  RooRealVar *abseb,*sfeb,*sfeb_mean_constrain,*sfeb_sigma_constrain,*diff_sfeb[MAXCATEGORIES];
  RooFormulaVar *eb,*diff_eb[MAXCATEGORIES];
  RooGaussian *sfeb_constrain;
  RooRealVar *abseq,*sfeq,*sfeq_mean_constrain,*sfeq_sigma_constrain;
  RooFormulaVar *eq;
  RooGaussian *sfeq_constrain;
  RooRealVar *jetocc[MAXCATEGORIES];
  RooRealVar *fcorrect[MAXCATEGORIES],*fcorrect_mean_constrain[MAXCATEGORIES],*fcorrect_sigma_constrain[MAXCATEGORIES];
  RooGaussian *fcorrect_constrain[MAXCATEGORIES];
  RooRealVar *fttbar[MAXCATEGORIES],*fttbar_mean_constrain[MAXCATEGORIES],*fttbar_sigma_constrain[MAXCATEGORIES];
  RooGaussian *fttbar_constrain[MAXCATEGORIES];
  RooRealVar *fsingletop[MAXCATEGORIES],*fsingletop_mean_constrain[MAXCATEGORIES],*fsingletop_sigma_constrain[MAXCATEGORIES];
  RooGaussian *fsingletop_constrain[MAXCATEGORIES];
};

//
class HFCMeasurement
{
 public:

  enum FitTypes { FIT_R, FIT_EB, FIT_R_AND_EB, FIT_R_AND_XSEC, FIT_EB_AND_XSEC, FIT_EB_AND_EQ };
  SelectionMonitor controlHistos_;    
  CombinedHFCModel_t model;
  
  /**
     @short CTOR
   */
  HFCMeasurement(int fitType=0, int maxJets=3,float smR=1.0) : 
    isInit_(false), fitType_(fitType), maxJets_(maxJets),  smR_(smR), nMeasurements_(0)  
    {
      bookMonitoringHistograms();
      switch(fitType_)
	{
	case FIT_EB:          fitTypeTitle_="#varepsilon_{b}";                    fitTypeName_="effb";                        break;
	case FIT_R_AND_EB:    fitTypeTitle_="R vs #varepsilon_{b}";               fitTypeName_="rvseffb";                     break;
	case FIT_R_AND_XSEC:  fitTypeTitle_="R vs #sigma";                        fitTypeName_="rvssigma";                    break;
	case FIT_EB_AND_XSEC: fitTypeTitle_="#varepsilon_{b} vs #sigma";          fitTypeName_="effbvssigma";                 break;
	case FIT_EB_AND_EQ:   fitTypeTitle_="#varepsilon_{b} vs #varepsilon_{q}"; fitTypeName_="effbvseffq";                  break;
	default:              fitTypeTitle_="R";                                  fitTypeName_="r";           fitType_=FIT_R; break;
	}
    }
    
    /**
       @short DTOR
    */
    ~HFCMeasurement() { }
        
    /**
       @short steer the fit
    */
    void fitHFCtoEnsemble(top::EventSummaryHandler &evHandler);
    
    /**
       @short setters for parameters
    */
    void setStandardModelR(float r=1.0) { smR_=r; }

    /**
       @short configuration of the b-tag algorithm
    */
    void configureBtagAlgo(TString btagAlgo,double cut)
    {
      btagAlgo_ = btagAlgo;
      algoCut_  = cut;
    }
    
    /**
       @short set MC expected efficiency and data-driven measurement of the scale-factor
     */
    void setBtagEfficiency(double eff, double sfactor, double sfactorUnc)
    {
      effb_   = eff;
      sfb_    = sfactor;
      sfbUnc_ = sfactorUnc;
    } 

    /**
       @short set MC expected efficiency and data-driven measurement of the scale-factor
    */
    void setMistagEfficiency(double eff, double sfactor, double sfactorUnc)
    {
      effq_   = eff;
      sfq_    = sfactor;
      sfqUnc_ = sfactorUnc;
    } 

    /**
       @short event modeling per category: used also to instantiate the categories in the fit
     */
    void setSelectionFractions(double fcorrect,   double fcorrectunc, 
			       double fttbar,     double fttbarunc,
			       double fsingletop, double fsingletopunc,
			       int jetBin=0,      TString dilChannel="emu")
    {
      TString tag(dilChannel); tag+=jetBin;      
      fcorrect_[tag]      = fcorrect;
      fcorrectUnc_[tag]   = fcorrectunc;
      fttbar_[tag]        = fttbar;
      fttbarUnc_[tag]     = fttbarunc;
      fsingletop_[tag]    = fsingletop;
      fsingletopUnc_[tag] = fsingletopunc;
      categoryKeys_.insert(tag);
    }

    /**
       @short dump fitter configuration
     */
    void printConfiguration(std::ostream &os);

    /**
       @short save results
    */
    void saveMonitoringHistograms(TString tag);

 private:

    /**
       @short reads the current configuration and instantiates the PDFs for the fit
     */
    void initHFCModel();

    /**
       @short resets the current model values to the default ones
     */
    void resetModelValues();

    /**
       @short steers the fit 
     */
    void runHFCFit();

    /**
       @short steers the fit with jet pT categories (work in progress)
     */
    void runHFCDiffFit(TString dilCat);

    /**
       @short monitoring histogram handling 
     */
    void bookMonitoringHistograms();
    void resetHistograms();

    //internal parameters
    bool isInit_;
    int fitType_, maxJets_;
    TString fitTypeTitle_, fitTypeName_;
    double smR_;
    TString btagAlgo_;    
    double algoCut_;
    Float_t  effb_, sfb_, sfbUnc_, effq_, sfq_, sfqUnc_;
    std::set<TString> categoryKeys_;
    std::map<TString, Float_t> fcorrect_,  fcorrectUnc_, fttbar_, fttbarUnc_,  fsingletop_, fsingletopUnc_;
    int nMeasurements_;
};


#endif
