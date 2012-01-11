/*****************************************************************************
 * Project: RooFit                                                           *
 *                                                                           *
  * This code was autogenerated by RooClassFactory                            * 
 *****************************************************************************/

#ifndef _heavyflavorpdf_cxx_
#define _heavyflavorpdf_cxx_

#if !defined(__CINT__) || defined(__MAKECINT__)

#include <math.h> 
#include "TMath.h" 
#include "Riostream.h" 

#include "RooAbsReal.h" 
#include "RooAbsCategory.h" 
#include "RooAbsPdf.h"
#include "RooCachedPdf.h"
#include "RooRealProxy.h"
#include "RooCategoryProxy.h"
#include "RooAbsReal.h"
#include "RooAbsCategory.h"
#endif

class HeavyFlavorPDF : public RooAbsPdf {
public:
  HeavyFlavorPDF() { setJetMultiplicity(2); } ; 
  HeavyFlavorPDF(const char *name, const char *title,
		 RooAbsReal& _bmult,
		 RooAbsReal& _r,
		 RooAbsReal& _eb,
		 RooAbsReal& _eq,
		 RooAbsReal& _fcorrect,
		 RooAbsReal& _fttbar,
		 RooAbsReal& _fsingletop,
		 RooAbsReal& _jetocc);
  
  HeavyFlavorPDF(const HeavyFlavorPDF& other, const char* name=0) ;
  virtual TObject* clone(const char* newname) const { return new HeavyFlavorPDF(*this,newname); }
  inline virtual ~HeavyFlavorPDF() { }

  void setJetMultiplicity(int _jmult)  { jmult=_jmult; }

protected:

  RooRealProxy bmult ;
  RooRealProxy r ;
  RooRealProxy eb ;
  RooRealProxy eq ;
  RooRealProxy fcorrect;
  RooRealProxy fttbar;
  RooRealProxy fsingletop;
  RooRealProxy jetocc;
  Int_t jmult;
  Double_t evaluate() const ;
  inline void cacheValues(int nBtags, Double_t prob) const
    {
      lastBinCached_=nBtags;
      lastValueCached_=prob;
    }
  Double_t _evaluate(int jetMult, int nBtags) const;
  Double_t _evaluateKernel(int nMisassignments, int nBtags, bool doNorm=true) const;
  Double_t _normalizedAcceptance(Double_t R) const;
  
private:

  //has to be declared as mutable as it is modified inside a const function
  //the method has to be const because RooFit requires it to be that way
  //cf. http://stackoverflow.com/questions/105014/c-mutable-keyword
  mutable Int_t lastBinCached_;
  mutable Double_t lastValueCached_;

  ClassDef(HeavyFlavorPDF,1) // Your description goes here...
};
 
#endif
