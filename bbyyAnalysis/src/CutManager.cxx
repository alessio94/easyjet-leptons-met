/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CutManager.h"

void CutManager::CheckCutResults() {

    ATH_MSG_INFO("Total events that passed all cuts --> " << (*this).PassAllCuts);

    /* Events passed by each cut */
    for (const CutEntry& cut : *this)
        ATH_MSG_INFO("Cut : " << cut.name << " , Events passed --> " << cut.counter);

}

void CutManager::DoAbsoluteEfficiency(long long int nEvents, TH1* histo)
{

    /* Absolute efficiency histogram of yybb cuts */
    /* Absolute effiency is defined as :  (Events passed by the cut/ Total events) */
    // N_events(pass_i) / N_events 
    // N_events(pass_i+1) / N_events etc.
    //Fill first bin of total events.
    histo->SetBinContent(1, 100);

    for (size_t i = 0; i < size(); ++i)
    {
        int bin = i + 2;
        float percentage = static_cast<float>((*this)[i].counter) / nEvents * 100;
        histo->SetBinContent(bin, percentage);
    }

    // Set bin labels for each bin using the cut names
    histo->GetXaxis()->SetBinLabel(1, "All events");

    for (size_t i = 0; i < size(); ++i)
    {
        int bin = i + 2;
        histo->GetXaxis()->SetBinLabel(bin, (*this)[i].name.c_str());
    }

    histo->GetYaxis()->SetTitle("Absolute Efficiency %");
    histo->GetXaxis()->SetTitle("Cuts");

}

void  CutManager::DoRelativeEfficiency(long long int nEvents, TH1* histo)
{

    /* Relative efficiency histogram of yybb cuts */
    /* Relative efficiency  is defined as :  
    (Events passed by the cut with respect to the previous cuts / Total events passed by the previous cut) */
    // N_events(pass_i  AND pass_i-1  AND ... AND pass_0) / N_events (pass_i-1 AND ..... AND pass_0)

    // Start from the second bin (first cut). Then divide its bin Content by the bin Content of the previous bin.
    // Then for the third bin, divide its bin contents by the bin content of the second bin.
    // Definition of relative efficiency for the i-cut would be:

    histo->SetBinContent(1, 100); // Set First bin 100% of events.
    histo->SetBinContent(2, static_cast<float>((*this)[0].relativeCounter) / nEvents * 100);
    
    for (size_t i = 1; i < size(); ++i)
    {
        int bin = i + 2;
        histo->SetBinContent(bin, static_cast<float>((*this)[i].relativeCounter) / (*this)[i - 1].relativeCounter * 100);
    }

    // Set bin labels for each bin using the cut names
    histo->GetXaxis()->SetBinLabel(1, "All events");

    for (size_t i = 0; i < size(); ++i)
    {
        int bin = i + 2;
        histo->GetXaxis()->SetBinLabel(bin, (*this)[i].name.c_str());
    }

    histo->GetYaxis()->SetTitle("Relative Efficiency %");
    histo->GetXaxis()->SetTitle("Cuts");

}

void CutManager::DoStandardCutFlow(long long int nEvents, TH1* histo)
{

    /* Standard CutFlow Plot */
    /* Standard cutflow  is defined as :  (Events passed by the cut with respect to the previous cuts / Total events) */
    // N_events(pass_i  AND pass_i-1  AND ... AND pass_0) / N_events 
    // Set bin labels for each bin using the cut names
    // Set Bin Content for Standard CutFlow plot

    histo->SetBinContent(1, 100); // Set First bin 100% of events.

    for (size_t i = 0; i < size(); ++i)
    {
        int bin = i + 2;
        histo->SetBinContent(bin, static_cast<float>((*this)[i].relativeCounter) / nEvents * 100);
    }

    histo->GetXaxis()->SetBinLabel(1, "All events");

    for (size_t i = 0; i < size(); ++i)
    {
        int bin = i + 2;
        histo->GetXaxis()->SetBinLabel(bin, (*this)[i].name.c_str());
    }

    histo->GetYaxis()->SetTitle("Efficiency %");
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