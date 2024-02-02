/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "EasyjetHub/CutManager.h"

void CutManager::CheckCutResults() {

    ATH_MSG_INFO("Total events that passed all cuts --> " << (*this).PassAllCuts);

    /* Events passed by each cut */
    for (const CutEntry& cut : *this)
        ATH_MSG_INFO("Cut : " << cut.name << " , Events passed --> " << cut.counter);

}

void CutManager::DoAbsoluteEfficiency(long long int nEvents, TEfficiency* eff)
{

    /* Absolute efficiency histogram of selection cuts */
    /* Absolute effiency is defined as :  (Events passed by the cut/ Total events) */
    // N_events(pass_i) / N_events 
    // N_events(pass_i+1) / N_events etc.   

    // Set First bin to total number of events.
    eff->SetTotalEvents(1,nEvents);
    eff->SetPassedEvents(1,nEvents); 

    for (size_t i = 0; i < size(); ++i)
    {
        int bin = i+2;
        eff->SetTotalEvents(bin, nEvents);
        eff->SetPassedEvents(bin, (*this)[i].counter);
    }
    
}

void CutManager::DoWeightedAbsoluteEfficiency(float totalWeight, TEfficiency* eff)
{

    /* Absolute efficiency histogram of selection cuts weighted by mcEventWeights */

    int nbins = eff->GetTotalHistogram()->GetNbinsX();
    TH1D *h_tot = new TH1D("h_tot","tot",nbins,0.5,nbins+0.5);
    TH1D *h_pass = new TH1D("h_pass","pass",nbins,0.5,nbins+0.5);

    h_tot->SetBinContent(1,totalWeight);
    h_pass->SetBinContent(1,totalWeight);

    for (size_t i = 0; i < size(); ++i)
    {
        int bin = i+2;
        h_tot->SetBinContent(bin,totalWeight);
        h_pass->SetBinContent(bin,(*this)[i].w_counter);
    }

    if(TEfficiency::CheckConsistency(*h_pass,*h_tot))
    {
         eff->SetTotalHistogram(*h_tot,"");
         eff->SetPassedHistogram(*h_pass,"");
    }
    
    delete h_tot;
    delete h_pass;
    
}

void  CutManager::DoRelativeEfficiency(long long int nEvents, TEfficiency* eff)
{

    /* Relative efficiency histogram of slection cuts */
    /* Relative efficiency  is defined as :  
    (Events passed by the cut with respect to the previous cuts / Total events passed by the previous cut) */
    // N_events(pass_i  AND pass_i-1  AND ... AND pass_0) / N_events (pass_i-1 AND ..... AND pass_0)

    // Start from the second bin (first cut). Then divide its bin Content by the bin Content of the previous bin.
    // Then for the third bin, divide its bin contents by the bin content of the second bin.
    // Definition of relative efficiency for the i-cut would be:

    // Set First bin to total number of events.
    eff->SetTotalEvents(1,nEvents);
    eff->SetPassedEvents(1,nEvents); 
    
    // Set second bin to events passed by first cut divided by total number of events.
    eff->SetTotalEvents(2,nEvents);
    eff->SetPassedEvents(2, (*this)[0].relativeCounter);
    
    for (size_t i = 1; i < size(); ++i)
    {
        int bin = i + 2;
        eff->SetTotalEvents(bin,(*this)[i-1].relativeCounter);
        eff->SetPassedEvents(bin,(*this)[i].relativeCounter);
    }

}

void  CutManager::DoWeightedRelativeEfficiency(float totalWeight, TEfficiency* eff)
{
    /* Relative efficiency histogram of selection cuts weighted by mcEventWeights */

    int nbins = eff->GetTotalHistogram()->GetNbinsX();
    TH1D *h_tot = new TH1D("h_tot","tot",nbins,0.5,nbins+0.5);
    TH1D *h_pass = new TH1D("h_pass","pass",nbins,0.5,nbins+0.5);

    h_tot->SetBinContent(1,totalWeight);
    h_pass->SetBinContent(1,totalWeight);
    
    h_tot->SetBinContent(2,totalWeight);
    h_pass->SetBinContent(2, (*this)[0].w_relativeCounter);
    
    for (size_t i = 1; i < size(); ++i)
    {
        int bin = i + 2;
        h_tot->SetBinContent(bin,(*this)[i-1].w_relativeCounter);
        h_pass->SetBinContent(bin,(*this)[i].w_relativeCounter);
    }

    if(TEfficiency::CheckConsistency(*h_pass,*h_tot))
    {
         eff->SetTotalHistogram(*h_tot,"");
         eff->SetPassedHistogram(*h_pass,"");
    }
    
    delete h_tot;    
    delete h_pass;

}

