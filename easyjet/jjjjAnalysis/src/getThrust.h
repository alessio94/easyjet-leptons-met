#include "TLorentzVector.h"
#include <assert.h>

/*-------------------------------------*/
/*  File adapted by Javier Llorente    */
/*  from the Rivet Thrust projection   */
/*  to be used in ROOT analyses        */
/*-------------------------------------*/

inline bool mod2Cmp(const TVector3& a, const TVector3& b) {
  return a.Mag2() > b.Mag2();
}

void calcT(const std::vector<TVector3>& momenta, double& t, TVector3& taxis) {

  // This function implements the iterative algorithm as described in the
  // Pythia manual. We take eight (four) different starting vectors
  // constructed from the four (three) leading particles to make sure that
  // we don't find a local maximum.
  std::vector<TVector3> p = momenta;
  assert(p.size() >= 3);

  unsigned int n = 3;
  if (p.size() == 3) n = 3;
  std::vector<TVector3> tvec;
  std::vector<double> tval;
  std::sort(p.begin(), p.end(), mod2Cmp);
  for (size_t i = 0 ; i < pow(2, n-1); ++i) {
    // Create an initial vector from the leading four jets
    TVector3 foo(0,0,0);
    int sign = i;
    for (unsigned int k = 0 ; k < n ; ++k) {
      (sign % 2) == 1 ? foo += p[k] : foo -= p[k];
      sign /= 2;
    }
    foo=foo.Unit();
    
    // Iterate
    double diff=999.;
    while (diff>1e-5) {
      TVector3 foobar(0,0,0);
      for (unsigned int k=0 ; k<p.size() ; k++)
	foo.Dot(p[k])>0 ? foobar+=p[k] : foobar-=p[k];
      diff=(foo-foobar.Unit()).Mag();
      foo=foobar.Unit();
    }
    
    // Calculate the thrust value for the vector we found
    t=0.;
    for (unsigned int k=0 ; k<p.size() ; k++)
      t+=fabs(foo.Dot(p[k]));
    
    // Store everything
    tval.push_back(t);
    tvec.push_back(foo);
  }
  
  // Pick the solution with the largest thrust
  t=0.;
  for (unsigned int i=0 ; i<tvec.size() ; i++)
    if (tval[i]>t){
      t=tval[i];
      taxis=tvec[i];
    }
}

// Do the full calculation
std::vector< std::vector<double> > calcThrust(const std::vector<TLorentzVector>& input4){
  
  //[i][j]
  //[i] = 0 --> [j] = 0, 1 for transverse thrust, transverse minor
  //[i] = 1 --> [j] = 0, 1 --> The thrust axis transverse components

  std::vector<TLorentzVector> myJets; myJets.clear();
  for (size_t k = 0; k < input4.size(); ++k) myJets.push_back(input4[k]);

  //Ghosts
  if (myJets.size() == 2){
    TLorentzVector ghost; ghost.SetPxPyPzE(1e-13,0,0,1e-13);
    myJets.push_back(ghost);
  }

  std::vector<TVector3> fsmomenta; fsmomenta.clear();
  for (size_t k = 0; k < myJets.size(); ++k){
    TVector3 triVector = myJets[k].Vect();
    triVector.SetZ(0);
    fsmomenta.push_back(triVector);
  }

  std::vector<double> vThrusts; vThrusts.clear();
  std::vector<TVector3> vThrustAxes; vThrustAxes.clear();
  std::vector< std::vector<double> > vOutput; vOutput.clear();
  
  // Make a vector of the three-momenta in the final state
  double momentumSum = 0.0;
  
  for (unsigned int k = 0; k < fsmomenta.size(); ++k){
    TVector3 v = fsmomenta[k];
    momentumSum += v.Mag();
  }
  
  // Clear the caches
  vThrusts.clear();
  vThrustAxes.clear();
  vOutput.clear();

  // If there are fewer than 2 visible particles, we can't do much
  if (fsmomenta.size() < 2) {
    for (size_t i = 0; i < 3; ++i) {
      vThrusts.push_back(2);
      vThrustAxes.push_back(TVector3(0,0,0));
    }

    vOutput.push_back(vThrusts);

    std::vector<double> vOut2; vOut2.clear();
    vOut2.push_back(vThrustAxes[0].Px());
    vOut2.push_back(vThrustAxes[0].Py());

    vOutput.push_back(vOut2);

    return vOutput;
  }
  
  
  // Handle special case of thrust = 1 if there are only 2 particles
  if (fsmomenta.size() == 2) {
    TVector3 axis(0,0,0);
    vThrusts.push_back(1.0);
    vThrusts.push_back(0.0);
    vThrusts.push_back(0.0);
    axis = fsmomenta[0].Unit();
    if (axis.z() < 0) axis = -axis;
    vThrustAxes.push_back(axis);
    /// @todo Improve this --- special directions bad...
    /// (a,b,c) _|_ 1/(a^2+b^2) (b,-a,0) etc., but which combination minimises error?
    if (axis.z() < 0.75)
      vThrustAxes.push_back( (axis.Cross(TVector3(0,0,1))).Unit() );
    else
      vThrustAxes.push_back( (axis.Cross(TVector3(0,1,0))).Unit() );
    vThrustAxes.push_back( vThrustAxes[0].Cross(vThrustAxes[1]) );

    vOutput.push_back(vThrusts);

    std::vector<double> vOut2; vOut2.clear();
    vOut2.push_back(vThrustAxes[0].Px());
    vOut2.push_back(vThrustAxes[0].Py());

    vOutput.push_back(vOut2);

    return vOutput;
  }

  // Temporary variables for calcs
  TVector3 axis(0,0,0);
  double val = 0.;
  
  // Get thrust
  calcT(fsmomenta, val, axis);
  vThrusts.push_back(val / momentumSum);

  // Make sure that thrust always points along the +ve z-axis.
  if (axis.z() < 0) axis = -axis;
  axis = axis.Unit();
  vThrustAxes.push_back(axis);
  
  // Get thrust major
  std::vector<TVector3> threeMomenta; threeMomenta.clear();

  for (unsigned int k = 0; k < fsmomenta.size(); ++k){
    TVector3 v = fsmomenta[k];
    // Get the part of each 3-momentum which is perpendicular to the thrust axis
    const TVector3 vpar = v.Dot(axis.Unit()) * axis.Unit();
    threeMomenta.push_back(v - vpar);
  }
  calcT(threeMomenta, val, axis);
  vThrusts.push_back(val / momentumSum);
  if (axis.x() < 0) axis = -axis;
  axis = axis.Unit();
  vThrustAxes.push_back(axis);
  
  // Get thrust minor
  if (vThrustAxes[0].Dot(vThrustAxes[1]) < 1e-10) {
    axis = vThrustAxes[0].Cross(vThrustAxes[1]);
    vThrustAxes.push_back(axis);
    val = 0.0;

    for (unsigned int k = 0; k < fsmomenta.size(); ++k){
      TVector3 v = fsmomenta[k];
      val += fabs(axis.Dot(v));
    }
    vThrusts.push_back(val / momentumSum);
  } else {
    vThrusts.push_back(-1.0);
    vThrustAxes.push_back(TVector3(0,0,0));
  }
  

  vOutput.push_back(vThrusts);

  std::vector<double> vOut2; vOut2.clear();
  vOut2.push_back(vThrustAxes[0].Px());
  vOut2.push_back(vThrustAxes[0].Py());

  vOutput.push_back(vOut2);

  return vOutput;
}
