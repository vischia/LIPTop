 /***************************************************************************** 
  * Project: RooFit                                                           * 
  *                                                                           * 
  * This code was autogenerated by RooClassFactory                            * 
  *****************************************************************************/ 

 // Your description goes here... 

#include "LIP/Top/interface/HeavyFlavorPDF.h" 

 ClassImp(HeavyFlavorPDF) 

 HeavyFlavorPDF::HeavyFlavorPDF(const char *name, const char *title, 
				RooAbsReal& _bmult,
				RooAbsReal& _r,
				RooAbsReal& _eb,
				RooAbsReal& _eq,
				RooAbsReal& _fcorrect,
				RooAbsReal& _fttbar,
				RooAbsReal& _fsingletop,
				RooAbsReal& _jetocc):
   RooAbsPdf(name,title), 
   bmult("bmult","bmult",this,_bmult),
   r("r","r",this,_r),
   eb("eb","eb",this,_eb),
   eq("eq","eq",this,_eq),
   fcorrect("fcorrect","fcorrect",this,_fcorrect),
   fttbar("fttbar","fttbar",this,_fttbar),
   fsingletop("fsingletop","fsingletop",this,_fsingletop),
   jetocc("jetocc","jetocc",this,_jetocc),
   lastBinCached_(-1),
   lastValueCached_(-1)
 { 
   //default value
   setJetMultiplicity(2);
 }


HeavyFlavorPDF::HeavyFlavorPDF(const HeavyFlavorPDF& other, const char* name) :  
  RooAbsPdf(other,name), 
  bmult("bmult",this,other.bmult),
  r("r",this,other.r),
  eb("eb",this,other.eb),
  eq("eq",this,other.eq),
  fcorrect("fcorrect",this,other.fcorrect),
  fttbar("fttbar",this,other.fttbar),
  fsingletop("fsingletop",this,other.fsingletop),
  jetocc("jetocc",this,other.jetocc),
  jmult(other.jmult),
  lastBinCached_(-1),
  lastValueCached_(-1)
{ 

 } 


//
Double_t HeavyFlavorPDF::evaluate() const 
{ 
  Int_t nBtags=int(bmult);
  if(nBtags<0) return 0;
  if(nBtags>jmult) return 0;
  
  //check if this b-tag bin was already requested
  //  if(lastBinCached_==nBtags) return lastValueCached_;
  
  //otherwise update for the new value
  Double_t prob=_evaluate(jmult,nBtags);
  //cacheValues(nBtags,prob);

  //evaluate the probability functions
  return prob;
} 


// 
Double_t HeavyFlavorPDF::_evaluate(int jetMult,int nBtags) const
{
  //compute probability
  Double_t prob(0);
  switch(jetMult)
    {
      //2 jets: use the kernel functions
    case 2:
      {

	//update the values for the alphas
	int nevents = jetocc;
	int npairs=2*jmult*nevents;
	
	if((2+fsingletop)*fttbar*nevents==0) return 0;
	double alpha=(fcorrect*npairs)/((2+fsingletop)*fttbar*nevents);
	if(alpha<0 || alpha>1) {
	  //cout<< "[HeavyFlavorPDF::_evaluate] unphysical value for alpha: " << alpha << endl; 
	  return 0; 
	}
	
	double alpha2=pow(alpha,2)*fttbar;
	double alpha1=(2*alpha*(1-alpha)+alpha*fsingletop)*fttbar;
	double alpha0=1-alpha2-alpha1;
	
	//compute probability
	prob =  alpha2*_evaluateKernel(0,nBtags);
	prob += alpha1*_evaluateKernel(1,nBtags);
	prob += alpha0*_evaluateKernel(2,nBtags);
	break;
      }
	  
      //>2 jets evaluate recursively until 2 jets are found
    default:
      {
	if(nBtags==0)
	  {
	    prob = _evaluate(jetMult-1,0)*(1-eq);
	  }
	else if(nBtags==jetMult)
	  {
	    prob = _evaluate(jetMult-1,nBtags-1)*eq;
	  }
	else
	  {
	    prob =  _evaluate(jetMult-1,nBtags-1)*eq;
	    prob += _evaluate(jetMult-1,nBtags)*(1-eq);
	  }
	break;
      }
    }
  
  //the probability
  return prob;
}
    

