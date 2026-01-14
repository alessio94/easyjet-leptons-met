/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MbbKinFitDecoratorAlg.h"

#include <xAODJet/JetAuxContainer.h>
#include "TLorentzVector.h"

namespace MULTILEPTON
{
    MbbKinFitDecoratorAlg ::MbbKinFitDecoratorAlg(const std::string &name,
                                                  ISvcLocator *pSvcLocator)
        : EL::AnaAlgorithm(name, pSvcLocator)
    {
    }
    StatusCode MbbKinFitDecoratorAlg ::initialize()
    {

        ATH_CHECK(m_eventHandle.initialize(m_systematicsList));
        ATH_CHECK(m_electronHandle.initialize(m_systematicsList));
        ATH_CHECK(m_muonHandle.initialize(m_systematicsList));

        ATH_CHECK(m_jetHandle.initialize(m_systematicsList));

        ATH_CHECK(m_jetOutHandle.initialize(m_systematicsList));
        ATH_CHECK(m_KF_MBB.initialize(m_systematicsList, m_eventHandle));

        ATH_CHECK(m_iter1_pt.initialize(m_systematicsList, m_eventHandle));
        ATH_CHECK(m_iter1_eta.initialize(m_systematicsList, m_eventHandle));
        ATH_CHECK(m_iter1_phi.initialize(m_systematicsList, m_eventHandle));
        ATH_CHECK(m_iter1_m.initialize(m_systematicsList, m_eventHandle));
        ATH_CHECK(m_iter2_pt.initialize(m_systematicsList, m_eventHandle));
        ATH_CHECK(m_iter2_eta.initialize(m_systematicsList, m_eventHandle));
        ATH_CHECK(m_iter2_phi.initialize(m_systematicsList, m_eventHandle));
        ATH_CHECK(m_iter2_m.initialize(m_systematicsList, m_eventHandle));

        ANA_CHECK(m_systematicsList.initialize());

        ATH_CHECK(m_KFTool.retrieve());

        return StatusCode::SUCCESS;
    }

    StatusCode MbbKinFitDecoratorAlg::execute()
    {

        for (const auto &sys : m_systematicsList.systematicsVector())
        {
            if (!m_doSystematics && sys.name() != "")
                continue;

            const xAOD::EventInfo *event = nullptr;
            ANA_CHECK(m_eventHandle.retrieve(event, sys));
            const xAOD::ElectronContainer *electrons = nullptr;
            ANA_CHECK(m_electronHandle.retrieve(electrons, sys));
            const xAOD::MuonContainer *muons = nullptr;
            ANA_CHECK(m_muonHandle.retrieve(muons, sys));
            const xAOD::JetContainer *jets = nullptr;
            ANA_CHECK(m_jetHandle.retrieve(jets, sys));

            // setup KF tool inputs
            double KF1_Mbb = -99.;
            if (jets->size() >= 2)
            {
                TLorentzVector j1 = jets->at(0)->p4();
                TLorentzVector j2 = jets->at(1)->p4();
                TLorentzVector jj = j1 + j2;
                KF1_Mbb = jj.M();
            }

            auto workJetContainer = std::make_unique<xAOD::JetContainer>();
            auto workJetAuxContainer = std::make_unique<xAOD::JetAuxContainer>();
            workJetContainer->setStore(workJetAuxContainer.get());

            for (const auto jet : *jets)
            {
                xAOD::Jet *thisJet = new xAOD::Jet();
                workJetContainer->push_back(thisJet);
                *thisJet = *jet; 
            }

            ATH_CHECK(m_KFTool->applyKF_bb4l(
                    *electrons, *muons, *workJetContainer, KF1_Mbb));
            auto constDataWorkJetContainer = std::make_unique<ConstDataVector<xAOD::JetContainer>>();
            constDataWorkJetContainer->reserve(workJetContainer->size());

            const auto &iter1_jets = m_KFTool->getIter1Jets();
            const auto &iter2_jets = m_KFTool->getIter2Jets();
            std::vector<float> iter1_pt, iter1_eta, iter1_phi, iter1_m;
            std::vector<float> iter2_pt, iter2_eta, iter2_phi, iter2_m;
            for (const auto &jet : iter1_jets)
            {
                iter1_pt.push_back(jet.Pt());
                iter1_eta.push_back(jet.Eta());
                iter1_phi.push_back(jet.Phi());
                iter1_m.push_back(jet.M());
            }
            for (const auto &jet : iter2_jets)
            {
                iter2_pt.push_back(jet.Pt());
                iter2_eta.push_back(jet.Eta());
                iter2_phi.push_back(jet.Phi());
                iter2_m.push_back(jet.M());
            }
            m_iter1_pt.set(*event, iter1_pt, sys);
            m_iter1_eta.set(*event, iter1_eta, sys);
            m_iter1_phi.set(*event, iter1_phi, sys);
            m_iter1_m.set(*event, iter1_m, sys);
            m_iter2_pt.set(*event, iter2_pt, sys);
            m_iter2_eta.set(*event, iter2_eta, sys);
            m_iter2_phi.set(*event, iter2_phi, sys);
            m_iter2_m.set(*event, iter2_m, sys);

            for (xAOD::Jet *jet : *workJetContainer)
            {
                auto thisJet = std::make_unique<xAOD::Jet>(*jet);
                constDataWorkJetContainer->push_back(thisJet.release());
            }

            m_KF_MBB.set(*event, KF1_Mbb, sys);

            ATH_CHECK(m_jetOutHandle.record(std::move(constDataWorkJetContainer), sys));
        }
        return StatusCode::SUCCESS;
    }
}
