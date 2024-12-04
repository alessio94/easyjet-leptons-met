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

#include <vector>
#include <set>
#include <iostream>
#include <string>

using namespace std;

void drawDiff(TH1* newHisto, TH1* oldHisto, TH1* diff, const TString &branchname, const TString &outdir){

  TCanvas *cc = new TCanvas(branchname, branchname, 1200, 600);
  cc->Divide(3,1);
  // diff->SetXTitle(branchname);
  cc->cd(1);
  newHisto->SetXTitle(branchname);
  newHisto->SetYTitle("entries (new)");
  newHisto->Draw();
  newHisto->SetLineWidth(2);
  cc->cd(2);
  oldHisto->SetXTitle(branchname);
  oldHisto->SetYTitle("entries (old)");
  oldHisto->SetLineWidth(2);
  oldHisto->Draw();
  cc->cd(3);
  diff->SetXTitle(branchname);
  diff->SetYTitle("entries (new-old)");
  diff->SetLineWidth(2);
  diff->Draw();
  cc->SaveAs(outdir+"/"+branchname+".png");
  cc->SaveAs(outdir+"/"+branchname+".pdf");

}

void drawDiffSame(TH1* newHisto, TH1* oldHisto, TH1* diff, const TString& branchname, const TString& outdir){

  TCanvas *cc = new TCanvas(branchname, branchname, 1000, 600);
  cc->Divide(2,1);
  cc->cd(1);
  newHisto->SetXTitle(branchname);
  newHisto->SetYTitle("entries");
  newHisto->SetLineWidth(2);
  newHisto->SetLineColor(1);
  newHisto->Draw();
  oldHisto->SetLineColor(2);
  oldHisto->SetLineWidth(2);
  oldHisto->SetLineStyle(2);
  oldHisto->Draw("sames");

  float ymax = (newHisto->GetMaximum()>oldHisto->GetMaximum()) ? newHisto->GetMaximum() : oldHisto->GetMaximum();
  newHisto->SetMaximum(ymax*1.1);
  
  TLegend *leg = new TLegend(0.6,0.75,0.75,0.85);
  leg->SetBorderSize(0);
  leg->SetFillColor(0);
  leg->AddEntry(newHisto,"new","l");
  leg->AddEntry(oldHisto,"old","l");
  leg->Draw();

  cc->cd(2);
  diff->SetXTitle(branchname);
  diff->SetYTitle("entries (new-old)");
  diff->SetLineWidth(2);
  diff->Draw();
  
  cc->SaveAs(outdir+"/"+branchname+".png");
  cc->SaveAs(outdir+"/"+branchname+".pdf");

}

void dumpSet(const set<TString> &tset){
  for(auto a : tset) cout << a << endl;
}