//
Double_t HeavyFlavorPDF::_evaluateKernel(int nMisassignments, int nBtags, bool doNorm) const
{
  Double_t prob(0);
  
  //check ranges
  if(nMisassignments<0 || nMisassignments>2) return prob;
  if(nBtags<0 || nBtags>2) return prob;

  switch(nMisassignments)
    {
      //pure signal
    case 0:
      {
	switch(nBtags)
	  {
	  case 0:
	    {
	      prob =  TMath::Power(r*(1-eb),2)*_normalizedAcceptance(1);
	      prob += 2*r*(1-r)*(1-eb)*(1-eq)*_normalizedAcceptance(0.5);
	      prob += TMath::Power((1-r)*(1-eq),2)*_normalizedAcceptance(0);
	      break;
	    }
	  case 1:
	    {
	      prob =  2*TMath::Power(r,2)*(1-eb)*eb*_normalizedAcceptance(1);
	      prob += 2*r*(1-r)*((1-eb)*eq+eb*(1-eq))*_normalizedAcceptance(0.5);
	      prob += 2*TMath::Power((1-r),2)*eq*(1-eq)*_normalizedAcceptance(0);
	      break;
	    }
	  case 2:
	    {
	      prob =  TMath::Power(r*eb,2)*_normalizedAcceptance(1);
	      prob += 2*r*(1-r)*eb*eq*_normalizedAcceptance(0.5);
	      prob += TMath::Power((1-r)*eq,2)*_normalizedAcceptance(0);
	      break;
	    }
	  }
	break;
      }
      
      //signal with 1 misassignment
    case 1:
      {
	switch(nBtags)
	  {
	  case 0:
	    {
	      prob =  TMath::Power(r,2)*(1-eb)*(1-eq)*_normalizedAcceptance(1);
	      prob += r*(1-r)*((1-eb)+(1-eq))*(1-eq)*_normalizedAcceptance(0.5);
	      prob += TMath::Power((1-r)*(1-eq),2)*_normalizedAcceptance(0);
	      break;
	    }
	  case 1:
	    {
	      prob =  TMath::Power(r,2)*(eb*(1-eq)+(1-eb)*eq)*_normalizedAcceptance(1);
	      prob += r*(1-r)*(eb*(1-eq)+(1-eb)*eq+2*eq*(1-eq))*_normalizedAcceptance(0.5);
	      prob += TMath::Power((1-r),2)*eq*(1-eq)*_normalizedAcceptance(0);
	      break;
	    }
	  case 2:
	    {
	      prob =  TMath::Power(r,2)*eb*eq*_normalizedAcceptance(1);
	      prob += r*(1-r)*((eb+eq)*eq)*_normalizedAcceptance(0.5);
	      prob += TMath::Power((1-r)*eq,2)*_normalizedAcceptance(0);
	      break;
	    }
	  }
	break;
      }
      
      //pure background + totally misreconstructed signal
    case 2:
      {
	switch(nBtags)
	  {
	  case 0:
	    {
	      prob = TMath::Power(1-eq,2);
	      break;
	    }
	  case 1:
	    {
	      prob = 2*eq*(1-eq);
	      break;
	    }
	  case 2:
	    {
	      prob = TMath::Power(eq,2);
	      break;
	    }
	  }
	break;
      }
    }
  

  Double_t norm(1.0);
  if(doNorm)
    {
      norm = _evaluateKernel(nMisassignments,0,false)
	+ _evaluateKernel(nMisassignments,1,false)
	+ _evaluateKernel(nMisassignments,2,false);
    }	
  prob /= norm;	
  
  //the result
  return prob;
}

//
Double_t HeavyFlavorPDF::_normalizedAcceptance(Double_t R)  const
{
  return 1.0;//R*(1.0-lfacceptance)+1.0;
}
