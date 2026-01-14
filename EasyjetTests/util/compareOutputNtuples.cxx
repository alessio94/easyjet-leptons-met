#include <TH1.h>
#include <TFile.h>
#include <TChain.h>
#include <TTree.h>
#include <TString.h>
#include <TBranch.h>
#include <TObjArray.h>
#include <TDirectory.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TLegend.h>
#include <TSystem.h>
#include <TEnv.h>
#include <TROOT.h>

#include <vector>
#include <set>
#include <iostream>
#include <string>

using namespace std;

void drawDiff(TH1* newHisto, TH1* refHisto, TH1* diff, const TString &branchname, const TString &outdir){

  TCanvas *cc = new TCanvas(branchname, branchname, 1200, 600);
  cc->Divide(3,1);
  cc->cd(1);
  newHisto->SetXTitle(branchname);
  newHisto->SetYTitle("entries (new)");
  newHisto->Draw();
  newHisto->SetLineWidth(2);
  cc->cd(2);
  refHisto->SetXTitle(branchname);
  refHisto->SetYTitle("entries (ref)");
  refHisto->SetLineWidth(2);
  refHisto->Draw();
  cc->cd(3);
  diff->SetXTitle(branchname);
  diff->SetYTitle("entries (new-ref)");
  diff->SetLineWidth(2);
  diff->Draw();
  cc->SaveAs(outdir+"/"+branchname+".png");
  cc->SaveAs(outdir+"/"+branchname+".pdf");
  delete cc;

}

void drawDiffSame(TH1* newHisto, TH1* refHisto, TH1* diff, const TString& branchname, const TString& outdir){

  TCanvas *cc = new TCanvas(branchname, branchname, 1000, 600);
  cc->Divide(2,1);
  cc->cd(1);
  newHisto->SetXTitle(branchname);
  newHisto->SetYTitle("entries");
  newHisto->SetLineWidth(2);
  newHisto->SetLineColor(1);
  newHisto->Draw();
  refHisto->SetLineColor(2);
  refHisto->SetLineWidth(2);
  refHisto->SetLineStyle(2);
  refHisto->Draw("sames");

  float ymax = (newHisto->GetMaximum()>refHisto->GetMaximum()) ? newHisto->GetMaximum() : refHisto->GetMaximum();
  newHisto->SetMaximum(ymax*1.1);
  
  TLegend *leg = new TLegend(0.6,0.75,0.75,0.85);
  leg->SetBorderSize(0);
  leg->SetFillColor(0);
  leg->AddEntry(newHisto,"new","l");
  leg->AddEntry(refHisto,"ref","l");
  leg->Draw();

  cc->cd(2);
  diff->SetXTitle(branchname);
  diff->SetYTitle("entries (new-ref)");
  diff->SetLineWidth(2);
  diff->Draw();
  
  cc->SaveAs(outdir+"/"+branchname+".png");
  cc->SaveAs(outdir+"/"+branchname+".pdf");
  delete leg;
  delete cc;

}

void dumpSet(const set<TString> &tset){
  for(auto a : tset) cout << a << endl;
}

