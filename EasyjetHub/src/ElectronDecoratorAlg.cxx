/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Carl Gwilliam


#include "ElectronDecoratorAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

#include <AthenaKernel/Units.h>

#include <xAODEgamma/EgammaxAODHelpers.h>
#include <TLorentzVector.h>
#include <cmath>


namespace Easyjet
{
  ElectronDecoratorAlg ::ElectronDecoratorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator) { }

  StatusCode ElectronDecoratorAlg ::initialize()
  {
    ATH_CHECK (m_electronsInKey.initialize());
    ATH_CHECK	(m_trackParticleKey.initialize());
    ATH_CHECK	(m_gsfTrackParticleKey.initialize());
    ATH_CHECK	(m_VertexContainerKey.initialize());

    m_closestSiTrackDecorKey = m_electronsInKey.key() + ".closestSiTrack";
    m_bestmatchedElTrackDecorKey = m_electronsInKey.key() + ".bestmatchedElTrack";
    m_primaryVertexDecorKey = m_electronsInKey.key() + ".primaryVertex";
    ATH_CHECK (m_closestSiTrackDecorKey.initialize(m_doRetrieveTracks));
    ATH_CHECK (m_bestmatchedElTrackDecorKey.initialize(m_doRetrieveTracks));
    ATH_CHECK (m_primaryVertexDecorKey.initialize(m_doRetrieveTracks));


    m_mllConvKey = m_electronsInKey.key() + ".mllConv";
    m_mllConvAtConvVKey = m_electronsInKey.key() + ".mllConvAtConvV";
    m_radiusConvKey = m_electronsInKey.key() + ".radiusConv";
    m_separationMinDCTKey = m_electronsInKey.key() + ".separationMinDCT";
    ATH_CHECK (m_mllConvKey.initialize(m_doRetrieveTracks));
    ATH_CHECK (m_mllConvAtConvVKey.initialize(m_doRetrieveTracks));
    ATH_CHECK (m_radiusConvKey.initialize(m_doRetrieveTracks));
    ATH_CHECK (m_separationMinDCTKey.initialize(m_doRetrieveTracks));


    return StatusCode::SUCCESS;
  }

  StatusCode ElectronDecoratorAlg ::execute(const EventContext& ctx) const
  {
    // input handles
    SG::ReadHandle<xAOD::ElectronContainer> electronsIn(m_electronsInKey,ctx);
    ATH_CHECK (electronsIn.isValid());

    if (m_doRetrieveTracks){

      SG::WriteDecorHandle<xAOD::ElectronContainer, float> mllConvDecorHandle(m_mllConvKey);
      SG::WriteDecorHandle<xAOD::ElectronContainer, float> mllConvAtConvVDecorHandle(m_mllConvAtConvVKey);
      SG::WriteDecorHandle<xAOD::ElectronContainer, float> radiusConvDecorHandle(m_radiusConvKey);
      SG::WriteDecorHandle<xAOD::ElectronContainer, float> separationMinDCTDecorHandle(m_separationMinDCTKey);

      typedef ElementLink<xAOD::TrackParticleContainer> Link_track; 
      SG::WriteDecorHandle<xAOD::ElectronContainer, Link_track> closestSiTrackDecorHandle(m_closestSiTrackDecorKey);
      SG::WriteDecorHandle<xAOD::ElectronContainer, Link_track> bestmatchedElTrackDecorHandle(m_bestmatchedElTrackDecorKey);

      SG::ReadHandle<xAOD::TrackParticleContainer> tracksIn(m_trackParticleKey,ctx);
      ATH_CHECK (tracksIn.isValid());

      SG::ReadHandle<xAOD::TrackParticleContainer> gsfTracksIn(m_gsfTrackParticleKey,ctx);
      ATH_CHECK (tracksIn.isValid());

      SG::ReadHandle<xAOD::VertexContainer> vertexIn(m_VertexContainerKey,ctx);
      ATH_CHECK (vertexIn.isValid());

      const xAOD::Vertex* pvtx = nullptr;
      for(const xAOD::Vertex* vtx : *vertexIn) {
        if(vtx->vertexType() == xAOD::VxType::PriVtx) {
          pvtx = vtx;
          break;
        }
      }

      for(const xAOD::Electron* electron : *electronsIn) {
        float mll_conv = -99.;
        float mll_conv_atConvV = -99.;
        float radius_conv=-99.;
        float separationMinDCT = -99; // how good the distance between the circles is 

        const xAOD::TrackParticle *closestSiTrack(0);
        const xAOD::TrackParticle *bestmatchedElTrack = electron->trackParticle();

        double detaMin = 999;

        if (bestmatchedElTrack){
          for (const xAOD::TrackParticle *track : *tracksIn) {
            double dR = bestmatchedElTrack->p4().DeltaR(track->p4());
            double dz0 = std::abs(bestmatchedElTrack->z0() - track->z0())*sin(bestmatchedElTrack->theta()); 

            if ( dR<0.3 && dz0<0.5 ) { // loose DeltaR cut and tight delta(z0) cut. dz0 in mm
              bool hasSi = xAOD::EgammaHelpers::numberOfSiHits(track)>=8;
              
              double deta=std::abs(track->eta()-bestmatchedElTrack->eta()); 
              if(deta<detaMin && hasSi && ((bestmatchedElTrack->charge() * track->charge()) < 0) ) {
                detaMin=deta;
                closestSiTrack = track;
              }
            }
          } // end of loop over tracks

          if (closestSiTrack){
            TLorentzVector p0,p1;  
            p0.SetPtEtaPhiM(bestmatchedElTrack->pt(),bestmatchedElTrack->eta(),bestmatchedElTrack->phi(), m_e);   
            p1.SetPtEtaPhiM(closestSiTrack->pt(),closestSiTrack->eta(),closestSiTrack->phi(), m_e);
            mll_conv=(p0+p1).M();
            
            p0.SetPtEtaPhiM(bestmatchedElTrack->pt(),bestmatchedElTrack->eta(),0,m_e);   
            p1.SetPtEtaPhiM(closestSiTrack->pt(),closestSiTrack->eta(),0,m_e);
            mll_conv_atConvV=(p0+p1).M();

            // Conversion methods
            // Helix array:
            // 0 cotan(theta)
            // 1 curvature
            // 2 z
            // 3 d0
            // 4 phi

            ////////////// TrackToHelix Reco Electron Track
            double helix1[5];
            assignHelixArray(bestmatchedElTrack, helix1, pvtx);

            ///////////// TrackToHelix Other Electron Track
            double helix2[5];
            assignHelixArray(closestSiTrack, helix2, pvtx);
            
            double dct(helix1[0]-helix2[0]); 
                
            /////
            double beta(0.);
            if(helix1[4] < helix2[4]) {
              beta = 0.5*M_PI-helix1[4];
            } else {
              beta = 0.5*M_PI-helix2[4];
            }
          
          
            /// HelixToCircle Main Track Electron
            double r1 = 1/(2.*std::abs(helix1[1]));
          
            std::pair<float, float> position1 = position(helix1);
            double x1 = position1.first, y1 = position1.second; 
          
            /// HelixToCircle Other Electron Conv Track
            double r2 = 1/(2.*std::abs(helix2[1]));
          
            std::pair<float, float> position2 = position(helix2);
            double x2 = position2.first, y2 = position2.second; 

            //////
          
            double dx(x1- x2);
            if(dx <  1e-9 && dx > 0.) dx =  1e-9;
            if(dx > -1e-9 && dx < 0.) dx = -1e-9;
            double slope((y1-y2)/dx);
            double b(y1 - slope*x1);
            double alpha(atan(slope));
            double d(std::hypot(x1-x2, y1-y2));
            //only keeping opposite sign option
            double separation = d-r1-r2;
            double cpx1, cpx2;
            if(x1>x2)
              {
                cpx1 = x1-r1*cos(alpha);
                cpx2 = x2+r2*cos(alpha); 
              }
            else
              {
                cpx1 = x1+r1*cos(alpha);
                cpx2 = x2 - r2*cos(alpha);
              }
          
          
            double temp1 = (cpx1+cpx2)/2;
            double temp2 = slope*temp1+b;
            double convX = cos(beta)*temp1 + sin(beta)*temp2;
            double convY = -sin(beta)*temp1+ cos(beta)*temp2;
              
              
            ///////
            if(std::abs(separation)<1.&& std::abs(dct)<0.02){
              separationMinDCT=separation;
              radius_conv=sqrt(convX*convX + convY*convY);
              if( convX*cos(bestmatchedElTrack->phi()) + convY*sin(bestmatchedElTrack->phi()) < 0)	radius_conv = -radius_conv; 
            }
          }
        }
        const Link_track link_closestSiTrack = closestSiTrack ? Link_track(closestSiTrack, *tracksIn, ctx) : Link_track();
        closestSiTrackDecorHandle(*electron) = link_closestSiTrack;

        const Link_track link_bestmatchedElTrack = bestmatchedElTrack ? Link_track(bestmatchedElTrack, *gsfTracksIn, ctx) : Link_track();
        bestmatchedElTrackDecorHandle(*electron) = link_bestmatchedElTrack;

        mllConvDecorHandle(*electron) = mll_conv;
        mllConvAtConvVDecorHandle(*electron) = mll_conv_atConvV;
        radiusConvDecorHandle(*electron) = radius_conv;
        separationMinDCTDecorHandle(*electron) = separationMinDCT;
      } // end of loop over electrons
    }

    return StatusCode::SUCCESS;
  }

  void ElectronDecoratorAlg ::assignHelixArray(const xAOD::TrackParticle* track, double helix[5], const xAOD::Vertex* pvtx) const{
    helix[0] = 1./tan(track->theta());
    helix[1] = PTTOCURVATURE*track->charge()/track->pt();
    double c = cos(track->phi0());
    double s = sin(track->phi0());
    helix[3] = track->d0() + c*pvtx->y() - s*pvtx->x();
    c = c*1./tan(track->theta());
    s = s*1./tan(track->theta());
    helix[2] = track->z0() - c*pvtx->x() - s*pvtx->y() + pvtx->z();
    if(track->phi0()>0.)
      helix[4] = track->phi0();
    else
      helix[4] = 2*M_PI + track->phi0();
  }

  std::pair<float, float> ElectronDecoratorAlg ::position(const double* helix) const{
    double charge = helix[1]>0 ? 1 : -1;
    double rcenter = helix[3]/charge + 1/(2*std::abs(helix[1]));
    double phicenter = helix[4] + 0.5*M_PI*charge;
    double x = rcenter*cos(phicenter);
    double y = rcenter*sin(phicenter);

    return std::make_pair(x, y);
  }

}
