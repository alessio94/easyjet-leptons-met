/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

  CutEntry & CutManager.
  Classes to store and manage cut selections + plotting methods for cut efficiencies.
*/

/// @author Spyridon Merianos

#ifndef CUTMANAGER_H
#define CUTMANAGER_H

#include <AthenaBaseComps/AthMessaging.h>
#include <TEfficiency.h>
#include <TH1.h>
#include <vector>
#include <iostream>

struct CutEntry
{
    std::string name{}; // Name of the cut
    int counter{0}; //Counts the events passed by the cut.
    double w_counter{0.0}; //Counts the events passed by the cut in terms of mcEventWeights.at(0)
    bool passed{0}; // If event is passed this should be set to true. Otherwise, it should be set to false.
    int relativeCounter{0}; // Counts the events passed with respect to the previous cuts. 
    double w_relativeCounter{0.0}; // Counts the events passed with respect to the previous cuts in terms of mcEventWeights.at(0)
    
    CutEntry(const std::string& n)
        : name(n) {}
};

class CutManager : public AthMessaging, public std::vector<CutEntry>
{
    public:
        CutManager():
            AthMessaging("CutManagerMessaging"){}

        // Overload the function call operator to access by cut name (string)
        CutEntry& operator()(const std::string& name)
        {
            for (CutEntry& cutEntry : *this)
            {
                if (cutEntry.name == name)
                    return cutEntry;
            }
            throw std::runtime_error("Could not access cut : " + name + ". It doesn't exist in the standard cut lists.");
        }

        // Overload the function call operator to access by index
        CutEntry& operator[](size_t index)
        {
            if (index < size()) // No need for this->size() here
                return std::vector<CutEntry>::operator[](index); // Directly access base class operator
            throw std::out_of_range("Index out of range. Probably size mismatch between input Cut List and CutManager cuts.");
        }

        void add(const std::string& name)
        {
            push_back(CutEntry(name)); // No need for this->push_back here
        }

        // Erase a CutEntry element by name
        void erase(const std::string& name)
        {
            for (auto it = begin(); it != end();)
            {
                if (it->name == name)
                {
                    it = std::vector<CutEntry>::erase(it); // Specify std::vector<CutEntry>::erase
                    std::cout << "Erasing cut : " << name << ". Could not access respective decoration from the input file." << std::endl;
                    return;
                } 
                else
                {
                    ++it;
                }
            }
            throw std::runtime_error("Could not erase cut : " + name + ". It doesn't exist in the standard cut lists.");
        }

        // Check if a cut with a given name exists
        bool exists(const std::string& name) const
        {
            return std::find_if(begin(), end(),[&name](const CutEntry& info){return info.name == name;}) != end();
        }

        void CheckCutResults();
        void DoAbsoluteEfficiency(long long int nEvents, TEfficiency* eff);
        void DoRelativeEfficiency(long long int nEvents, TEfficiency* eff);
        void DoStandardCutFlow(long long int nEvents, TEfficiency* eff);
        void DoCutflowLabeling(long long int nEvents, TH1* histo);
        void DoWeightedAbsoluteEfficiency(float totalWeight, TEfficiency* eff);
        void DoWeightedRelativeEfficiency(float totalWeight, TEfficiency* eff);
        void DoWeightedStandardCutFlow(float totalWeight, TEfficiency* eff);
        void CheckInputCutList(std::vector<std::string> inputCutList, std::vector<std::string> standard_cuts );

        int PassAllCuts{0};

};

#endif // CUTMANAGER_H