void CutManager::DoStandardCutFlow(long long int nEvents, TEfficiency* eff)
{

    /* Standard CutFlow Plot */
    /* Standard cutflow  is defined as :  (Events passed by the cut with respect to the previous cuts / Total events) */
    // N_events(pass_i  AND pass_i-1  AND ... AND pass_0) / N_events 
    // Set bin labels for each bin using the cut names
    // Set Bin Content for Standard CutFlow plot

    // Set First bin to total number of events.
    eff->SetTotalEvents(1,nEvents);
    eff->SetPassedEvents(1,nEvents); 

    for (size_t i = 0; i < size(); ++i)
    {
        int bin = i + 2;
        eff->SetTotalEvents(bin,nEvents);
        eff->SetPassedEvents(bin,(*this)[i].relativeCounter);
    }

}


void CutManager::DoWeightedStandardCutFlow(float totalWeight, TEfficiency* eff)
{

/* Weighted Standard Cutflow histogram of selection cuts weighted by mcEventWeights */

    int nbins = eff->GetTotalHistogram()->GetNbinsX();
    TH1D *h_tot = new TH1D("h_tot","tot",nbins,0.5,nbins+0.5);
    TH1D *h_pass = new TH1D("h_pass","pass",nbins,0.5,nbins+0.5);

    h_tot->SetBinContent(1,totalWeight);
    h_pass->SetBinContent(1,totalWeight);

    for (size_t i = 0; i < size(); ++i)
    {
        int bin = i + 2;
        h_tot->SetBinContent(bin,totalWeight);
        h_pass->SetBinContent(bin,(*this)[i].w_relativeCounter);
    }

    if(TEfficiency::CheckConsistency(*h_pass,*h_tot))
    {
         eff->SetTotalHistogram(*h_tot,"");
         eff->SetPassedHistogram(*h_pass,"");
    }

    delete h_tot;    
    delete h_pass;

}

void CutManager::DoCutflowLabeling(long long int nEvents, TH1* histo)
{

    /* TEfficiency doesn't support directly bin labeling.
       We create, this Absolute Efficiency TH1 histogram which entails bin labels,
       corresponding to each cut. It should help the user to identify the cuts 
       for each of the bins in TEfficiency plots, while also
       filling the bins with the event yields. */

    // Set First bin to total number of events.
    histo->SetBinContent(1,nEvents);

    for (size_t i = 0; i < size(); ++i)
    {
        int bin = i+2;
        histo->SetBinContent(bin, (*this)[i].counter);
    }
    
    // Set bin labels for each bin using the cut names
    histo->GetXaxis()->SetBinLabel(1, "All events");

    for (size_t i = 0; i < size(); ++i)
    {
        int bin = i + 2;
        histo->GetXaxis()->SetBinLabel(bin, (*this)[i].name.c_str());
    }

    histo->GetYaxis()->SetTitle("Events Passed");
    histo->GetXaxis()->SetTitle("Cuts");
    
}


void CutManager::CheckInputCutList(std::vector<std::string> inputCutList, std::vector<std::string> standard_cuts )
{

    // In case of empty CutList list, then AssertionError or ValueErrors occur from python scripts

    // Check for incorrect cuts in the CutList of the config.yaml file
    for (const std::string &cut : inputCutList)
    {
        if (std::find(standard_cuts.begin(), standard_cuts.end(), cut) == standard_cuts.end())
        {
            ATH_MSG_ERROR("\nFound an incorrect cut in the inputCutList. Please provide at least some of the following cuts :");
            
            for (const std::string &standard_cut : standard_cuts)
            {
                std::cout << "            " << standard_cut << std::endl;
            }
            throw std::runtime_error("Exiting program."); // Exit the program with an error code
        }
    }

    return;

}