void compareTrees(TTree* refTree, TTree* newTree, bool useSameBinning, const TString& outdir){

  cout << "############ comparing " << refTree->GetName() <<" tree #################" << endl;
  int maxBranches=1000;
  int maxPlots=100;
 
  int refEntries = refTree->GetEntries();
  int newEntries = newTree->GetEntries();

  if(refEntries == newEntries){
    cout << " ref and new trees have the same number of entries = " << newEntries << endl;
  }
  else{
    cout << " different number of entries new=" <<newEntries << " ref=" << refEntries << endl;
    cout << "---> all branches will be different " << endl;
  }

  set<TString> addedBranchList;
  set<TString> removedBranchList;
  set<TString> diffBranchList;
  set<TString> okBranchList;

  TObjArray *newBranchList = newTree->GetListOfBranches();
  TObjArray *refBranchList = refTree->GetListOfBranches();

  int nplots=0;
  int ncbranches=0;
  for(int i=0; i<newBranchList->GetSize(); ++i){
    
    TString branchName = newBranchList->At(i)->GetName();
    
    if(refTree->FindBranch(branchName)){

      /// if at some points we want to add cuts
      TString cut ="";

      //// if at some point we want to compare specific branchs
      //if(!branchName.BeginsWith(""))continue;
      //if(!branchName.EndsWith(""))continue;
      //if(!branchName.EqualTo(""))continue;

      if(ncbranches >= maxBranches) continue;
      ++ncbranches;
      
      TString newHistoName= branchName+"_new";
      TString refHistoNameSB= branchName+"_refSB";
      TString refHistoName= branchName+"_ref";

      newTree->Draw(branchName+">>"+newHistoName,cut,"goff");
      void *newHistoTmp=0;
      gDirectory->GetObject(newHistoName,newHistoTmp);
      TH1* newHisto  = static_cast<TH1*>(newHistoTmp);
    
      TH1* refHistoSB = (TH1*)newHisto->Clone(refHistoNameSB);
      refTree->Draw(branchName+">>"+refHistoNameSB,cut,"goff");

      refTree->Draw(branchName+">>"+refHistoName,cut,"goff");
      void *refHistoTmp=0;
      gDirectory->GetObject(refHistoName,refHistoTmp);
      TH1 *refHisto  = static_cast<TH1*>(refHistoTmp);

      TH1* diff = (TH1*)refHistoSB->Clone(branchName+"_difference");
      diff->Scale(-1);
      diff->Add(newHisto);
	
      bool OK = true;
      for(int i=0; i<diff->GetNbinsX()+2; ++i){
	if(diff->GetBinContent(i)){
	  OK=false;
	  break;
	}
      }

      if(OK){
	okBranchList.insert(branchName);
      }
      else{
	diffBranchList.insert(branchName);
	if(nplots<maxPlots){
	  ++nplots;
	  if(useSameBinning){
	    drawDiffSame(newHisto, refHistoSB, diff, branchName, outdir);
	  }
	  else{
	    drawDiff(newHisto, refHisto, diff, branchName, outdir);
	  }
	}
      }

      gDirectory->Remove(newHisto);
      gDirectory->Remove(refHistoSB);
      gDirectory->Remove(refHisto);
      gDirectory->Remove(diff);
      
      newHisto->SetDirectory(0);
      refHistoSB->SetDirectory(0);
      refHisto->SetDirectory(0);
      diff->SetDirectory(0);
      
      delete newHisto;
      delete refHistoSB;
      delete refHisto;
      delete diff;

    }
    else{
      addedBranchList.insert(branchName);
    }
  }
  cout << endl;

  for(int i=0; i<refBranchList->GetSize(); ++i){
    TBranch * refBranch = dynamic_cast<TBranch*>(refBranchList->At(i));
    TString branchName = refBranch->GetName();
    if(!newTree->FindBranch(branchName)){
      removedBranchList.insert(branchName);
    }
  }

  cout << "List of OK branches:" << endl;
  cout << "--------------------" << endl;
  dumpSet(okBranchList);
  cout <<"----> " << okBranchList.size() << " branches are OK" << endl;

  cout << "List of add branches:" << endl;
  cout << "---------------------" << endl;
  dumpSet(addedBranchList);
  cout <<"----> " << addedBranchList.size() << " branches are added" << endl;

  cout << "List of removed branches:" << endl;
  cout << "-------------------------" << endl;
  dumpSet(removedBranchList);
  cout <<"----> " << removedBranchList.size() << " branches are removed" << endl;

  cout << "List of modified branches:" << endl;
  cout << "----------------------" << endl;
  dumpSet(diffBranchList);
  cout <<"----> " << diffBranchList.size() << " branches are different" << endl;

  cout << "Max branches to compare = " << maxBranches << endl;
  cout << "Max allowed plots = " << maxPlots << endl;
  
  std::cout << "Reference has " << refBranchList->GetSize() << " branches "
	    << " and new has " << newBranchList->GetSize()<< " branches "
	    << std::endl;
  

  
  cout << "############ end comparing " << refTree->GetName() <<" tree #################" << endl;

}
// stop clang check here. Not using const & since we want this function to be callable from command line using root
void compareOutputNtuples(TString refFileName, // NOLINT
			  TString newFileName, // NOLINT
			  TString treeName="AnalysisMiniTree", // NOLINT
			  TString outdir="compare_output", // NOLINT
			  bool drawsame=true){

  gSystem->Exec("rm -rf "+outdir);
  gSystem->Exec("mkdir -p "+outdir);

  //gEnv->SetValue("Root.ErrorIgnoreLevel=Info");
  gEnv->SetValue("Root.ErrorIgnoreLevel=Warning");

  gStyle->SetOptStat(111111);

  TFile* nr = TFile::Open(refFileName);
  TTree* ch1 = (TTree*)nr->Get(treeName);
  
  TFile* nf = TFile::Open(newFileName);
  TTree* ch2 = (TTree*)nf->Get(treeName);

  compareTrees(ch1, ch2, drawsame, outdir);

}


#ifndef __CLING__

void usage(const string &ss){
  cout << "Usage: " << ss << " reffilename newfilename [treeName] [outdir] " << endl;
  cout << "default treeName=\"AnalysisMiniTree\" outdir=\"compare_output\" " << endl;
}

int main(int argc, char **argv){

  if(argc<3){
    usage(argv[0]);
    return 1;
  }

  TString refFileName = argv[1];
  TString newFileName = argv[2];

  TString treeName="AnalysisMiniTree";
  TString outdir="compare_output";

  if(argc>3)treeName = argv[3];
  if(argc>4)outdir = argv[4];


  compareOutputNtuples(refFileName, newFileName, treeName, outdir);

  return 0;

}

#endif
