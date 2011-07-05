#include <iostream>
#include <boost/shared_ptr.hpp>
#include <fstream>

#include "LIP/Top/interface/EventSummaryHandler.h"
#include "LIP/Top/interface/KinResultsHandler.h"
#include "LIP/Top/interface/KinAnalysis.h"
#include "CMGTools/HtoZZ2l2nu/interface/setStyle.h"
#include "CMGTools/HtoZZ2l2nu/interface/ObjectFilters.h"

#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "FWCore/PythonParameterSet/interface/MakeParameterSets.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "TSystem.h"
#include "TFile.h"
#include "TChain.h"
#include "TChainElement.h"
#include "TCanvas.h"
#include "TString.h"
#include "TDirectory.h"

using namespace std;

//
int main(int argc, char* argv[])
{
  // load framework libraries
  gSystem->Load( "libFWCoreFWLite" );
  AutoLibraryLoader::enable();

  //check arguments
  if ( argc < 2 ) {
    std::cout << "Usage : " << argv[0] << " parameters_cfg.py" << std::endl;
    return 0;
  }
  
  //configure                                                                                                                                                                                                                 
  const edm::ParameterSet &runProcess = edm::readPSetsFrom(argv[1])->getParameter<edm::ParameterSet>("runProcess");
  TString evurl=runProcess.getParameter<std::string>("input");
  TString outUrl=runProcess.getParameter<std::string>("outdir");
  TString kindir = runProcess.getParameter<std::string>("kindir");
  bool isMC = runProcess.getParameter<bool>("isMC");
  TString dirname = runProcess.getParameter<std::string>("dirName");

  //book histos
  std::map<TString, TH1 *> results;
  TString cats[]={"all","ee","mumu","emu","etau","mutau"};
  size_t ncats=sizeof(cats)/sizeof(TString);
  TString subcats[]={"eq0btags","eq1btags","geq2btags"};
  size_t nsubcats=sizeof(subcats)/sizeof(TString);
  for( size_t icat=0; icat<ncats; icat++)
    {
      results[cats[icat]+"_njets"] = (TH1*) formatPlot( new TH1F (cats[icat]+"_njets", ";Jets;Events", 6, 0.,6.), 1,1,1,20,0,true,true,1,1,1);
      results[cats[icat]+"_btags"] = (TH1*) formatPlot( new TH1F (cats[icat]+"_btags", ";b-tag multiplicity;Events", 6, 0.,6.), 1,1,1,20,0,true,true,1,1,1);
      for(int ibin=1; ibin<=results[cats[icat]+"_njets"]->GetXaxis()->GetNbins(); ibin++)
	{
	  TString label(""); label += ibin-1; 
	  if(ibin==results[cats[icat]+"_njets"]->GetXaxis()->GetNbins()) label ="#geq" + label;
	  results[cats[icat]+"_njets"]->GetXaxis()->SetBinLabel(ibin,label + " jets");
	  results[cats[icat]+"_btags"]->GetXaxis()->SetBinLabel(ibin,label + " btags");
	}

      results[cats[icat]+"_leadjet"] = (TH1*) formatPlot( new TH1F (cats[icat]+"_leadjet", "; Leading jet p_{T} [GeV/c]; Events / (10 GeV/c)", 25, 0.,250.), 1,1,1,20,0,true,true,1,1,1);
      results[cats[icat]+"_subleadjet"] = (TH1*) formatPlot( new TH1F (cats[icat]+"_subleadjet", "; Sub-leading jet p_{T} [GeV/c]; Events / (10 GeV/c)", 25, 0.,250.), 1,1,1,20,0,true,true,1,1,1);
      results[cats[icat]+"_leadlepton"] = (TH1*) formatPlot( new TH1F (cats[icat]+"_leadlepton", "; Leading lepton p_{T} [GeV/c]; Events / (10 GeV/c)", 25, 0.,250.), 1,1,1,20,0,true,true,1,1,1);
      results[cats[icat]+"_subleadlepton"] = (TH1*) formatPlot( new TH1F (cats[icat]+"_subleadlepton", "; Sub-leading lepton p_{T} [GeV/c]; Events / (10 GeV/c)", 25, 0.,250.), 1,1,1,20,0,true,true,1,1,1);
      results[cats[icat]+"_met"] = (TH1*) formatPlot( new TH1F (cats[icat]+"_met", "; #slash{E}_{T} [GeV/c]; Events / (10 GeV/c)", 40, 0.,400.), 1,1,1,20,0,true,true,1,1,1);
      results[cats[icat]+"_ht"] = (TH1*) formatPlot( new TH1F (cats[icat]+"_ht", "; #sum_{jets} [GeV/c]; Events / (20 GeV/c)",100, 0.,2000.), 1,1,1,20,0,true,true,1,1,1);
      results[cats[icat]+"_st"] = (TH1*) formatPlot( new TH1F (cats[icat]+"_st", "; #sum_{leptons,E_{T}^{miss}} [GeV/c]; Events / (20 GeV/c)",100, 0.,2000.), 1,1,1,20,0,true,true,1,1,1);
      results[cats[icat]+"_sumpt"] = (TH1*) formatPlot( new TH1F (cats[icat]+"_sumpt", "; #sum_{leptons} p_{T} [GeV/c]; Events / (20 GeV/c)",100, 0.,2000.), 1,1,1,20,0,true,true,1,1,1);
      results[cats[icat]+"_htlep"] = (TH1*) formatPlot( new TH1F (cats[icat]+"_htlep", "; #sum_{jets,leptons,E_{T}^{miss}} [GeV/c]; Events / (20 GeV/c)",100, 0.,2000.), 1,1,1,20,0,true,true,1,1,1);
      results[cats[icat]+"_mtop"] = (TH1*) formatPlot( new TH1F (cats[icat]+"_mtop", "; m_{Top} [GeV/c^{2}]; Events / (10 GeV/c^{2})", 50, 0.,500.), 1,1,1,20,0,true,true,1,1,1);
      for(size_t isubcat=0; isubcat<nsubcats; isubcat++)
	{
	  results[cats[icat]+subcats[isubcat]+"_mtop"] = (TH1*) formatPlot( new TH1F (cats[icat]+subcats[isubcat]+"_mtop", "; m_{Top} [GeV/c^{2}]; Events / (10 GeV/c^{2})", 50, 0.,500.), 1,1,1,20,0,true,true,1,1,1);
	  results[cats[icat]+subcats[isubcat]+"_dilmass"] = (TH1F*)formatPlot( new TH1F (cats[icat]+subcats[isubcat]+"_dilmass", "; Mass(l,l') [GeV/c^{2}]; Events", 100, 0.,500.), 1,1,1,20,0,true,true,1,1,1);
	}
      results[cats[icat]+"_afb"] = (TH1*) formatPlot( new TH1F (cats[icat]+"_afb", "; #Delta #eta(t,#bar{t}); Events / (0.1)", 100, -5.,5.), 1,1,1,20,0,true,true,1,1,1);
      results[cats[icat]+"_mttbar"] = (TH1*) formatPlot( new TH1F (cats[icat]+"_mttbar", "; Mass(t,#bar{t}) [GeV/c^{2}]; Events / (20 GeV/c^{2})", 100, 0.,2000.), 1,1,1,20,0,true,true,1,1,1);
    
      results[cats[icat]+"_mtopvsdilmass"] = (TH1F*)formatPlot( new TH2F (cats[icat]+"_mtopvsdilmass", "; m_{Top} [GeV/c^{2}]; Mass(l,l') [GeV/c^{2}]; Events", 100, 0.,500.,100, 0.,500.), 1,1,1,20,0,true,true,1,1,1);
      results[cats[icat]+"_mtopvsmlj"] = (TH1F*)formatPlot( new TH2F (cats[icat]+"_mtopvsmlj", "; m_{Top} [GeV/c^{2}]; Mass(l,j) [GeV/c^{2}]; Events", 100, 0.,500.,100, 0.,500.), 1,1,1,20,0,true,true,1,1,1);
      results[cats[icat]+"_mtopvsmet"] = (TH1F*)formatPlot( new TH2F (cats[icat]+"_mtopvsmet", "; m_{Top} [GeV/c^{2}]; #slash{E}_{T} [GeV/c]; Events", 100, 0.,500.,50, 0.,250.), 1,1,1,20,0,true,true,1,1,1);

      results[cats[icat]+"_mtopvsmttbar"] = (TH1F*)formatPlot( new TH2F (cats[icat]+"_mtopvsmttbar", "; m_{Top} [GeV/c^{2}]; Mass(t,#bar{t}) [GeV/c^{2}]; Events", 100, 0.,500.,100, 0.,2000.), 1,1,1,20,0,true,true,1,1,1);
      results[cats[icat]+"_mtopvsafb"] = (TH1F*)formatPlot( new TH2F (cats[icat]+"_mtopvsafb", "; m_{Top} [GeV/c^{2}]; #Delta #eta(t,#bar{t}); Events", 100, 0.,500.,100, -5.,5.), 1,1,1,20,0,true,true,1,1,1);
      results[cats[icat]+"_mttbarvsafb"] = (TH1F*)formatPlot( new TH2F (cats[icat]+"_mttbarvsafb", "; Mass(t,#bar{t}) [GeV/c^{2}];#Delta #eta(t,#bar{t}); Events", 100, 0.,2000.,100,-5.,5.), 1,1,1,20,0,true,true,1,1,1);
    }


  //fix entries flag
  ofstream *outf=0;
  if(!isMC) outf=new ofstream("highmassevents.txt",ios::app);

  //process events file
  gSystem->ExpandPathName(evurl);
  TFile *evfile = TFile::Open(evurl);
  if(evfile==0) return -1;
  if(evfile->IsZombie()) return -1;
  EventSummaryHandler evSummaryHandler;
  if( !evSummaryHandler.attachToTree( (TTree *)evfile->Get(dirname) ) ) 
    {
      evfile->Close();
      return -1;
    }  
  TTree *evTree=evSummaryHandler.getTree();

  //process kin file
  TString kinUrl(evurl);
  kinUrl.ReplaceAll(".root","/"+kindir);
  gSystem->ExpandPathName(kinUrl);
  cout << "Kin results from " << kinUrl << " to be processed with summary from " << evurl << endl;
  KinResultsHandler kinHandler;
  kinHandler.init(kinUrl,false);
  TChain *t=kinHandler.getResultsChain();
 
  //loop over events
  std::map<TString,int> selEvents;
  for (int inum=0; inum < evTree->GetEntriesFast(); ++inum)
  {
    evTree->GetEvent(inum);
    EventSummary_t &ev = evSummaryHandler.getEvent();
    TString key(""); key+= ev.run; key+="-"; key += ev.lumi; key+="-"; key += ev.event;
    selEvents[key]=inum;
  }

  //loop over kin results
  int nresults(0);
  for (int inum=0; inum < t->GetEntries(); ++inum){
    t->GetEvent(inum);
 
    //get original event
    Int_t irun,ievent,ilumi;
    kinHandler.getEventInfo(irun,ievent,ilumi);

    TString key("");  key+= irun; key+="-"; key += ilumi;  key+="-"; key += ievent;

    if(selEvents.find(key)==selEvents.end()) continue;
    nresults++;
    evTree->GetEntry( selEvents[key] );

    //get event summary
    EventSummary_t &ev = evSummaryHandler.getEvent();
    std::vector<TString> categs;
    categs.push_back("all");
    if(ev.cat==dilepton::MUMU)  categs.push_back("mumu");
    if(ev.cat==dilepton::EE)  categs.push_back("ee");
    if(ev.cat==dilepton::EMU) categs.push_back("emu");
    if(ev.cat==dilepton::ETAU) categs.push_back("etau");
    if(ev.cat==dilepton::MUTAU) categs.push_back("mutau");
    
    //get particles from the event
    int njets(0),nbtags(0);
    KinCandidateCollection_t leptons, jets, mets,vtx;
    for(Int_t ipart=0; ipart<ev.nparticles; ipart++)
      {
	TLorentzVector p4(ev.px[ipart],ev.py[ipart],ev.pz[ipart],ev.en[ipart]);
	if(isnan(p4.Pt()) || isinf(p4.Pt())) continue;
	switch( ev.id[ipart] )
	  {
	  case 0:
	    mets.push_back( KinCandidate_t(p4,p4.Pt()) );
            break;
          case 1:
            jets.push_back( KinCandidate_t(p4, ev.info1[ipart]) );
	    njets++;
	    if(ev.info1[ipart]>1.7) nbtags++;
            break;
	  case 500:
	    vtx.push_back( KinCandidate_t(p4,p4.Pt()) );
	    break;
          default:
            leptons.push_back( KinCandidate_t(p4,ev.id[ipart]) );
            break;
	  }
      }
    sort(leptons.begin(),leptons.end(),KinAnalysis::sortKinCandidates);
    sort(jets.begin(),jets.end(),KinAnalysis::sortKinCandidates);
    sort(mets.begin(),mets.end(),KinAnalysis::sortKinCandidates);

    TString subcat="eq0btags";
    if(nbtags==1) subcat="eq1btags";
    if(nbtags>=2) subcat="geq2btags";

    //get the combination preferred by KIN
    TH1F *h1=kinHandler.getHisto("mt",1), *h2=kinHandler.getHisto("mt",2);
    h1->Rebin(2); h2->Rebin(2);
    Int_t icomb=(h1->Integral()< h2->Integral())+1;
    TH1F *mpref=kinHandler.getHisto("mt",icomb);
    double mtop = kinHandler.getMPVEstimate(mpref) [1];
    TH1F *mttbarpref=kinHandler.getHisto("mttbar",icomb);
    double mttbar = kinHandler.getMPVEstimate(mttbarpref)[1];
    TH1F *afbpref=kinHandler.getHisto("afb",icomb);
    double afb = kinHandler.getMPVEstimate(afbpref)[1];
    
    //compute dilepton/dijet invariant mass
    TLorentzVector dil = leptons[0].first+leptons[1].first;
    float dilmass = dil.M();
    if(fabs(dilmass-91)<15 && (ev.cat==dilepton::EE || ev.cat==dilepton::MUMU))continue;
    double ptlep1(max(leptons[0].first.Pt(),leptons[1].first.Pt())), ptlep2(min(leptons[0].first.Pt(),leptons[1].first.Pt()));    
    TLorentzVector dij = jets[0].first+jets[1].first;
    float mjj=dij.M();
    double ptjet1(max(jets[0].first.Pt(),jets[1].first.Pt())), ptjet2(min(jets[0].first.Pt(),jets[1].first.Pt()));
    
    //get the lepton-jet pairs
    TLorentzVector lj1=leptons[0].first+jets[icomb==1?0:1].first;
    TLorentzVector lj2=leptons[1].first+jets[icomb==1?1:0].first;

    //ht
    double ht(0);
    for(size_t ijet=0; ijet<jets.size(); ijet++) ht += jets[ijet].first.Pt();
    double sumptlep(leptons[0].first.Pt()+leptons[1].first.Pt()+mets[0].first.Pt());
    double st(sumptlep+mets[0].first.Pt());
    double htlep(st+ht);

    //fill histos
    float weight = ev.weight;

    for(std::vector<TString>::iterator cIt = categs.begin(); cIt != categs.end(); cIt++)
      {
	results[*cIt+"_njets"]->Fill(njets,weight);
	results[*cIt+"_btags"]->Fill(nbtags,weight);
	results[*cIt+"_leadjet"]->Fill(ptjet1,weight);
	results[*cIt+"_subleadjet"]->Fill(ptjet2,weight);
	results[*cIt+"_leadlepton"]->Fill(ptlep1,weight);
	results[*cIt+"_subleadlepton"]->Fill(ptlep2,weight);
	results[*cIt+"_met"]->Fill(mets[0].first.Pt(),weight);
	results[*cIt+"_ht"]->Fill(ht,weight);
	results[*cIt+"_st"]->Fill(st,weight);
	results[*cIt+"_sumpt"]->Fill(sumptlep,weight);
	results[*cIt+"_htlep"]->Fill(htlep,weight);
	if(mtop>0)
	  {
	    results[*cIt+"_mtop"]->Fill(mtop,weight);
	    results[*cIt+subcat+"_mtop"]->Fill(mtop,weight);
	    results[*cIt+subcat+"_dilmass"]->Fill(dilmass,weight);
	    ((TH2F *)results[*cIt+"_mtopvsdilmass"])->Fill(mtop,dilmass,weight);
	    ((TH2F *)results[*cIt+"_mtopvsmlj"])->Fill(mtop,lj1.M(),weight);
	    ((TH2F *)results[*cIt+"_mtopvsmlj"])->Fill(mtop,lj2.M(),weight);
	    ((TH2F *)results[*cIt+"_mtopvsmet"])->Fill(mtop,mets[0].first.Pt(),weight);
	    ((TH2F *)results[*cIt+"_mtopvsmttbar"])->Fill(mtop,mttbar,weight);
	    ((TH2F *)results[*cIt+"_mtopvsafb"])->Fill(mtop,afb,weight);
	    ((TH2F *)results[*cIt+"_mttbarvsafb"])->Fill(mttbar,afb,weight);
	    results[*cIt+"_afb"]->Fill(afb);
	    results[*cIt+"_mttbar"]->Fill(mttbar);
	  }
      }

    //for data only
    if (!isMC && mtop>350) 
      *outf << "| " << irun << ":" << ilumi << ":" << ievent 
	    << " | " << categs[1] 
	    << " | " << mtop 
	    << " | " << mttbar 
	    << " | " << ptlep1 << ";" << ptlep2  << " | " << dilmass
	    << " | " << ptjet1 << ";" << ptjet2  << " | " << mjj
	    << " | " << mets[0].first.Pt() << " | " << htlep << endl; 
  }
  kinHandler.end();

  if(!isMC) 
    {
      outf->close(); 
      delete outf;
    }


  //if MC: rescale to number of selected events and to units of pb
  cout << "Selected " << selEvents.size() << " found " << nresults << " kin results " << endl;
  if(isMC && nresults)
    {
      double scaleFactor=selEvents.size()/nresults;
      TString tag=gSystem->BaseName(evurl);
      tag.ReplaceAll(".root","");
      TH1F *cutflowH = (TH1F *) evfile->Get("evAnalyzer/"+tag+"/cutflow");
      if(cutflowH)
	{
	  float cnorm=cutflowH->GetBinContent(1);
	  if(cnorm>0) scaleFactor/=cnorm;
	}
      cout << scaleFactor << endl;
      for(std::map<TString,TH1 *>::iterator hIt = results.begin(); hIt != results.end(); hIt++) hIt->second->Scale(scaleFactor);
    }


  //save to file
  gSystem->Exec("mkdir -p " + outUrl);
  outUrl += "/";
  outUrl += gSystem->BaseName(evurl);
  TFile *file=TFile::Open(outUrl, "recreate");

  TDirectory *baseOutDir=file->mkdir("localAnalysis");
  for( size_t icat=0; icat<ncats; icat++)
    {
      baseOutDir->cd();
      if(icat) baseOutDir->mkdir( cats[icat] )->cd();
      for(std::map<TString,TH1 *>::iterator hIt = results.begin(); hIt != results.end(); hIt++) 
	{
	  if(!hIt->first.BeginsWith(cats[icat])) continue;
	  hIt->second->Write();
	}
    }

  file->Close(); 
}  