void compareTrees(TTree* oldTree, TTree* newTree, bool useSameBinning, const TString& outdir){

  cout << "############ comparing " << oldTree->GetName() <<" tree #################" << endl;

  int oldEntries = oldTree->GetEntries();
  int newEntries = newTree->GetEntries();

  if(oldEntries == newEntries){
    cout << " old and new trees have the same number of entries = " << newEntries << endl;
  }
  else{
    cout << " different number of entries new=" <<newEntries << " old=" << oldEntries << endl;
    cout << "---> all branches will be different " << endl;
  }

  set<TString> addedBranchList;
  set<TString> removedBranchList;
  set<TString> diffBranchList;
  set<TString> okBranchList;
  

  TObjArray *newBranchList = newTree->GetListOfBranches();

  for(int i=0; i<newBranchList->GetSize(); ++i){

    TBranch *newBranch = dynamic_cast<TBranch*>(newBranchList->At(i));
    TString branchName = newBranch->GetName();

    if(oldTree->FindBranch(branchName)){

      /// if at some points we want to add cuts
      TString cut ="";

      //// if at some point we want to compare specific branchs
      //if(!branchName.BeginsWith(""))continue;
      //if(!branchName.EndsWith(""))continue;
      //if(!branchName.EqualTo(""))continue;

      TString newHistoName= branchName+"_new";
      TString oldHistoNameSB= branchName+"_oldSB";
      TString oldHistoName= branchName+"_old";

      newTree->Draw(branchName+">>"+newHistoName,cut,"goff");
      void *newHistoTmp=0;
      gDirectory->GetObject(newHistoName,newHistoTmp);
      TH1 *newHisto  = static_cast<TH1*>(newHistoTmp);

      TH1* oldHistoSB = (TH1*)newHisto->Clone(oldHistoNameSB);
      oldTree->Draw(branchName+">>"+oldHistoNameSB,cut,"goff");

      oldTree->Draw(branchName+">>"+oldHistoName,cut,"goff");
      void *oldHistoTmp=0;
      gDirectory->GetObject(oldHistoName,oldHistoTmp);
      TH1 *oldHisto  = static_cast<TH1*>(oldHistoTmp);

      TH1* diff = (TH1*)oldHistoSB->Clone(branchName+"_difference");
      diff->Scale(-1);
      diff->Add(newHisto);
	
      bool OK = true;
      for(int i=0; i<diff->GetNbinsX()+2; ++i){
	if(diff->GetBinContent(i)){
	  OK=false;
	  break;
	}
      }

      if(OK ){
	okBranchList.insert(branchName);
	//drawDiffSame(newHisto, oldHistoSB, diff, branchName);
	continue;
      }

      diffBranchList.insert(branchName);

      if(useSameBinning){
	drawDiffSame(newHisto, oldHistoSB, diff, branchName, outdir);
      }
      else{
	drawDiff(newHisto, oldHisto, diff, branchName, outdir);
      }

    }
    else{
      addedBranchList.insert(branchName);
    }

  }


  TObjArray *oldBranchList = oldTree->GetListOfBranches();
  for(int i=0; i<oldBranchList->GetSize(); ++i){
    TBranch * oldBranch = dynamic_cast<TBranch*>(oldBranchList->At(i));
    TString branchName = oldBranch->GetName();
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


  cout << "List of diff branches:" << endl;
  cout << "----------------------" << endl;
  dumpSet(diffBranchList);
  cout <<"----> " << diffBranchList.size() << " branches are diff" << endl;

  cout << "############ end comparing " << oldTree->GetName() <<" tree #################" << endl;

}
// stop clang check here. Not using const & since we want this function to be callable from command line using root
void compareOutputNtuples(TString oldFileName, // NOLINT
			  TString newFileName, // NOLINT
			  TString treeName="AnalysisMiniTree", // NOLINT
			  TString outdir="compare_output", // NOLINT
			  bool drawsame=true){

  gSystem->Exec("rm -rf "+outdir);
  gSystem->Exec("mkdir -p "+outdir);

  //gEnv->SetValue("Root.ErrorIgnoreLevel=Info");
  gEnv->SetValue("Root.ErrorIgnoreLevel=Warning");

  gStyle->SetOptStat(111111);

  TChain *ch1 = new TChain(treeName);
  TChain *ch2 = new TChain(treeName);


  ch1->Add(newFileName);
  ch2->Add(oldFileName);

  compareTrees(ch2, ch1, drawsame, outdir);

}


#ifndef __CLING__

void usage(const string &ss){
  cout << "Usage: " << ss << " oldfilename newfilename [treeName] [outdir] " << endl;
  cout << "default treeName=\"AnalysisMiniTree\" outdir=\"compare_output\" " << endl;
}

int main(int argc, char **argv){

  if(argc<3){
    usage(argv[0]);
    return 1;
  }

  TString oldFileName = argv[1];
  TString newFileName = argv[2];

  TString treeName="AnalysisMiniTree";
  TString outdir="compare_output";

  if(argc>3)treeName = argv[3];
  if(argc>4)outdir = argv[4];


  compareOutputNtuples(oldFileName, newFileName, treeName, outdir);

  return 0;

}

#endif
