//Modified by Rory Feb 21, 2017.
//L899-L901 - Modified theta definition
//969-L1007 - Asymmetry Variables and cross-section
//L1028 - Added asym cross-section to fZASigma_Lab

# ifndef __CINT__
#include <iostream>
#include <fstream>      // std::ifstream
#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <iomanip>
#include <stdio.h>
#include <math.h>
#include <cmath>
#include <string>       // std::string
#include <sstream>      // std::stringstream, std::stringbuf

#include <TStopwatch.h>
#include <TDatime.h>
#include <TVector3.h>
#include <TLorentzVector.h>
// #include <TTree.h>
// #include <TFile.h>
// #include <TH1.h>

#include "pimFermi.h"
#include "eloss.h"
#include "initilize.h"
#include "setrootfile.h"
#include "correction.h"
#include "SigmaL.h"
#include "SigmaT.h"
#include "SigmaLT.h"
#include "SigmaTT.h"
#include "asy.h"
#include "phase.h"
#include "fsi.h"

using std::setw;
using std::setprecision;
using std::cout;
using std::cin;
using std::endl;
using namespace std;


// 1) Missing mass, momentum and enrgy.
// 2) Modification to phi of scat electrona and pion
// 3) Follow the SIMC to generate the lab pion momentum.

//---------------------------------------------------------

//---------------------------------------------------------
double pim::fermiMomentum() {

  double fMom;
  bool kFermi = true;
  while ( kFermi ) {
    double fProton_Rand_Mom_Col      = fRandom->Uniform( 0, 300.0);
    double fProton_Rand_Mom_Col_Prob = fRandom->Uniform( fProb[300], fProb[0] );
    int    fProton_Mom_Int           = std::ceil( fProton_Rand_Mom_Col );
    double f3He_Value                = fProb[ fProton_Mom_Int - 1 ];

    if ( fProton_Rand_Mom_Col_Prob <= f3He_Value ) {
      fMom = fProton_Rand_Mom_Col;	
      kFermi = false;
    }
  }

  return fMom;
}

//---------------------------------------------------------
// g++ -o pim pimFermi.C `root-config --cflags --glibs`
//---------------------------------------------------------

int main()
{
  // double fEpsilon_Hermes;
  double cos_t, sin_t;
  pim myPim;
  myPim.Initilize();

  TDatime dsTime;
  cout << "Start Time:   " << dsTime.GetHour() << ":" << dsTime.GetMinute() << endl;

  TStopwatch tTime;
  tTime.Start(); 

  kCalcFermi  = true;
  kCalcBremss = true;
  kCalcIon    = true;
  kFSI        = true;

  cout << "Enter the number of events:";
  cin >> fNEvents;
  cout << endl;

  cout << "Enter the file number:";
  cin >> fNFile;
  cout << endl;

  stringstream ssElectronK;
  ssElectronK << fElectron_Kin_Col_GeV;

  stringstream ssEvents;
  ssEvents << fNEvents;
  stringstream ssFile;
  ssFile << fNFile;

  string sRFile = "./RootFiles/DEMP_Ee_";
  string sLFile = "./LundFiles/DEMP_Ee_";
  sRFile += ssElectronK.str();
  sLFile += ssElectronK.str();
  sRFile += "_Events_";
  sLFile += "_Events_";
  sRFile += ssEvents.str();
  sLFile += ssEvents.str();
  sRFile += "_File_";
  sLFile += "_File_";
  sRFile += ssFile.str();
  sLFile += ssFile.str();
  sRFile += ".root";
  sLFile += ".lund";

  string sTFile = "./LundFiles/DEMP_Ee_";
  sTFile += ssElectronK.str();
  sTFile += "_Events_";
  sTFile += ssEvents.str();
  sTFile += "_File_";
  sTFile += ssFile.str();
  sTFile += ".txt";
  
  ofstream ppiOut ( sLFile.c_str() );
  ofstream ppiDetails ( sTFile.c_str() );

  myPim.setrootfile( sRFile );
  int i = 0;

  for ( i = 0; i < fNEvents; i++ ) {

    TDatime dFractTime;  

    fNGenerated ++;    

    if ( i % ( fNEvents / 10 ) == 0 ) {
      cout << "Event: " << setw(8) << i 
    	   << "     % of events " << setw(4) << ((1.0*i)/(1.0*fNEvents))*100.0
    	   << "   Day: " <<  dFractTime.GetDay() 
    	   << "   Time:   " << dFractTime.GetHour() 
    	   << ":" << dFractTime.GetMinute() 
    	   << ":" << dFractTime.GetSecond() 
    	   << endl;	  
    }
    
    fProton_Theta_Col                     = 0.0;
    fProton_Phi_Col                       = 0.0;
    fProton_Mom_Col                       = 1e-6;    
    fVertex_X                             = fRandom->Uniform(-0.25,0.25);
    fVertex_Y                             = fRandom->Uniform(-0.25,0.25);
    fVertex_Z                             = fRandom->Uniform(-370,-330);

    if  ( kCalcFermi ) {
      fProton_Mom_Col                     = myPim.fermiMomentum();
      fProton_Theta_Col                   = ( 180.0 / fPi ) * TMath::ACos( fRandom->Uniform( cos(0.0 * fPi/180.0) ,
											     cos(180.0 * fPi/180.0) ) );
      fProton_Phi_Col                     = fRandom->Uniform( 0 , 360 );
    }

    fProton_Energy_Col                    = TMath::Sqrt( TMath::Power( fProton_Mom_Col , 2 ) + TMath::Power(fProton_Mass,2 ) ); 
    fProton_MomZ_Col                      = fProton_Mom_Col * TMath::Cos((fPi/180.0)*fProton_Theta_Col);    
    fProton_MomX_Col                      = ( fProton_Mom_Col * TMath::Sin((fPi/180.0)*fProton_Theta_Col) * 
					      TMath::Cos((fPi/180.0)*fProton_Phi_Col) );
    fProton_MomY_Col                      = ( fProton_Mom_Col * TMath::Sin((fPi/180.0)*fProton_Theta_Col) * 
					      TMath::Sin((fPi/180.0)*fProton_Phi_Col) );
    fBetaX_Col_RF                         = fProton_MomX_Col / fProton_Energy_Col;
    fBetaY_Col_RF                         = fProton_MomY_Col / fProton_Energy_Col;
    fBetaZ_Col_RF                         = fProton_MomZ_Col / fProton_Energy_Col;
    fBeta_Col_RF                          = TMath::Sqrt( TMath::Power( fBetaX_Col_RF , 2 ) + 
							 TMath::Power( fBetaY_Col_RF , 2 ) + 
							 TMath::Power( fBetaZ_Col_RF , 2 ) );
    fGamma_Col_RF                         = 1.0/TMath::Sqrt( 1 - TMath::Power(fBeta_Col_RF,2));

    fProton_Mom_Col_GeV                   = fProton_Mom_Col / 1000.0;
    fProton_Energy_Col_GeV                = fProton_Energy_Col / 1000.0;
    fProton_MomX_Col_GeV                  = fProton_MomX_Col / 1000.0;
    fProton_MomY_Col_GeV                  = fProton_MomY_Col / 1000.0;
    fProton_MomZ_Col_GeV                  = fProton_MomZ_Col / 1000.0;

    fTarget_Theta_Col                     = fProton_Theta_Col;
    fTarget_Phi_Col                       = fProton_Phi_Col;
    fTarget_Mom_Col                       = fProton_Mom_Col;
    fTarget_Energy_Col                    = fProton_Energy_Col;
    fTarget_MomX_Col                      = fProton_MomX_Col;
    fTarget_MomY_Col                      = fProton_MomY_Col;
    fTarget_MomZ_Col                      = fProton_MomZ_Col;
    fTarget_Mom_Col_GeV                   = fProton_Mom_Col_GeV;
    fTarget_Energy_Col_GeV                = fProton_Energy_Col_GeV;
    fTarget_MomX_Col_GeV                  = fProton_MomX_Col_GeV;
    fTarget_MomY_Col_GeV                  = fProton_MomY_Col_GeV;
    fTarget_MomZ_Col_GeV                  = fProton_MomZ_Col_GeV;

    double fTarget_Energy_Check = fTarget_Energy_Col;
    double fTarget_MomX_Check   = fTarget_MomX_Col;
    double fTarget_MomY_Check   = fTarget_MomY_Col;
    double fTarget_MomZ_Check   = fTarget_MomZ_Col;

    // ----------------------------------------------------
    // Incoming Electron in lab (collider) frame
    // ----------------------------------------------------
    fElectron_Energy_Col                  = fElectron_Kin_Col;

    if ( kCalcBremss && kCalcIon ) {
      fElectron_Targ_Thickness            = ( 370.0 + fVertex_Z ) * fHelium_Density;
      fElectron_Targ_Thickness_RadLen     = ( fElectron_Targ_Thickness ) / X0( fHelium_Z , fHelium_A );
      fElectron_Targ_BT                   = fElectron_Targ_Thickness_RadLen * b( fHelium_Z );
      fElectron_Targ_Bremss_Loss          = Bremsstrahlung( fElectron_Energy_Col , fElectron_Targ_BT );
      fElectron_Targ_Ion_Loss             = IonLoss( fElectron_Mass, fElectron_Energy_Col, fHelium_A, fHelium_Z, 
						     fHelium_Density, fElectron_Targ_Thickness );
      fElectron_Energy_Col                = fElectron_Energy_Col - fElectron_Targ_Ion_Loss - fElectron_Targ_Bremss_Loss;
    }
    
    fElectron_Mom_Col                     = TMath::Sqrt( TMath::Power(fElectron_Energy_Col , 2) - TMath::Power(fElectron_Mass , 2) );
    fElectron_Theta_Col                   = 0.0;
    fElectron_Phi_Col                     = 0.0;
    fElectron_MomZ_Col                    = fElectron_Mom_Col * TMath::Cos((fPi/180.0)*fElectron_Theta_Col);  
    fElectron_MomX_Col                    = ( fElectron_Mom_Col * TMath::Sin((fPi/180.0)*fElectron_Theta_Col) * 
					      TMath::Cos((fPi/180.0)*fElectron_Phi_Col) );
    fElectron_MomY_Col                    = ( fElectron_Mom_Col * TMath::Sin((fPi/180.0)*fElectron_Theta_Col) * 
					      TMath::Sin((fPi/180.0)*fElectron_Phi_Col) );  
    
    if ( fElectron_Energy_Col  < 0 )  { 
      fElectron_Energy_Col                = 1e-6;
      fElectron_Mom_Col                   = 1e-6;
      fElectron_MomZ_Col                  = 1e-6;
      fElectron_MomX_Col                  = 1e-6;
      fElectron_MomY_Col                  = 1e-6;
    }

    fElectron_Energy_Col_GeV              = fElectron_Energy_Col / 1000.0;
    fElectron_Mom_Col_GeV                 = fElectron_Mom_Col / 1000.0;
    fElectron_MomX_Col_GeV                = fElectron_MomX_Col / 1000.0;
    fElectron_MomY_Col_GeV                = fElectron_MomY_Col / 1000.0;
    fElectron_MomZ_Col_GeV                = fElectron_MomZ_Col / 1000.0;

    double fElectron_Energy_Check = fElectron_Energy_Col;
    double fElectron_MomX_Check   = fElectron_MomX_Col;
    double fElectron_MomY_Check   = fElectron_MomY_Col;
    double fElectron_MomZ_Check   = fElectron_MomZ_Col;

    fElectron_Corrected_Energy_Col        = fElectron_Energy_Col;
    fElectron_Corrected_Mom_Col           = fElectron_Mom_Col;
    fElectron_Corrected_MomX_Col          = fElectron_MomX_Col;
    fElectron_Corrected_MomY_Col          = fElectron_MomY_Col;
    fElectron_Corrected_MomZ_Col          = fElectron_MomZ_Col;
    fElectron_Corrected_Energy_Col_GeV    = fElectron_Energy_Col_GeV;
    fElectron_Corrected_Mom_Col_GeV       = fElectron_Mom_Col_GeV;
    fElectron_Corrected_MomX_Col_GeV      = fElectron_MomX_Col_GeV;
    fElectron_Corrected_MomY_Col_GeV      = fElectron_MomY_Col_GeV;
    fElectron_Corrected_MomZ_Col_GeV      = fElectron_MomZ_Col_GeV;


    if ( fElectron_Corrected_Energy_Col_GeV < 0 )  { 
      fElectron_Corrected_Energy_Col        = 0.0;
      fElectron_Corrected_Mom_Col           = 0.0;
      fElectron_Corrected_MomX_Col          = 0.0;
      fElectron_Corrected_MomY_Col          = 0.0;
      fElectron_Corrected_MomZ_Col          = 0.0;
      fElectron_Corrected_Energy_Col_GeV    = 0.0;
      fElectron_Corrected_Mom_Col_GeV       = 0.0;
      fElectron_Corrected_MomX_Col_GeV      = 0.0;
      fElectron_Corrected_MomY_Col_GeV      = 0.0;
      fElectron_Corrected_MomZ_Col_GeV      = 0.0;
    }

    // ---------------------------------------------------------------------
    // Specify the energy and solid angle of scatterd electron in lab frame
    // ---------------------------------------------------------------------
    fScatElec_Theta_Col            = ( 180.0 / fPi ) * TMath::ACos( fRandom->Uniform( cos( fScatElec_Theta_I * fPi / 180.0 ) , cos( fScatElec_Theta_F * fPi / 180.0 ) ) );
    fScatElec_Phi_Col              = fRandom->Uniform(0,360);
    fScatElec_Energy_Col           = fRandom->Uniform( fScatElec_E_Lo * fElectron_Energy_Col , fScatElec_E_Hi * fElectron_Energy_Col );
    fScatElec_Mom_Col              = TMath::Sqrt( TMath::Power( fScatElec_Energy_Col,2) - TMath::Power( fElectron_Mass , 2) );
    fScatElec_MomZ_Col             = fScatElec_Mom_Col * TMath::Cos((fPi/180.0)*fScatElec_Theta_Col);  
    fScatElec_MomX_Col             = ( fScatElec_Mom_Col * TMath::Sin((fPi/180.0)*fScatElec_Theta_Col) * 
				       TMath::Cos((fPi/180.0)*fScatElec_Phi_Col) );
    fScatElec_MomY_Col             = ( fScatElec_Mom_Col * TMath::Sin((fPi/180.0)*fScatElec_Theta_Col) * 
				       TMath::Sin((fPi/180.0)*fScatElec_Phi_Col) );
    // --------------------------------- Energy Corrections for Scattered Electron ----------------------------------------------

    fScatElec_Corrected_Theta_Col     = fScatElec_Theta_Col;
    fScatElec_Corrected_Phi_Col       = fScatElec_Phi_Col;
    fScatElec_Corrected_Energy_Col    = fScatElec_Energy_Col;
    fScatElec_Corrected_Mom_Col       = fScatElec_Mom_Col;
    fScatElec_Corrected_MomX_Col      = fScatElec_MomX_Col;
    fScatElec_Corrected_MomY_Col      = fScatElec_MomY_Col;
    fScatElec_Corrected_MomZ_Col      = fScatElec_MomZ_Col;
    fScatElec_Delta_Mom_Col           = 0;

    double fScatElec_Energy_Check = fScatElec_Energy_Col;
    double fScatElec_MomX_Check   = fScatElec_MomX_Col;
    double fScatElec_MomY_Check   = fScatElec_MomY_Col;
    double fScatElec_MomZ_Check   = fScatElec_MomZ_Col;
    
    if ( kCalcBremss && kCalcIon ) {

      // Double_t fWindow_Density          = 2.76;    // g/cm^3 
      // Double_t fWindow_Thickness        = 19.4246; // g/cm2 this is equal to my X0
      // Double_t fWindow_Thickness_RadLen = 7.038;   // cm
      // Double_t fWindow_Z                = 17;
      // Double_t fWindow_A                = 35;
      // X0 radiation length g cm−2 page 398 of review of particle physics
      // GE180 glass window, 2*120um, density 2.76g/cm3, X=2*120e-4/(19.4/2.76)=3.4e-3, Lumi=15e-6/1.6e-19*2*120e-4*2.76*6.02e23=3.74e36/cm2/s

      fScatElec_Targ_Thickness          = ( -1.0 * fVertex_Z - 330.0 ) * fHelium_Density; // g/cm2
      fScatElec_Targ_Thickness_RadLen   = ( fScatElec_Targ_Thickness ) / X0( fHelium_Z , fHelium_A ); // thickness in rad length, its a ratio
      fScatElec_Targ_BT                 = fScatElec_Targ_Thickness_RadLen * b( fHelium_Z );

      fTargWindow_Thickness                = 0.0120 * fWindow_Density; // g/cm2
      fTargWindow_Thickness_RadLen         = fTargWindow_Thickness / fWindow_Thickness;
      fTargWindow_BT                       = fTargWindow_Thickness_RadLen * b( fWindow_Z );
      
      fScatElec_Air_Thickness           = 450.0 * fAir_Density; 
      if ( fScatElec_Theta_Col < 16 ) {	
	fScatElec_Air_Thickness         = 950.0 * fAir_Density; 
      }
      fRadiation_Lenght_Air             = ( X0( 7, 14.007) * X0( 8, 16 ) ) / ( 0.766 * X0( 8, 15.999 ) + 0.233 * X0( 7, 14.007) ); // g / cm2
      fScatElec_Air_Thickness_RadLen    = fScatElec_Air_Thickness / fRadiation_Lenght_Air; // cm
      fScatElec_Air_BT                  = fScatElec_Air_Thickness_RadLen * b( fAir_Z ); // b is unitless

      fScatElec_TargWindow_Bremss_Loss  = Bremsstrahlung( fScatElec_Energy_Col , fTargWindow_BT );
      fScatElec_Targ_Bremss_Loss        = Bremsstrahlung( fScatElec_Energy_Col , fScatElec_Targ_BT );
      fScatElec_Air_Bremss_Loss         = Bremsstrahlung( fScatElec_Energy_Col , fScatElec_Air_BT );
      fScatElec_TargWindow_Ion_Loss     = IonLoss( fElectron_Mass, fScatElec_Energy_Col, fWindow_A, fWindow_Z, fWindow_Density, fTargWindow_Thickness );
      fScatElec_Targ_Ion_Loss           = IonLoss( fElectron_Mass, fScatElec_Energy_Col, fHelium_A, fHelium_Z, fHelium_Density, fScatElec_Targ_Thickness );
      fScatElec_Air_Ion_Loss            = IonLoss( fElectron_Mass, fScatElec_Energy_Col, fAir_A,    fAir_Z,    fAir_Density,    fScatElec_Air_Thickness );

      fScatElec_Corrected_Energy_Col    = ( fScatElec_Energy_Col - fScatElec_Targ_Bremss_Loss - fScatElec_Air_Bremss_Loss - fScatElec_TargWindow_Bremss_Loss -
					                           fScatElec_Targ_Ion_Loss    - fScatElec_Air_Ion_Loss    - fScatElec_TargWindow_Ion_Loss );      

      fScatElec_Corrected_Mom_Col       = TMath::Sqrt( TMath::Power( fScatElec_Corrected_Energy_Col , 2 ) - TMath::Power( fElectron_Mass , 2 ) );
      fScatElec_Delta_Mom_Col           = ( fScatElec_Mom_Col - fScatElec_Corrected_Mom_Col );
      fScatElec_Corrected_MomX_Col      = ( fScatElec_MomX_Col - fScatElec_Delta_Mom_Col *   
					    TMath::Sin((fPi/180.0) * fScatElec_Theta_Col ) * TMath::Cos((fPi/180.0) * fScatElec_Phi_Col ) );
      fScatElec_Corrected_MomY_Col      = ( fScatElec_MomY_Col - fScatElec_Delta_Mom_Col * 
					    TMath::Sin((fPi/180.0) * fScatElec_Theta_Col ) * TMath::Sin((fPi/180.0) * fScatElec_Phi_Col ));
      fScatElec_Corrected_MomZ_Col      = fScatElec_MomZ_Col - fScatElec_Delta_Mom_Col * TMath::Cos((fPi/180.0) * fScatElec_Theta_Col );
      fScatElec_Corrected_Theta_Col     = (180/fPi)* TMath::ATan( TMath::Sqrt( TMath::Power(fScatElec_Corrected_MomX_Col,2) + 
									       TMath::Power(fScatElec_Corrected_MomY_Col,2) ) / fScatElec_Corrected_MomZ_Col );      
      fScatElec_Corrected_Phi_Col   = (180/fPi)*TMath::ATan2(fScatElec_Corrected_MomY_Col,fScatElec_Corrected_MomX_Col);
      
      if ( ( fScatElec_Corrected_Theta_Col < 0 ) && ( fScatElec_Corrected_Phi_Col <= 0 ) ) 
	{ fScatElec_Corrected_Theta_Col = 180.0 + fScatElec_Corrected_Theta_Col;  fScatElec_Corrected_Phi_Col = 360 + fScatElec_Corrected_Phi_Col;} 
      if ( ( fScatElec_Corrected_Theta_Col > 0 ) && ( fScatElec_Corrected_Phi_Col < 0  ) ) 
	{ fScatElec_Corrected_Theta_Col = fScatElec_Corrected_Theta_Col;          fScatElec_Corrected_Phi_Col = 360 + fScatElec_Corrected_Phi_Col;}
      if ( ( fScatElec_Corrected_Theta_Col < 0 ) && ( fScatElec_Corrected_Phi_Col > 0 ) )  
	{ fScatElec_Corrected_Theta_Col = 180.0 + fScatElec_Corrected_Theta_Col;  fScatElec_Corrected_Phi_Col = fScatElec_Corrected_Phi_Col;}
      
    }


    if ( fScatElec_Corrected_Energy_Col < 0 )  { 
      fScatElec_Corrected_Energy_Col                = 1e-6;
      fScatElec_Corrected_Mom_Col                   = 1e-6;
      fScatElec_Corrected_MomZ_Col                  = 1e-6;
      fScatElec_Corrected_MomX_Col                  = 1e-6;
      fScatElec_Corrected_MomY_Col                  = 1e-6;
    }

    fScatElec_Corrected_Energy_Col_GeV    = fScatElec_Corrected_Energy_Col / 1000.0;
    fScatElec_Corrected_Mom_Col_GeV       = fScatElec_Corrected_Mom_Col / 1000.0;
    fScatElec_Delta_Mom_Col_GeV           = fScatElec_Delta_Mom_Col / 1000.0;
    fScatElec_Corrected_MomX_Col_GeV      = fScatElec_Corrected_MomX_Col / 1000.0;
    fScatElec_Corrected_MomY_Col_GeV      = fScatElec_Corrected_MomY_Col / 1000.0;
    fScatElec_Corrected_MomZ_Col_GeV      = fScatElec_Corrected_MomZ_Col / 1000.0;

    fScatElec_Energy_Col_GeV              = fScatElec_Energy_Col / 1000.0;
    fScatElec_Mom_Col_GeV                 = fScatElec_Mom_Col / 1000.0;
    fScatElec_MomX_Col_GeV                = fScatElec_MomX_Col / 1000.0;
    fScatElec_MomY_Col_GeV                = fScatElec_MomY_Col / 1000.0;
    fScatElec_MomZ_Col_GeV                = fScatElec_MomZ_Col / 1000.0;

    // --------------------------------- Energy Corrections for Scattered Electron ----------------------------------------------


    // ----------------------------------------------------
    // Photon in lab frame
    // ----------------------------------------------------    
    fPhoton_Energy_Col          = fElectron_Energy_Col - fScatElec_Energy_Col;

    fPhoton_Mom_Col             = ( TMath::Sqrt( std::abs( TMath::Power( fElectron_Mom_Col , 2 ) +
     							   TMath::Power( fScatElec_Mom_Col , 2 ) - 
     							   2.0 * fElectron_Mom_Col * fScatElec_Mom_Col * 
     							   TMath::Cos( ( fPi / 180.0 ) * ( fScatElec_Theta_Col ) ) ) ) );    

    fPhoton_MomX_Col            = fElectron_MomX_Col - fScatElec_MomX_Col;
    fPhoton_MomY_Col            = fElectron_MomY_Col - fScatElec_MomY_Col;
    fPhoton_MomZ_Col            = fElectron_MomZ_Col - fScatElec_MomZ_Col;

    // fPhoton_Mom_Col             = TMath::Sqrt( TMath::Power( fPhoton_MomX_Col , 2 ) + 
    // 					       TMath::Power( fPhoton_MomY_Col , 2 ) + 
    // 					       TMath::Power( fPhoton_MomZ_Col , 2 ) );      


    fPhoton_Energy_Col_GeV      = fPhoton_Energy_Col / 1000.0;
    fPhoton_Mom_Col_GeV         = fPhoton_Mom_Col    / 1000.0;
    fPhoton_MomX_Col_GeV        = fPhoton_MomX_Col   / 1000.0;
    fPhoton_MomY_Col_GeV        = fPhoton_MomY_Col   / 1000.0;
    fPhoton_MomZ_Col_GeV        = fPhoton_MomZ_Col   / 1000.0;

    fPhoton_Theta_Col       = (180.0/fPi)*TMath::ATan( TMath::Sqrt( TMath::Power(fPhoton_MomX_Col,2) + 
								    TMath::Power(fPhoton_MomY_Col,2))/ fPhoton_MomZ_Col);    
    fPhoton_Phi_Col         = (180/fPi)*TMath::ATan2(fPhoton_MomY_Col,fPhoton_MomX_Col);
    
    if ( ( fPhoton_Theta_Col < 0 ) && ( fPhoton_Phi_Col <= 0 ) ) 
      { fPhoton_Theta_Col = 180.0 + fPhoton_Theta_Col;  fPhoton_Phi_Col = 360 + fPhoton_Phi_Col;}    
    if ( ( fPhoton_Theta_Col > 0 ) && ( fPhoton_Phi_Col < 0  ) ) 
      { fPhoton_Theta_Col = fPhoton_Theta_Col;          fPhoton_Phi_Col = 360 + fPhoton_Phi_Col;}    
    if ( ( fPhoton_Theta_Col < 0 ) && ( fPhoton_Phi_Col > 0  ) ) 
      { fPhoton_Theta_Col = 180.0 + fPhoton_Theta_Col;  fPhoton_Phi_Col = fPhoton_Phi_Col;}
    
    // ----------------------------------------------------
    // Qsq, Energy and magnitude of Momentum of Virtual Photon in Proton's rest frame
    // ----------------------------------------------------    
    fQsq_GeV                    = TMath::Power( fPhoton_Mom_Col_GeV ,2 ) -  TMath::Power( fPhoton_Energy_Col_GeV ,2 );    
    fQsq                        = fQsq_GeV * 1000000.0;
    
    ftestsig = -1.0 * ( TMath::Power( fElectron_Mass_GeV , 2 ) + TMath::Power( fElectron_Mass_GeV , 2 ) -
			2.0 * ( fElectron_Energy_Col_GeV * fScatElec_Energy_Col_GeV - 
				fElectron_MomX_Col_GeV   * fScatElec_MomX_Col_GeV - 
				fElectron_MomY_Col_GeV   * fScatElec_MomY_Col_GeV - 
				fElectron_MomZ_Col_GeV   * fScatElec_MomZ_Col_GeV ) );
    
    // ----------------------------------------------------
    // W square, Invariant Mass (P_g + P_p)^2
    // ----------------------------------------------------
    fWSq_GeV    = TMath::Power( fProton_Mass_GeV , 2 ) - fQsq_GeV +  2.0 * ( fPhoton_Energy_Col_GeV * fProton_Energy_Col_GeV - 
									     fPhoton_MomX_Col_GeV   * fProton_MomX_Col_GeV - 
									     fPhoton_MomY_Col_GeV   * fProton_MomY_Col_GeV - 
									     fPhoton_MomZ_Col_GeV   * fProton_MomZ_Col_GeV );        

    if ( fWSq_GeV < 0 ) { 
      fWSqNeg ++;
      continue;
    }
    
    // ----------------------------------------------------
    // Pion in Col frame
    // ----------------------------------------------------    
    fBot_Pion_Mom = -1;
    fTop_Pion_Mom = -1;

    double pionStep  = 1;    // 2 * 1MeV = 2 MeV
    double myMom     = 0.0;
    double pionLarge = 0.0;
    double pionSmall = 0.0;
    double fPionDiff = 1e-3; // 1e-3 * 1MeV = 1000 eV
    
    fPion_Theta_Col            = ( 180.0 / fPi ) * TMath::ACos( fRandom->Uniform( cos( fPion_Theta_I * fPi / 180.0 ) ,
										  cos( fPion_Theta_F * fPi / 180.0 ) ) );
    fPion_Phi_Col              = fRandom->Uniform(0,360);

    myMom = 0.0 - pionStep;
    while( myMom <= 11000.0 ){
      fPion_Mom_Col            = myMom;
      fPion_MomZ_Col           = fPion_Mom_Col * TMath::Cos((fPi/180.0)*fPion_Theta_Col);
      fPion_MomX_Col           = fPion_Mom_Col * TMath::Sin((fPi/180.0)*fPion_Theta_Col) * TMath::Cos((fPi/180.0)*fPion_Phi_Col);
      fPion_MomY_Col           = fPion_Mom_Col * TMath::Sin((fPi/180.0)*fPion_Theta_Col) * TMath::Sin((fPi/180.0)*fPion_Phi_Col);
      fPion_Energy_Col         = TMath::Sqrt( TMath::Power( fPion_Mom_Col , 2) + TMath::Power( fPion_Mass , 2) );    
      fPion_Mom_Col_GeV        = fPion_Mom_Col / 1000.0;

      fPion_MomZ_Col_GeV       = fPion_MomZ_Col / 1000.0;
      fPion_MomX_Col_GeV       = fPion_MomX_Col / 1000.0;
      fPion_MomY_Col_GeV       = fPion_MomY_Col / 1000.0;
      fPion_Energy_Col_GeV     = fPion_Energy_Col / 1000.0;

      fNeutron_MomX_Col        = fProton_MomX_Col + fElectron_MomX_Col - fScatElec_MomX_Col - fPion_MomX_Col;
      fNeutron_MomY_Col        = fProton_MomY_Col + fElectron_MomY_Col - fScatElec_MomY_Col - fPion_MomY_Col;
      fNeutron_MomZ_Col        = fProton_MomZ_Col + fElectron_MomZ_Col - fScatElec_MomZ_Col - fPion_MomZ_Col;
      fNeutron_Mom_Col         = TMath::Sqrt( TMath::Power( fNeutron_MomX_Col , 2 ) + 
					      TMath::Power( fNeutron_MomY_Col , 2 ) + 
					      TMath::Power( fNeutron_MomZ_Col , 2 ) );
      fNeutron_Energy_Col      = TMath::Sqrt( TMath::Power( fNeutron_Mom_Col , 2 ) + TMath::Power( fNeutron_Mass ,2 ) );

      fNeutron_Theta_Col  = (180/fPi)*TMath::ATan( TMath::Sqrt( TMath::Power(fNeutron_MomX_Col,2) + 
								TMath::Power(fNeutron_MomY_Col,2) ) / fNeutron_MomZ_Col );
      fNeutron_Phi_Col    = (180/fPi)*TMath::ATan2(fNeutron_MomY_Col,fNeutron_MomX_Col); 
      
      if ( ( fNeutron_Theta_Col < 0 ) && ( fNeutron_Phi_Col <= 0 ) ) 
	{ fNeutron_Theta_Col = 180.0 + fNeutron_Theta_Col;  fNeutron_Phi_Col = 360 + fNeutron_Phi_Col;}    
      if ( ( fNeutron_Theta_Col > 0 ) && ( fNeutron_Phi_Col < 0  ) ) 
	{ fNeutron_Theta_Col = fNeutron_Theta_Col;          fNeutron_Phi_Col = 360 + fNeutron_Phi_Col;}    
      if ( ( fNeutron_Theta_Col < 0 ) && ( fNeutron_Phi_Col > 0  ) ) 
	{ fNeutron_Theta_Col = 180.0 + fNeutron_Theta_Col;  fNeutron_Phi_Col = fNeutron_Phi_Col;}
      
      fNeutron_Mom_Col_GeV     = fNeutron_Mom_Col / 1000.0;
      fNeutron_MomZ_Col_GeV    = fNeutron_MomZ_Col / 1000.0;
      fNeutron_MomX_Col_GeV    = fNeutron_MomX_Col / 1000.0;
      fNeutron_MomY_Col_GeV    = fNeutron_MomY_Col / 1000.0;
      fNeutron_Energy_Col_GeV  = fNeutron_Energy_Col / 1000.0;

      fWSq_PiN_GeV            = TMath::Power( fPion_Mass_GeV    , 2 ) + TMath::Power( fNeutron_Mass_GeV , 2 ) + 
	2.0 * ( fPion_Energy_Col_GeV * fNeutron_Energy_Col_GeV - 
		fPion_MomX_Col_GeV   * fNeutron_MomX_Col_GeV - 
		fPion_MomY_Col_GeV   * fNeutron_MomY_Col_GeV - 
		fPion_MomZ_Col_GeV   * fNeutron_MomZ_Col_GeV );
      
      if ( std::abs( std::abs(fWSq_PiN_GeV) - std::abs(fWSq_GeV) ) < fPionDiff ){
	pionLarge = fPion_Mom_Col;
	break;
      }
      myMom = myMom + pionStep;
    }

    fBot_Pion_Mom = fPion_Mom_Col;

    // -------------------------------------------- Second Loop ---------------------------------------------------
      
    myMom = 11000.0 + pionStep;
    while( myMom >= 0.0 ){
      fPion_Mom_Col            = myMom;
      fPion_MomZ_Col           = fPion_Mom_Col * TMath::Cos((fPi/180.0)*fPion_Theta_Col);
      fPion_MomX_Col           = fPion_Mom_Col * TMath::Sin((fPi/180.0)*fPion_Theta_Col) * TMath::Cos((fPi/180.0)*fPion_Phi_Col);
      fPion_MomY_Col           = fPion_Mom_Col * TMath::Sin((fPi/180.0)*fPion_Theta_Col) * TMath::Sin((fPi/180.0)*fPion_Phi_Col);
      fPion_Energy_Col         = TMath::Sqrt( TMath::Power( fPion_Mom_Col , 2) + TMath::Power( fPion_Mass , 2) );    
      fPion_Mom_Col_GeV        = fPion_Mom_Col / 1000.0;
      fPion_MomZ_Col_GeV       = fPion_MomZ_Col / 1000.0;
      fPion_MomX_Col_GeV       = fPion_MomX_Col / 1000.0;
      fPion_MomY_Col_GeV       = fPion_MomY_Col / 1000.0;
      fPion_Energy_Col_GeV     = fPion_Energy_Col / 1000.0;

      fNeutron_MomX_Col        = fProton_MomX_Col + fElectron_MomX_Col - fScatElec_MomX_Col - fPion_MomX_Col;
      fNeutron_MomY_Col        = fProton_MomY_Col + fElectron_MomY_Col - fScatElec_MomY_Col - fPion_MomY_Col;
      fNeutron_MomZ_Col        = fProton_MomZ_Col + fElectron_MomZ_Col - fScatElec_MomZ_Col - fPion_MomZ_Col;
      fNeutron_Mom_Col         = TMath::Sqrt( TMath::Power( fNeutron_MomX_Col , 2 ) + 
					      TMath::Power( fNeutron_MomY_Col , 2 ) + 
					      TMath::Power( fNeutron_MomZ_Col , 2 ) );
      fNeutron_Energy_Col      = TMath::Sqrt( TMath::Power( fNeutron_Mom_Col , 2 ) + TMath::Power( fNeutron_Mass ,2 ) );

      fNeutron_Theta_Col  = (180/fPi)*TMath::ATan( TMath::Sqrt( TMath::Power(fNeutron_MomX_Col,2) + 
								TMath::Power(fNeutron_MomY_Col,2) ) / fNeutron_MomZ_Col );
      fNeutron_Phi_Col    = (180/fPi)*TMath::ATan2(fNeutron_MomY_Col,fNeutron_MomX_Col); 
      
      if ( ( fNeutron_Theta_Col < 0 ) && ( fNeutron_Phi_Col <= 0 ) ) 
	{ fNeutron_Theta_Col = 180.0 + fNeutron_Theta_Col;  fNeutron_Phi_Col = 360 + fNeutron_Phi_Col;}    
      if ( ( fNeutron_Theta_Col > 0 ) && ( fNeutron_Phi_Col < 0  ) ) 
	{ fNeutron_Theta_Col = fNeutron_Theta_Col;          fNeutron_Phi_Col = 360 + fNeutron_Phi_Col;}    
      if ( ( fNeutron_Theta_Col < 0 ) && ( fNeutron_Phi_Col > 0  ) ) 
	{ fNeutron_Theta_Col = 180.0 + fNeutron_Theta_Col;  fNeutron_Phi_Col = fNeutron_Phi_Col;}
      
      fNeutron_Mom_Col_GeV     = fNeutron_Mom_Col / 1000.0;
      fNeutron_MomZ_Col_GeV    = fNeutron_MomZ_Col / 1000.0;
      fNeutron_MomX_Col_GeV    = fNeutron_MomX_Col / 1000.0;
      fNeutron_MomY_Col_GeV    = fNeutron_MomY_Col / 1000.0;
      fNeutron_Energy_Col_GeV  = fNeutron_Energy_Col / 1000.0;

      fWSq_PiN_GeV            = TMath::Power( fPion_Mass_GeV    , 2 ) + TMath::Power( fNeutron_Mass_GeV , 2 ) + 
	2.0 * ( fPion_Energy_Col_GeV * fNeutron_Energy_Col_GeV - 
		fPion_MomX_Col_GeV   * fNeutron_MomX_Col_GeV - 
		fPion_MomY_Col_GeV   * fNeutron_MomY_Col_GeV - 
		fPion_MomZ_Col_GeV   * fNeutron_MomZ_Col_GeV );
      
      if ( std::abs( std::abs(fWSq_PiN_GeV) - std::abs(fWSq_GeV) ) < fPionDiff ){
	pionLarge = fPion_Mom_Col;
	break;
      }
      myMom = myMom - pionStep;
    }
    
    fTop_Pion_Mom = fPion_Mom_Col;

    if ( ( fBot_Pion_Mom == 11000 ) && ( fTop_Pion_Mom == 0 ) ) {
      continue;
    };

    
    fPion_Mom_Same = 0;
    if ( fBot_Pion_Mom == fTop_Pion_Mom ){ fPion_Mom_Same = 1; }
    if ( fPion_Mom_Same == 0 ){ 
      if ( fRandom->Uniform( 0, 1 ) <  0.5 ) { fPion_Mom_Col = fBot_Pion_Mom; }
      if ( fRandom->Uniform( 0, 1 ) >= 0.5 ) { fPion_Mom_Col = fTop_Pion_Mom; }
    }

    // -------------------------------------------- Second Loop ---------------------------------------------------

    // fNeutron_Energy_Col_GeV                       = fNeutron_Energy_Col / 1000.0;
    // fNeutron_Mom_Col_GeV                          = fNeutron_Mom_Col / 1000.0;
    // fNeutron_MomX_Col_GeV                         = fNeutron_MomX_Col / 1000.0;
    // fNeutron_MomY_Col_GeV                         = fNeutron_MomY_Col / 1000.0;
    // fNeutron_MomZ_Col_GeV                         = fNeutron_MomZ_Col / 1000.0;

    // fRecoilProton_Theta_Col                       = fNeutron_Theta_Col;
    // fRecoilProton_Phi_Col                         = fNeutron_Phi_Col;
    // fRecoilProton_Energy_Col                      = fNeutron_Energy_Col;
    // fRecoilProton_Mom_Col                         = fNeutron_Mom_Col;
    // fRecoilProton_MomX_Col                        = fNeutron_MomX_Col;
    // fRecoilProton_MomY_Col                        = fNeutron_MomY_Col;
    // fRecoilProton_MomZ_Col                        = fNeutron_MomZ_Col;

    // fRecoilProton_Energy_Col_GeV                  = fRecoilProton_Energy_Col / 1000.0;
    // fRecoilProton_Mom_Col_GeV                     = fRecoilProton_Mom_Col / 1000.0;
    // fRecoilProton_MomX_Col_GeV                    = fRecoilProton_MomX_Col / 1000.0;
    // fRecoilProton_MomY_Col_GeV                    = fRecoilProton_MomY_Col / 1000.0;
    // fRecoilProton_MomZ_Col_GeV                    = fRecoilProton_MomZ_Col / 1000.0;


    //-----------------------------------------------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------------------------------------------

    fPhaseShiftWeight = 1.0;
    if ( kFSI ) {
    
      fFSIProton_Theta_Col      = 0.0;
      fFSIProton_Phi_Col        = 0.0;
      fFSIProton_Mom_Col        = 1e-6;    

      if  ( kCalcFermi ) {
	fFSIProton_Mom_Col      = myPim.fermiMomentum();
	fFSIProton_Theta_Col    = ( 180.0 / fPi ) * TMath::ACos( fRandom->Uniform( cos(0.0 * fPi/180.0) ,
										   cos(180.0 * fPi/180.0) ) );
	fFSIProton_Phi_Col      = fRandom->Uniform( 0 , 360 );
      }

      fFSIProton_Energy_Col     = TMath::Sqrt( TMath::Power( fFSIProton_Mom_Col , 2 ) + TMath::Power(fProton_Mass,2 ) ); 
      fFSIProton_MomZ_Col       = fFSIProton_Mom_Col * TMath::Cos((fPi/180.0)*fFSIProton_Theta_Col);    
      fFSIProton_MomX_Col       = ( fFSIProton_Mom_Col * TMath::Sin((fPi/180.0)*fFSIProton_Theta_Col) * TMath::Cos((fPi/180.0)*fFSIProton_Phi_Col) );
      fFSIProton_MomY_Col       = ( fFSIProton_Mom_Col * TMath::Sin((fPi/180.0)*fFSIProton_Theta_Col) * TMath::Sin((fPi/180.0)*fFSIProton_Phi_Col) );

      fFSIProton_Mom_Col_GeV    = fFSIProton_Mom_Col / 1000.0;
      fFSIProton_Energy_Col_GeV = fFSIProton_Energy_Col / 1000.0;
      fFSIProton_MomX_Col_GeV   = fFSIProton_MomX_Col / 1000.0;
      fFSIProton_MomY_Col_GeV   = fFSIProton_MomY_Col / 1000.0;
      fFSIProton_MomZ_Col_GeV   = fFSIProton_MomZ_Col / 1000.0;

      TLorentzVector target( fFSIProton_MomX_Col_GeV, fFSIProton_MomY_Col_GeV, fFSIProton_MomZ_Col_GeV, fFSIProton_Energy_Col_GeV );
      TLorentzVector beam(   fPion_MomX_Col_GeV ,     fPion_MomY_Col_GeV ,     fPion_MomZ_Col_GeV ,     fPion_Energy_Col_GeV );
    
      TLorentzVector fS = target + beam;
      double fMandS = fS.Mag2();  
      TLorentzVector W = beam + target;
      Double_t masses[2] = { 0.938, 0.139} ;
      SetDecay(W, 2, masses, "fermi");
      TLorentzVector pProton;
      TLorentzVector pPim;
      TLorentzVector pProtoncm;
      TLorentzVector pPimcm;  
  
      fPhaseSpaceWeight = Generate();
      pProton           = GetDecay(0);
      pPim              = GetDecay(1);
      pProtoncm         = GetProtonCM();
      pPimcm            = GetPionCM();

      double pPionVecCM = sqrt( pow( pPimcm.Px() , 2 ) + pow( pPimcm.Py() , 2 ) + pow( pPimcm.Pz() , 2 ) ); 
      double fCosCM     = pPimcm.Px()/pPionVecCM;
    
      phaseshifts( 2 , pPionVecCM , fMandS );  

      double fZ0 = getZ0();
      double fZ1 = getZ1();
      double fZ2 = getZ2();
      
      fPhaseShiftWeight = fZ0 + fZ1 * fCosCM + fZ2 * ( pow ( fCosCM , 2 ) );
      fPhaseShiftWeight = fPhaseShiftWeight * fPhaseSpaceWeight;    

      // fPionTh = pPim.Theta() * 57.29;

      fPion_MomX_Col           = pPim.X();
      fPion_MomY_Col           = pPim.Y();
      fPion_MomZ_Col           = pPim.Z();
      fPion_Energy_Col         = pPim.T();
      fPion_Mom_Col            = pPim.M();

      fPion_Mom_Col_GeV        = fPion_Mom_Col / 1000.0;
      fPion_MomZ_Col_GeV       = fPion_MomZ_Col / 1000.0;
      fPion_MomX_Col_GeV       = fPion_MomX_Col / 1000.0;
      fPion_MomY_Col_GeV       = fPion_MomY_Col / 1000.0;
      fPion_Energy_Col_GeV     = fPion_Energy_Col / 1000.0;
      
      fPion_Theta_Col  = (180/fPi)*TMath::ATan( TMath::Sqrt( TMath::Power(fPion_MomX_Col,2) + TMath::Power(fPion_MomY_Col,2) ) / fPion_MomZ_Col );
      fPion_Phi_Col    = (180/fPi)*TMath::ATan2(fPion_MomY_Col,fPion_MomX_Col); 
      
      if ( ( fPion_Theta_Col < 0 ) && ( fPion_Phi_Col <= 0 ) ) 	{ fPion_Theta_Col = 180.0 + fPion_Theta_Col;  fPion_Phi_Col = 360 + fPion_Phi_Col;}    
      if ( ( fPion_Theta_Col > 0 ) && ( fPion_Phi_Col < 0  ) ) 	{ fPion_Theta_Col = fPion_Theta_Col;          fPion_Phi_Col = 360 + fPion_Phi_Col;}    
      if ( ( fPion_Theta_Col < 0 ) && ( fPion_Phi_Col > 0  ) ) 	{ fPion_Theta_Col = 180.0 + fPion_Theta_Col;  fPion_Phi_Col = fPion_Phi_Col;}

    }

    //-----------------------------------------------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------------------------------------------

    // --------------------------------- Energy Corrections for Pion ----------------------------------------------

    fPion_Corrected_Theta_Col     =     fPion_Theta_Col;
    fPion_Corrected_Phi_Col       =     fPion_Phi_Col;
    fPion_Corrected_Energy_Col    =     fPion_Energy_Col;
    fPion_Corrected_Mom_Col       =     fPion_Mom_Col;
    fPion_Corrected_MomX_Col      =     fPion_MomX_Col;
    fPion_Corrected_MomY_Col      =     fPion_MomY_Col;
    fPion_Corrected_MomZ_Col      =     fPion_MomZ_Col;
    fPion_Delta_Mom_Col           =     0;

    double fPion_Energy_Check = fPion_Energy_Col;
    double fPion_MomX_Check   = fPion_MomX_Col;
    double fPion_MomY_Check   = fPion_MomY_Col;
    double fPion_MomZ_Check   = fPion_MomZ_Col;

    if ( kCalcBremss && kCalcIon ) {
    
      fPion_Targ_Thickness          = ( -1.0*fVertex_Z - 330.0 ) * fHelium_Density;
      // fPion_Targ_Thickness_RadLen   = ( fPion_Targ_Thickness ) / X0( fHelium_Z , fHelium_A );
      // fPion_Targ_BT                 = fPion_Targ_Thickness_RadLen * b( fHelium_Z );

      fTargWindow_Thickness          = 0.0120 * fWindow_Density; // g/cm2
      // fTargWindow_Thickness_RadLen  = fTargWindow_Thickness / fWindow_Thickness;
      // fTargWindow_BT                = fTargWindow_Thickness_RadLen * b( fWindow_Z );

      fPion_Air_Thickness           = 450.0 * fAir_Density; 
      if ( fPion_Theta_Col < 16 ) {
	fPion_Air_Thickness         = 950.0 * fAir_Density;
      }    

      // fRadiation_Lenght_Air         = ( X0( 7, 14.007) * X0( 8, 16 ) ) / ( 0.766 * X0( 8, 15.999 ) + 0.233 * X0( 7, 14.007) );
      // fPion_Air_Thickness_RadLen    = fPion_Air_Thickness / fRadiation_Lenght_Air;
      // fPion_Air_BT                  = fPion_Air_Thickness_RadLen * b( fAir_Z );

      // fPion_Air_Bremss_Loss         = Bremsstrahlung( fPion_Energy_Col , fPion_Air_BT );
      // fPion_TargWindow_Bremss_Loss  = Bremsstrahlung( fPion_Energy_Col , fTargWindow_BT );
      // fPion_Targ_Bremss_Loss        = Bremsstrahlung( fPion_Energy_Col , fPion_Targ_BT );

      fPion_TargWindow_Ion_Loss     = IonLoss( fPion_Mass, fPion_Energy_Col, fWindow_A, fWindow_Z, fWindow_Density, fTargWindow_Thickness );
      fPion_Targ_Ion_Loss           = IonLoss( fPion_Mass, fPion_Energy_Col, fHelium_A, fHelium_Z, fHelium_Density, fPion_Targ_Thickness );
      fPion_Air_Ion_Loss            = IonLoss( fPion_Mass, fPion_Energy_Col, fAir_A,    fAir_Z,    fAir_Density,    fPion_Air_Thickness );

      // fPion_Corrected_Energy_Col    = fPion_Energy_Col - fPion_Targ_Bremss_Loss - fPion_Air_Bremss_Loss - fPion_Targ_Ion_Loss - fPion_Air_Ion_Loss;
      fPion_Corrected_Energy_Col    = fPion_Energy_Col - fPion_Targ_Ion_Loss - fPion_Air_Ion_Loss - fPion_TargWindow_Ion_Loss;
    
      fPion_Corrected_Mom_Col       = TMath::Sqrt( TMath::Power( fPion_Corrected_Energy_Col , 2 ) - TMath::Power( fPion_Mass , 2 ) );
      fPion_Delta_Mom_Col           = ( fPion_Mom_Col - fPion_Corrected_Mom_Col );
      fPion_Corrected_MomX_Col      = fPion_MomX_Col - fPion_Delta_Mom_Col * TMath::Sin((fPi/180.0) * fPion_Theta_Col ) *  TMath::Cos((fPi/180.0) * fPion_Phi_Col );
      fPion_Corrected_MomY_Col      = fPion_MomY_Col - fPion_Delta_Mom_Col * TMath::Sin((fPi/180.0) * fPion_Theta_Col ) *  TMath::Sin((fPi/180.0) * fPion_Phi_Col );
      fPion_Corrected_MomZ_Col      = fPion_MomZ_Col - fPion_Delta_Mom_Col * TMath::Cos((fPi/180.0) * fPion_Theta_Col );
      fPion_Corrected_Theta_Col     = (180/fPi)* TMath::ATan( TMath::Sqrt( TMath::Power(fPion_Corrected_MomX_Col,2) + 
									   TMath::Power(fPion_Corrected_MomY_Col,2) ) / fPion_Corrected_MomZ_Col );

      fPion_Corrected_Theta_Col     = (180/fPi)* TMath::ATan( TMath::Sqrt( TMath::Power(fPion_Corrected_MomX_Col,2) + 
									   TMath::Power(fPion_Corrected_MomY_Col,2) ) / fPion_Corrected_MomZ_Col );
      
      fPion_Corrected_Phi_Col       = (180/fPi)*TMath::ATan2(fPion_Corrected_MomY_Col,fPion_Corrected_MomX_Col);
      
      if ( ( fPion_Corrected_Theta_Col < 0 ) && ( fPion_Corrected_Phi_Col <= 0 ) ) 
	{ fPion_Corrected_Theta_Col = 180.0 + fPion_Corrected_Theta_Col;  fPion_Corrected_Phi_Col = 360 + fPion_Corrected_Phi_Col;} 
      if ( ( fPion_Corrected_Theta_Col > 0 ) && ( fPion_Corrected_Phi_Col < 0  ) ) 
	{ fPion_Corrected_Theta_Col = fPion_Corrected_Theta_Col;          fPion_Corrected_Phi_Col = 360 + fPion_Corrected_Phi_Col;}
      if ( ( fPion_Corrected_Theta_Col < 0 ) && ( fPion_Corrected_Phi_Col > 0 ) )  
	{ fPion_Corrected_Theta_Col = 180.0 + fPion_Corrected_Theta_Col;  fPion_Corrected_Phi_Col = fPion_Corrected_Phi_Col;}

    }

    if ( fPion_Corrected_Energy_Col < 0 )  { 
      fPion_Corrected_Energy_Col                = 1e-6;
      fPion_Corrected_Mom_Col                   = 1e-6;
      fPion_Corrected_MomZ_Col                  = 1e-6;
      fPion_Corrected_MomX_Col                  = 1e-6;
      fPion_Corrected_MomY_Col                  = 1e-6;
    }
    
    fPion_Corrected_Energy_Col_GeV    = fPion_Corrected_Energy_Col / 1000.0;
    fPion_Corrected_Mom_Col_GeV       = fPion_Corrected_Mom_Col / 1000.0;
    fPion_Delta_Mom_Col_GeV           = fPion_Delta_Mom_Col / 1000.0;
    fPion_Corrected_MomX_Col_GeV      = fPion_Corrected_MomX_Col / 1000.0;
    fPion_Corrected_MomY_Col_GeV      = fPion_Corrected_MomY_Col / 1000.0;
    fPion_Corrected_MomZ_Col_GeV      = fPion_Corrected_MomZ_Col / 1000.0;

    // --------------------------------- Energy Corrections for Pion ----------------------------------------------


    // --------------------------------- Energy Corrections for Recoil Proton ----------------------------------------------
        
    fNeutron_Corrected_Theta_Col     =     fNeutron_Theta_Col;
    fNeutron_Corrected_Phi_Col       =     fNeutron_Phi_Col;
    fNeutron_Corrected_Energy_Col    =     fNeutron_Energy_Col;
    fNeutron_Corrected_Mom_Col       =     fNeutron_Mom_Col;
    fNeutron_Corrected_MomX_Col      =     fNeutron_MomX_Col;
    fNeutron_Corrected_MomY_Col      =     fNeutron_MomY_Col;
    fNeutron_Corrected_MomZ_Col      =     fNeutron_MomZ_Col;
    fNeutron_Delta_Mom_Col           =     0;

    double fRecoilProton_Energy_Check = fNeutron_Energy_Col;
    double fRecoilProton_MomX_Check   = fNeutron_MomX_Col;
    double fRecoilProton_MomY_Check   = fNeutron_MomY_Col;
    double fRecoilProton_MomZ_Check   = fNeutron_MomZ_Col;

    if ( kCalcBremss && kCalcIon ) {

      fNeutron_Targ_Thickness          = ( -1.0*fVertex_Z - 330.0 ) * fHelium_Density;
      // fNeutron_Targ_Thickness_RadLen   = ( fNeutron_Targ_Thickness ) / X0( fHelium_Z , fHelium_A );
      // fNeutron_Targ_BT                 = fNeutron_Targ_Thickness_RadLen * b( fHelium_Z );

      fTargWindow_Thickness             = 0.0120 * fWindow_Density; // g/cm2
      // fTargWindow_Thickness_RadLen  = fTargWindow_Thickness / fWindow_Thickness;
      // fTargWindow_BT                = fTargWindow_Thickness_RadLen * b( fWindow_Z );


      fNeutron_Air_Thickness           = 450.0 * fAir_Density; 
      if ( fNeutron_Theta_Col < 16 ) {
	fNeutron_Air_Thickness         = 950.0 * fAir_Density;
      }

      // fRadiation_Lenght_Air            = ( X0( 7, 14.007) * X0( 8, 16 ) ) / ( 0.766 * X0( 8, 15.999 ) + 0.233 * X0( 7, 14.007) );
      // fNeutron_Air_Thickness_RadLen    = fNeutron_Air_Thickness / fRadiation_Lenght_Air;
      // fNeutron_Air_BT                  = fNeutron_Air_Thickness_RadLen * b( fAir_Z );

      // fNeutron_Air_Bremss_Loss         = Bremsstrahlung( fNeutron_Energy_Col , fNeutron_Air_BT );
      // fNeutron_Targ_Bremss_Loss        = Bremsstrahlung( fNeutron_Energy_Col , fNeutron_Targ_BT );
      // fNeutron_TargWindow_Bremss_Loss  = Bremsstrahlung( fNeutron_Energy_Col , fTargWindow_BT );

      fNeutron_TargWindow_Ion_Loss     = IonLoss( fNeutron_Mass, fNeutron_Energy_Col, fWindow_A, fWindow_Z, fWindow_Density, fTargWindow_Thickness );
      fNeutron_Targ_Ion_Loss           = IonLoss( fNeutron_Mass, fNeutron_Energy_Col, fHelium_A, fHelium_Z, fHelium_Density, fNeutron_Targ_Thickness );
      fNeutron_Air_Ion_Loss            = IonLoss( fNeutron_Mass, fNeutron_Energy_Col, fAir_A,    fAir_Z,    fAir_Density,    fNeutron_Air_Thickness );

      // fNeutron_Corrected_Energy_Col    = fNeutron_Energy_Col - fNeutron_Targ_Bremss_Loss - fNeutron_Air_Bremss_Loss - 
      // 	fNeutron_Targ_Ion_Loss -    fNeutron_Air_Ion_Loss;
      fNeutron_Corrected_Energy_Col    = fNeutron_Energy_Col - fNeutron_Targ_Ion_Loss - fNeutron_Air_Ion_Loss - fNeutron_TargWindow_Ion_Loss;

      fNeutron_Corrected_Mom_Col       = TMath::Sqrt( TMath::Power( fNeutron_Corrected_Energy_Col , 2 ) - TMath::Power( fNeutron_Mass , 2 ) );
      fNeutron_Delta_Mom_Col           = ( fNeutron_Mom_Col - fNeutron_Corrected_Mom_Col );
      fNeutron_Corrected_MomX_Col      = ( fNeutron_MomX_Col - fNeutron_Delta_Mom_Col * TMath::Sin((fPi/180.0) * fNeutron_Theta_Col ) *  
					     TMath::Cos((fPi/180.0) * fNeutron_Phi_Col ) );
      fNeutron_Corrected_MomY_Col      = ( fNeutron_MomY_Col - fNeutron_Delta_Mom_Col * TMath::Sin((fPi/180.0) * fNeutron_Theta_Col ) *  
					   TMath::Sin((fPi/180.0) * fNeutron_Phi_Col ) );
      fNeutron_Corrected_MomZ_Col      = fNeutron_MomZ_Col - fNeutron_Delta_Mom_Col * TMath::Cos((fPi/180.0) * fNeutron_Theta_Col );
      fNeutron_Corrected_Theta_Col     = (180/fPi)* TMath::ATan( TMath::Sqrt( TMath::Power(fNeutron_Corrected_MomX_Col,2) + 
									      TMath::Power(fNeutron_Corrected_MomY_Col,2) ) / fNeutron_Corrected_MomZ_Col );

      fNeutron_Corrected_Theta_Col     = (180/fPi)* TMath::ATan( TMath::Sqrt( TMath::Power(fNeutron_Corrected_MomX_Col,2) + 
									      TMath::Power(fNeutron_Corrected_MomY_Col,2) ) / fNeutron_Corrected_MomZ_Col );
      
      fNeutron_Corrected_Phi_Col   = (180/fPi)*TMath::ATan2(fNeutron_Corrected_MomY_Col,fNeutron_Corrected_MomX_Col);
      
      if ( ( fNeutron_Corrected_Theta_Col < 0 ) && ( fNeutron_Corrected_Phi_Col <= 0 ) ) 
	{ fNeutron_Corrected_Theta_Col = 180.0 + fNeutron_Corrected_Theta_Col;  fNeutron_Corrected_Phi_Col = 360 + fNeutron_Corrected_Phi_Col;} 
      if ( ( fNeutron_Corrected_Theta_Col > 0 ) && ( fNeutron_Corrected_Phi_Col < 0  ) ) 
	{ fNeutron_Corrected_Theta_Col = fNeutron_Corrected_Theta_Col;          fNeutron_Corrected_Phi_Col = 360 + fNeutron_Corrected_Phi_Col;}
      if ( ( fNeutron_Corrected_Theta_Col < 0 ) && ( fNeutron_Corrected_Phi_Col > 0 ) )  
	{ fNeutron_Corrected_Theta_Col = 180.0 + fNeutron_Corrected_Theta_Col;  fNeutron_Corrected_Phi_Col = fNeutron_Corrected_Phi_Col;}

    }

    if ( fNeutron_Corrected_Energy_Col < 0 )  { 
      fNeutron_Corrected_Energy_Col                = 1e-6;
      fNeutron_Corrected_Mom_Col                   = 1e-6;
      fNeutron_Corrected_MomZ_Col                  = 1e-6;
      fNeutron_Corrected_MomX_Col                  = 1e-6;
      fNeutron_Corrected_MomY_Col                  = 1e-6;
    }

    fNeutron_Corrected_Energy_Col_GeV             = fNeutron_Corrected_Energy_Col / 1000.0;
    fNeutron_Corrected_Mom_Col_GeV                = fNeutron_Corrected_Mom_Col / 1000.0;
    fNeutron_Delta_Mom_Col_GeV                    = fNeutron_Delta_Mom_Col / 1000.0;
    fNeutron_Corrected_MomX_Col_GeV               = fNeutron_Corrected_MomX_Col / 1000.0;
    fNeutron_Corrected_MomY_Col_GeV               = fNeutron_Corrected_MomY_Col / 1000.0;
    fNeutron_Corrected_MomZ_Col_GeV               = fNeutron_Corrected_MomZ_Col / 1000.0;

    // fNeutron_Energy_Col_GeV                       = fNeutron_Energy_Col / 1000.0;
    // fNeutron_Mom_Col_GeV                          = fNeutron_Mom_Col / 1000.0;
    // fNeutron_MomX_Col_GeV                         = fNeutron_MomX_Col / 1000.0;
    // fNeutron_MomY_Col_GeV                         = fNeutron_MomY_Col / 1000.0;
    // fNeutron_MomZ_Col_GeV                         = fNeutron_MomZ_Col / 1000.0;

    fRecoilProton_Targ_Thickness                  = fNeutron_Targ_Thickness;
    fRecoilProton_Targ_Thickness_RadLen           = fNeutron_Targ_Thickness_RadLen;
    fRecoilProton_Targ_BT                         = fNeutron_Targ_BT;
    fRecoilProton_Targ_Bremss_Loss                = fNeutron_Targ_Bremss_Loss;
    fRecoilProton_Targ_Ion_Loss                   = fNeutron_Targ_Ion_Loss;
    fRecoilProton_Air_Thickness                   = fNeutron_Air_Thickness;
    fRecoilProton_Air_Thickness_RadLen            = fNeutron_Air_Thickness_RadLen;
    fRecoilProton_Air_BT                          = fNeutron_Air_BT;
    fRecoilProton_Air_Bremss_Loss                 = fNeutron_Air_Bremss_Loss;
    fRecoilProton_Air_Ion_Loss                    = fNeutron_Air_Ion_Loss;

    fRecoilProton_Corrected_Energy_Col            = fNeutron_Corrected_Energy_Col;
    fRecoilProton_Corrected_Mom_Col               = fNeutron_Corrected_Mom_Col;
    fRecoilProton_Corrected_MomX_Col              = fNeutron_Corrected_MomX_Col;
    fRecoilProton_Corrected_MomY_Col              = fNeutron_Corrected_MomY_Col;
    fRecoilProton_Corrected_MomZ_Col              = fNeutron_Corrected_MomZ_Col;
    fRecoilProton_Corrected_Theta_Col             = fNeutron_Corrected_Theta_Col;
    fRecoilProton_Corrected_Phi_Col               = fNeutron_Corrected_Phi_Col;
    fRecoilProton_Delta_Mom_Col                   = fNeutron_Delta_Mom_Col;

    fRecoilProton_Theta_Col                       = fNeutron_Theta_Col;
    fRecoilProton_Phi_Col                         = fNeutron_Phi_Col;
    fRecoilProton_Energy_Col                      = fNeutron_Energy_Col;
    fRecoilProton_Mom_Col                         = fNeutron_Mom_Col;
    fRecoilProton_MomX_Col                        = fNeutron_MomX_Col;
    fRecoilProton_MomY_Col                        = fNeutron_MomY_Col;
    fRecoilProton_MomZ_Col                        = fNeutron_MomZ_Col;

    fRecoilProton_Energy_Col_GeV                  = fRecoilProton_Energy_Col / 1000.0;
    fRecoilProton_Mom_Col_GeV                     = fRecoilProton_Mom_Col / 1000.0;
    fRecoilProton_MomX_Col_GeV                    = fRecoilProton_MomX_Col / 1000.0;
    fRecoilProton_MomY_Col_GeV                    = fRecoilProton_MomY_Col / 1000.0;
    fRecoilProton_MomZ_Col_GeV                    = fRecoilProton_MomZ_Col / 1000.0;

    fRecoilProton_Corrected_Energy_Col_GeV        = fRecoilProton_Corrected_Energy_Col / 1000.0;
    fRecoilProton_Corrected_Mom_Col_GeV           = fRecoilProton_Corrected_Mom_Col / 1000.0;
    fRecoilProton_Corrected_MomX_Col_GeV          = fRecoilProton_Corrected_MomX_Col / 1000.0;
    fRecoilProton_Corrected_MomY_Col_GeV          = fRecoilProton_Corrected_MomY_Col / 1000.0;
    fRecoilProton_Corrected_MomZ_Col_GeV          = fRecoilProton_Corrected_MomZ_Col / 1000.0;
    fRecoilProton_Delta_Mom_Col_GeV               = fRecoilProton_Delta_Mom_Col / 1000.0;
    // --------------------------------- Energy Corrections for Recoil Proton ----------------------------------------------


    // ---------------------------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------------------------
    // Calculate w and -t
    // ---------------------------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------------------------
    fWSq = fWSq_GeV * 1000000.0;

    fW_GeV    = TMath::Sqrt( TMath::Power(fProton_Mass_GeV,2) - fQsq_GeV + 
			     2.0 * ( fPhoton_Energy_Col_GeV * fProton_Energy_Col_GeV - 
				     fPhoton_MomX_Col_GeV   * fProton_MomX_Col_GeV - 
				     fPhoton_MomY_Col_GeV   * fProton_MomY_Col_GeV - 
				     fPhoton_MomZ_Col_GeV   * fProton_MomZ_Col_GeV ) );        
    fW = fW_GeV * 1000.0;
    
    // --------------------------------------------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------------------------------------------

    fBeta_CM_RF           = 1.0*(fPhoton_Mom_Col / ( fPhoton_Energy_Col + fProton_Mass));
    fGamma_CM_RF          = ( fPhoton_Energy_Col + fProton_Mass) / fW;    
    fPion_Energy_CM       = ( TMath::Power( fW , 2) + TMath::Power(fPion_Mass , 2) - 
			      TMath::Power(fNeutron_Mass , 2) ) / ( 2.0 * fW);    
    fPion_Mom_CM          = TMath::Sqrt( TMath::Power(fPion_Energy_CM , 2) - 
					 TMath::Power(fPion_Mass , 2));
    fPion_Energy_CM_GeV   = fPion_Energy_CM / 1000.0;
    fPion_Mom_CM_GeV      = fPion_Mom_CM / 1000.0;

    // this equation is valid for parallel kinematics only!
    fT_Para = ( TMath::Power((fPhoton_Mom_Col    - fPion_Mom_Col),2) -
		TMath::Power((fPhoton_Energy_Col - fPion_Energy_Col),2));
    fT_Para_GeV = fT_Para/1000000.0;
    
    // ----------------------------------------------------
    // general equation for t for non-parallel kinematics
    // ----------------------------------------------------
    fT = ( TMath::Power( ( fPhoton_MomX_Col   - fPion_MomX_Col   ) , 2 ) + 
	   TMath::Power( ( fPhoton_MomY_Col   - fPion_MomY_Col   ) , 2 ) + 
	   TMath::Power( ( fPhoton_MomZ_Col   - fPion_MomZ_Col   ) , 2 ) - 
	   TMath::Power( ( fPhoton_Energy_Col - fPion_Energy_Col ) , 2 ));
    
    fT_GeV = fT / 1000000.0;    
    
    fx = fQsq_GeV/( 2.0 * fProton_Mass_GeV * fPhoton_Energy_Col_GeV );
    fz = fPion_Energy_Col/fPhoton_Energy_Col;  
    fy = fQsq / ( fx * ( TMath::Power(( fProton_Energy_Col+fElectron_Energy_Col ),2) - 
			 TMath::Power(( fProton_Mom_Col - fElectron_Mom_Col ),2 ) ) - 
		  TMath::Power(fProton_Mass,2) ) ;

    // if ( fx > 1 || fy > 1 )
    //   continue;
    
    // -------------------------------------------------------------------------------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------------------------------

    fS_I_Col = ( fElectron_Mass * fElectron_Mass + fProton_Mass * fProton_Mass + 2 * 
		 ( fElectron_Energy_Col * fProton_Energy_Col - 
		   fElectron_MomX_Col * fProton_MomX_Col - fElectron_MomY_Col * fProton_MomY_Col - fElectron_MomZ_Col * fProton_MomZ_Col ) );

    fS_F_Col = ( fElectron_Mass * fElectron_Mass +  
		 fNeutron_Mass * fNeutron_Mass + 
		 fPion_Mass * fPion_Mass + 
		 2 * ( fScatElec_Energy_Col * fNeutron_Energy_Col - 
		       fScatElec_MomX_Col * fNeutron_MomX_Col - 
		       fScatElec_MomY_Col * fNeutron_MomY_Col -
		       fScatElec_MomZ_Col * fNeutron_MomZ_Col ) + 
		 2 * ( fScatElec_Energy_Col * fPion_Energy_Col - 
		       fScatElec_MomX_Col * fPion_MomX_Col - 
		       fScatElec_MomY_Col * fPion_MomY_Col -
		       fScatElec_MomZ_Col * fPion_MomZ_Col ) + 
		 2 * ( fNeutron_Energy_Col * fPion_Energy_Col - 
		       fNeutron_MomX_Col * fPion_MomX_Col - 
		       fNeutron_MomY_Col * fPion_MomY_Col -
		       fNeutron_MomZ_Col * fPion_MomZ_Col ) );
    
    fS_I_Col_GeV = fS_I_Col / 1000000.0;
    fS_F_Col_GeV = fS_F_Col / 1000000.0;

    fMandSConserve = std::abs( sqrt( fS_I_Col_GeV ) - sqrt( fS_F_Col_GeV ) );
    
    kSConserve = false;
    if( std::abs( fS_I_Col_GeV - fS_F_Col_GeV ) < fDiff ) {
      kSConserve = true;
    }
    
    fXMomConserve   = 0;
    fYMomConserve   = 0;
    fZMomConserve   = 0;
    fEnergyConserve = 0;

    if ( ( fElectron_Energy_Check + fTarget_Energy_Check - fScatElec_Energy_Check - fPion_Energy_Check - fRecoilProton_Energy_Check ) < fDiff ) { fEnergyConserve = 1; }
    if ( ( fElectron_MomX_Check   + fTarget_MomX_Check   - fScatElec_MomX_Check   - fPion_MomX_Check   - fRecoilProton_MomX_Check )   < fDiff ) { fXMomConserve   = 1; }
    if ( ( fElectron_MomY_Check   + fTarget_MomY_Check   - fScatElec_MomY_Check   - fPion_MomY_Check   - fRecoilProton_MomY_Check )   < fDiff ) { fYMomConserve   = 1; }
    if ( ( fElectron_MomZ_Check   + fTarget_MomZ_Check   - fScatElec_MomZ_Check   - fPion_MomZ_Check   - fRecoilProton_MomZ_Check )   < fDiff ) { fZMomConserve   = 1; }

    if ( ( fXMomConserve == 0 ) || ( fYMomConserve == 0 ) || ( fZMomConserve == 0 ) || ( fEnergyConserve == 0 ) ) {
      fNMomConserve ++;
      continue;
    }

    // --------------------------------------------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------
    // Transformation to target's rest frame (RF)
    // ------------------------------------------------------------------------------------
    // ----------------------------------------------------
    // Transformation to the Proton frame
    // ----------------------------------------------------
    fProton_MomX_RF                       = 0.0;
    fProton_MomY_RF                       = 0.0;
    fProton_MomZ_RF                       = 0.0;
    fProton_Mom_RF                        = 0.0;
    fProton_Energy_RF                     = fProton_Mass;
    fProton_MomX_RF_GeV                   = fProton_MomX_RF / 1000.0;
    fProton_MomY_RF_GeV                   = fProton_MomY_RF / 1000.0;
    fProton_MomZ_RF_GeV                   = fProton_MomZ_RF / 1000.0;
    fProton_Mom_RF_GeV                    = fProton_Mom_RF / 1000.0;
    fProton_Energy_RF_GeV                 = fProton_Energy_RF / 1000.0;


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                                                                                          //
    //                                                                                                                          //
    //                                                                                                                          //
    //            Transformation of e', pi- and recoil proton to target's rest frmae with energy loss                           //
    //                                                                                                                          //
    //                                                                                                                          //
    //                                                                                                                          //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    
    // ------------------------------------------------------------------------------------
    // Electron
    // ------------------------------------------------------------------------------------
    fElectron_Energy_RF       = fGamma_Col_RF * fElectron_Corrected_Energy_Col - 
      fGamma_Col_RF * fBetaX_Col_RF * fElectron_Corrected_MomX_Col - 
      fGamma_Col_RF * fBetaY_Col_RF * fElectron_Corrected_MomY_Col -
      fGamma_Col_RF * fBetaZ_Col_RF * fElectron_Corrected_MomZ_Col;

    fElectron_MomX_RF           = -fGamma_Col_RF * fBetaX_Col_RF * fElectron_Corrected_Energy_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaX_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fElectron_Corrected_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fElectron_Corrected_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fElectron_Corrected_MomZ_Col;
    
    fElectron_MomY_RF           = -fGamma_Col_RF * fBetaY_Col_RF * fElectron_Corrected_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fElectron_Corrected_MomX_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaY_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fElectron_Corrected_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fElectron_Corrected_MomZ_Col;
    
    fElectron_MomZ_RF           = -fGamma_Col_RF * fBetaZ_Col_RF * fElectron_Corrected_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fElectron_Corrected_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fElectron_Corrected_MomY_Col +
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaZ_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fElectron_Corrected_MomZ_Col; 
    
    fElectron_Mom_RF            = TMath::Sqrt( TMath::Power(fElectron_MomX_RF,2) + 
					       TMath::Power(fElectron_MomY_RF,2) + 
					       TMath::Power(fElectron_MomZ_RF,2));

    fElectron_Theta_RF        = (180/fPi) *  TMath::ATan( TMath::Sqrt( TMath::Power(fElectron_MomX_RF , 2) + 
								       TMath::Power(fElectron_MomY_RF , 2) ) / fElectron_MomZ_RF );
    fElectron_Phi_RF          = (180/fPi)*TMath::ATan2(fElectron_MomY_RF,fElectron_MomX_RF);

    if ( ( fElectron_Theta_RF < 0 ) && ( fElectron_Phi_RF <= 0 ) ) 
      { fElectron_Theta_RF = 180.0 + fElectron_Theta_RF; fElectron_Phi_RF = 360 + fElectron_Phi_RF;}
    if ( ( fElectron_Theta_RF > 0 ) && ( fElectron_Phi_RF < 0 )  ) 
      { fElectron_Theta_RF = fElectron_Theta_RF;         fElectron_Phi_RF = 360 + fElectron_Phi_RF;}
    if ( ( fElectron_Theta_RF < 0 ) && ( fElectron_Phi_RF > 0 )  ) 
      { fElectron_Theta_RF = 180.0 + fElectron_Theta_RF; fElectron_Phi_RF = fElectron_Phi_RF;}


    // ------------------------------------------------------------------------------------
    // Scattered Electron
    // ------------------------------------------------------------------------------------
    fScatElec_Energy_RF       = fGamma_Col_RF * fScatElec_Corrected_Energy_Col - 
      fGamma_Col_RF * fBetaX_Col_RF * fScatElec_Corrected_MomX_Col - 
      fGamma_Col_RF * fBetaY_Col_RF * fScatElec_Corrected_MomY_Col -
      fGamma_Col_RF * fBetaZ_Col_RF * fScatElec_Corrected_MomZ_Col;

    fScatElec_MomX_RF           = -fGamma_Col_RF * fBetaX_Col_RF * fScatElec_Corrected_Energy_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaX_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fScatElec_Corrected_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fScatElec_Corrected_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fScatElec_Corrected_MomZ_Col;
    
    fScatElec_MomY_RF           = -fGamma_Col_RF * fBetaY_Col_RF * fScatElec_Corrected_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fScatElec_Corrected_MomX_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaY_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fScatElec_Corrected_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fScatElec_Corrected_MomZ_Col;
    
    fScatElec_MomZ_RF           = -fGamma_Col_RF * fBetaZ_Col_RF * fScatElec_Corrected_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fScatElec_Corrected_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fScatElec_Corrected_MomY_Col +
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaZ_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fScatElec_Corrected_MomZ_Col; 
    
    fScatElec_Mom_RF            = TMath::Sqrt( TMath::Power(fScatElec_MomX_RF,2) + 
					       TMath::Power(fScatElec_MomY_RF,2) + 
					       TMath::Power(fScatElec_MomZ_RF,2));

    fScatElec_Theta_RF        = (180/fPi) *  TMath::ATan( TMath::Sqrt( TMath::Power(fScatElec_MomX_RF , 2) + 
								       TMath::Power(fScatElec_MomY_RF , 2) ) / fScatElec_MomZ_RF );
    fScatElec_Phi_RF          = (180/fPi)*TMath::ATan2(fScatElec_MomY_RF,fScatElec_MomX_RF);

    if ( ( fScatElec_Theta_RF < 0 ) && ( fScatElec_Phi_RF <= 0 ) ) 
      { fScatElec_Theta_RF = 180.0 + fScatElec_Theta_RF; fScatElec_Phi_RF = 360 + fScatElec_Phi_RF;}
    if ( ( fScatElec_Theta_RF > 0 ) && ( fScatElec_Phi_RF < 0 )  ) 
      { fScatElec_Theta_RF = fScatElec_Theta_RF;         fScatElec_Phi_RF = 360 + fScatElec_Phi_RF;}
    if ( ( fScatElec_Theta_RF < 0 ) && ( fScatElec_Phi_RF > 0 )  ) 
      { fScatElec_Theta_RF = 180.0 + fScatElec_Theta_RF; fScatElec_Phi_RF = fScatElec_Phi_RF;}



    // ------------------------------------------------------------------------------------
    // Photon
    // ------------------------------------------------------------------------------------
    fPhoton_Energy_RF         = fGamma_Col_RF * fPhoton_Energy_Col - 
      fGamma_Col_RF * fBetaX_Col_RF * fPhoton_MomX_Col - 
      fGamma_Col_RF * fBetaY_Col_RF * fPhoton_MomY_Col -
      fGamma_Col_RF * fBetaZ_Col_RF * fPhoton_MomZ_Col;

    fPhoton_MomX_RF           = -fGamma_Col_RF * fBetaX_Col_RF * fPhoton_Energy_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaX_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fPhoton_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPhoton_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPhoton_MomZ_Col;
    
    fPhoton_MomY_RF           = -fGamma_Col_RF * fBetaY_Col_RF * fPhoton_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPhoton_MomX_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaY_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fPhoton_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPhoton_MomZ_Col;
    
    fPhoton_MomZ_RF           = -fGamma_Col_RF * fBetaZ_Col_RF * fPhoton_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPhoton_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPhoton_MomY_Col +
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaZ_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fPhoton_MomZ_Col; 
    
    fPhoton_Mom_RF            = TMath::Sqrt( TMath::Power(fPhoton_MomX_RF,2) + 
					     TMath::Power(fPhoton_MomY_RF,2) + 
					     TMath::Power(fPhoton_MomZ_RF,2));

    fPhoton_Theta_RF        = (180/fPi) *  TMath::ATan( TMath::Sqrt( TMath::Power(fPhoton_MomX_RF , 2) + 
								     TMath::Power(fPhoton_MomY_RF , 2) ) / fPhoton_MomZ_RF );
    fPhoton_Phi_RF          = (180/fPi)*TMath::ATan2(fPhoton_MomY_RF,fPhoton_MomX_RF);

    if ( ( fPhoton_Theta_RF < 0 ) && ( fPhoton_Phi_RF <= 0 ) ) 
      { fPhoton_Theta_RF = 180.0 + fPhoton_Theta_RF; fPhoton_Phi_RF = 360 + fPhoton_Phi_RF;}
    if ( ( fPhoton_Theta_RF > 0 ) && ( fPhoton_Phi_RF < 0 )  ) 
      { fPhoton_Theta_RF = fPhoton_Theta_RF;         fPhoton_Phi_RF = 360 + fPhoton_Phi_RF;}
    if ( ( fPhoton_Theta_RF < 0 ) && ( fPhoton_Phi_RF > 0 )  ) 
      { fPhoton_Theta_RF = 180.0 + fPhoton_Theta_RF; fPhoton_Phi_RF = fPhoton_Phi_RF;}

    // ------------------------------------------------------------------------------------
    // Pion
    // ------------------------------------------------------------------------------------
    fPion_Energy_RF           = fGamma_Col_RF * fPion_Corrected_Energy_Col - 
      fGamma_Col_RF * fBetaX_Col_RF * fPion_Corrected_MomX_Col - 
      fGamma_Col_RF * fBetaY_Col_RF * fPion_Corrected_MomY_Col -
      fGamma_Col_RF * fBetaZ_Col_RF * fPion_Corrected_MomZ_Col;

    fPion_MomX_RF             = -fGamma_Col_RF * fBetaX_Col_RF * fPion_Corrected_Energy_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaX_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fPion_Corrected_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPion_Corrected_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPion_Corrected_MomZ_Col;
    
    fPion_MomY_RF             = -fGamma_Col_RF * fBetaY_Col_RF * fPion_Corrected_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPion_Corrected_MomX_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaY_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fPion_Corrected_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPion_Corrected_MomZ_Col;
    
    fPion_MomZ_RF             = -fGamma_Col_RF * fBetaZ_Col_RF * fPion_Corrected_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPion_Corrected_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPion_Corrected_MomY_Col +
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaZ_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fPion_Corrected_MomZ_Col; 
    
    fPion_Mom_RF              = TMath::Sqrt( TMath::Power(fPion_MomX_RF,2) + 
					     TMath::Power(fPion_MomY_RF,2) + 
					     TMath::Power(fPion_MomZ_RF,2));

    fPion_Theta_RF        = (180/fPi) *  TMath::ATan( TMath::Sqrt( TMath::Power(fPion_MomX_RF , 2) + 
								   TMath::Power(fPion_MomY_RF , 2) ) / fPion_MomZ_RF );
    fPion_Phi_RF          = (180/fPi)*TMath::ATan2(fPion_MomY_RF,fPion_MomX_RF);

    if ( ( fPion_Theta_RF < 0 ) && ( fPion_Phi_RF <= 0 ) ) 
      { fPion_Theta_RF = 180.0 + fPion_Theta_RF; fPion_Phi_RF = 360 + fPion_Phi_RF;}
    if ( ( fPion_Theta_RF > 0 ) && ( fPion_Phi_RF < 0 )  ) 
      { fPion_Theta_RF = fPion_Theta_RF;         fPion_Phi_RF = 360 + fPion_Phi_RF;}
    if ( ( fPion_Theta_RF < 0 ) && ( fPion_Phi_RF > 0 )  ) 
      { fPion_Theta_RF = 180.0 + fPion_Theta_RF; fPion_Phi_RF = fPion_Phi_RF;}

    // ------------------------------------------------------------------------------------
    // Neutron
    // ------------------------------------------------------------------------------------
    fNeutron_Energy_RF           = fGamma_Col_RF * fNeutron_Corrected_Energy_Col - 
      fGamma_Col_RF * fBetaX_Col_RF * fNeutron_Corrected_MomX_Col - 
      fGamma_Col_RF * fBetaY_Col_RF * fNeutron_Corrected_MomY_Col -
      fGamma_Col_RF * fBetaZ_Col_RF * fNeutron_Corrected_MomZ_Col;

    fNeutron_MomX_RF             = -fGamma_Col_RF * fBetaX_Col_RF * fNeutron_Corrected_Energy_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaX_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fNeutron_Corrected_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fNeutron_Corrected_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fNeutron_Corrected_MomZ_Col;
    
    fNeutron_MomY_RF             = -fGamma_Col_RF * fBetaY_Col_RF * fNeutron_Corrected_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fNeutron_Corrected_MomX_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaY_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fNeutron_Corrected_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fNeutron_Corrected_MomZ_Col;
    
    fNeutron_MomZ_RF             = -fGamma_Col_RF * fBetaZ_Col_RF * fNeutron_Corrected_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fNeutron_Corrected_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fNeutron_Corrected_MomY_Col +
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaZ_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fNeutron_Corrected_MomZ_Col; 
    
    fNeutron_Mom_RF              = TMath::Sqrt( TMath::Power(fNeutron_MomX_RF,2) + 
						TMath::Power(fNeutron_MomY_RF,2) + 
						TMath::Power(fNeutron_MomZ_RF,2));

    fNeutron_Theta_RF        = (180/fPi) *  TMath::ATan( TMath::Sqrt( TMath::Power(fNeutron_MomX_RF , 2) + 
								      TMath::Power(fNeutron_MomY_RF , 2) ) / fNeutron_MomZ_RF );
    fNeutron_Phi_RF          = (180/fPi)*TMath::ATan2(fNeutron_MomY_RF,fNeutron_MomX_RF);

    if ( ( fNeutron_Theta_RF < 0 ) && ( fNeutron_Phi_RF <= 0 ) ) 
      { fNeutron_Theta_RF = 180.0 + fNeutron_Theta_RF; fNeutron_Phi_RF = 360 + fNeutron_Phi_RF;}
    if ( ( fNeutron_Theta_RF > 0 ) && ( fNeutron_Phi_RF < 0 )  ) 
      { fNeutron_Theta_RF = fNeutron_Theta_RF;         fNeutron_Phi_RF = 360 + fNeutron_Phi_RF;}
    if ( ( fNeutron_Theta_RF < 0 ) && ( fNeutron_Phi_RF > 0 )  ) 
      { fNeutron_Theta_RF = 180.0 + fNeutron_Theta_RF; fNeutron_Phi_RF = fNeutron_Phi_RF;}


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                                                                                          //
    //                                                     Start                                                                //
    //                                                                                                                          //
    //            Transformation of e', pi- and recoil proton to target's rest frmae without energy loss                        //
    //                                                                                                                          //
    //                                                                                                                          //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    /*
    // ------------------------------------------------------------------------------------
    // Electron
    // ------------------------------------------------------------------------------------
    fElectron_Energy_RF       = fGamma_Col_RF * fElectron_Energy_Col - 
      fGamma_Col_RF * fBetaX_Col_RF * fElectron_MomX_Col - 
      fGamma_Col_RF * fBetaY_Col_RF * fElectron_MomY_Col -
      fGamma_Col_RF * fBetaZ_Col_RF * fElectron_MomZ_Col;

    fElectron_MomX_RF           = -fGamma_Col_RF * fBetaX_Col_RF * fElectron_Energy_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaX_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fElectron_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fElectron_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fElectron_MomZ_Col;
    
    fElectron_MomY_RF           = -fGamma_Col_RF * fBetaY_Col_RF * fElectron_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fElectron_MomX_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaY_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fElectron_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fElectron_MomZ_Col;
    
    fElectron_MomZ_RF           = -fGamma_Col_RF * fBetaZ_Col_RF * fElectron_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fElectron_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fElectron_MomY_Col +
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaZ_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fElectron_MomZ_Col; 
    
    fElectron_Mom_RF            = TMath::Sqrt( TMath::Power(fElectron_MomX_RF,2) + 
					       TMath::Power(fElectron_MomY_RF,2) + 
					       TMath::Power(fElectron_MomZ_RF,2));

    fElectron_Theta_RF        = (180/fPi) *  TMath::ATan( TMath::Sqrt( TMath::Power(fElectron_MomX_RF , 2) + 
								       TMath::Power(fElectron_MomY_RF , 2) ) / fElectron_MomZ_RF );
    fElectron_Phi_RF          = (180/fPi)*TMath::ATan2(fElectron_MomY_RF,fElectron_MomX_RF);

    if ( ( fElectron_Theta_RF < 0 ) && ( fElectron_Phi_RF <= 0 ) ) 
      { fElectron_Theta_RF = 180.0 + fElectron_Theta_RF; fElectron_Phi_RF = 360 + fElectron_Phi_RF;}
    if ( ( fElectron_Theta_RF > 0 ) && ( fElectron_Phi_RF < 0 )  ) 
      { fElectron_Theta_RF = fElectron_Theta_RF;         fElectron_Phi_RF = 360 + fElectron_Phi_RF;}
    if ( ( fElectron_Theta_RF < 0 ) && ( fElectron_Phi_RF > 0 )  ) 
      { fElectron_Theta_RF = 180.0 + fElectron_Theta_RF; fElectron_Phi_RF = fElectron_Phi_RF;}


    // ------------------------------------------------------------------------------------
    // Scattered Electron
    // ------------------------------------------------------------------------------------
    fScatElec_Energy_RF       = fGamma_Col_RF * fScatElec_Energy_Col - 
      fGamma_Col_RF * fBetaX_Col_RF * fScatElec_MomX_Col - 
      fGamma_Col_RF * fBetaY_Col_RF * fScatElec_MomY_Col -
      fGamma_Col_RF * fBetaZ_Col_RF * fScatElec_MomZ_Col;

    fScatElec_MomX_RF           = -fGamma_Col_RF * fBetaX_Col_RF * fScatElec_Energy_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaX_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fScatElec_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fScatElec_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fScatElec_MomZ_Col;
    
    fScatElec_MomY_RF           = -fGamma_Col_RF * fBetaY_Col_RF * fScatElec_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fScatElec_MomX_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaY_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fScatElec_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fScatElec_MomZ_Col;
    
    fScatElec_MomZ_RF           = -fGamma_Col_RF * fBetaZ_Col_RF * fScatElec_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fScatElec_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fScatElec_MomY_Col +
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaZ_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fScatElec_MomZ_Col; 
    
    fScatElec_Mom_RF            = TMath::Sqrt( TMath::Power(fScatElec_MomX_RF,2) + 
					       TMath::Power(fScatElec_MomY_RF,2) + 
					       TMath::Power(fScatElec_MomZ_RF,2));

    fScatElec_Theta_RF        = (180/fPi) *  TMath::ATan( TMath::Sqrt( TMath::Power(fScatElec_MomX_RF , 2) + 
								       TMath::Power(fScatElec_MomY_RF , 2) ) / fScatElec_MomZ_RF );
    fScatElec_Phi_RF          = (180/fPi)*TMath::ATan2(fScatElec_MomY_RF,fScatElec_MomX_RF);

    if ( ( fScatElec_Theta_RF < 0 ) && ( fScatElec_Phi_RF <= 0 ) ) 
      { fScatElec_Theta_RF = 180.0 + fScatElec_Theta_RF; fScatElec_Phi_RF = 360 + fScatElec_Phi_RF;}
    if ( ( fScatElec_Theta_RF > 0 ) && ( fScatElec_Phi_RF < 0 )  ) 
      { fScatElec_Theta_RF = fScatElec_Theta_RF;         fScatElec_Phi_RF = 360 + fScatElec_Phi_RF;}
    if ( ( fScatElec_Theta_RF < 0 ) && ( fScatElec_Phi_RF > 0 )  ) 
      { fScatElec_Theta_RF = 180.0 + fScatElec_Theta_RF; fScatElec_Phi_RF = fScatElec_Phi_RF;}



    // ------------------------------------------------------------------------------------
    // Photon
    // ------------------------------------------------------------------------------------
    fPhoton_Energy_RF         = fGamma_Col_RF * fPhoton_Energy_Col - 
      fGamma_Col_RF * fBetaX_Col_RF * fPhoton_MomX_Col - 
      fGamma_Col_RF * fBetaY_Col_RF * fPhoton_MomY_Col -
      fGamma_Col_RF * fBetaZ_Col_RF * fPhoton_MomZ_Col;

    fPhoton_MomX_RF           = -fGamma_Col_RF * fBetaX_Col_RF * fPhoton_Energy_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaX_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fPhoton_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPhoton_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPhoton_MomZ_Col;
    
    fPhoton_MomY_RF           = -fGamma_Col_RF * fBetaY_Col_RF * fPhoton_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPhoton_MomX_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaY_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fPhoton_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPhoton_MomZ_Col;
    
    fPhoton_MomZ_RF           = -fGamma_Col_RF * fBetaZ_Col_RF * fPhoton_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPhoton_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPhoton_MomY_Col +
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaZ_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fPhoton_MomZ_Col; 
    
    fPhoton_Mom_RF            = TMath::Sqrt( TMath::Power(fPhoton_MomX_RF,2) + 
					     TMath::Power(fPhoton_MomY_RF,2) + 
					     TMath::Power(fPhoton_MomZ_RF,2));

    fPhoton_Theta_RF        = (180/fPi) *  TMath::ATan( TMath::Sqrt( TMath::Power(fPhoton_MomX_RF , 2) + 
								     TMath::Power(fPhoton_MomY_RF , 2) ) / fPhoton_MomZ_RF );
    fPhoton_Phi_RF          = (180/fPi)*TMath::ATan2(fPhoton_MomY_RF,fPhoton_MomX_RF);

    if ( ( fPhoton_Theta_RF < 0 ) && ( fPhoton_Phi_RF <= 0 ) ) 
      { fPhoton_Theta_RF = 180.0 + fPhoton_Theta_RF; fPhoton_Phi_RF = 360 + fPhoton_Phi_RF;}
    if ( ( fPhoton_Theta_RF > 0 ) && ( fPhoton_Phi_RF < 0 )  ) 
      { fPhoton_Theta_RF = fPhoton_Theta_RF;         fPhoton_Phi_RF = 360 + fPhoton_Phi_RF;}
    if ( ( fPhoton_Theta_RF < 0 ) && ( fPhoton_Phi_RF > 0 )  ) 
      { fPhoton_Theta_RF = 180.0 + fPhoton_Theta_RF; fPhoton_Phi_RF = fPhoton_Phi_RF;}

    // ------------------------------------------------------------------------------------
    // Pion
    // ------------------------------------------------------------------------------------
    fPion_Energy_RF           = fGamma_Col_RF * fPion_Energy_Col - 
      fGamma_Col_RF * fBetaX_Col_RF * fPion_MomX_Col - 
      fGamma_Col_RF * fBetaY_Col_RF * fPion_MomY_Col -
      fGamma_Col_RF * fBetaZ_Col_RF * fPion_MomZ_Col;

    fPion_MomX_RF             = -fGamma_Col_RF * fBetaX_Col_RF * fPion_Energy_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaX_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fPion_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPion_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPion_MomZ_Col;
    
    fPion_MomY_RF             = -fGamma_Col_RF * fBetaY_Col_RF * fPion_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPion_MomX_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaY_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fPion_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPion_MomZ_Col;
    
    fPion_MomZ_RF             = -fGamma_Col_RF * fBetaZ_Col_RF * fPion_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPion_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fPion_MomY_Col +
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaZ_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fPion_MomZ_Col; 
    
    fPion_Mom_RF              = TMath::Sqrt( TMath::Power(fPion_MomX_RF,2) + 
					     TMath::Power(fPion_MomY_RF,2) + 
					     TMath::Power(fPion_MomZ_RF,2));

    fPion_Theta_RF        = (180/fPi) *  TMath::ATan( TMath::Sqrt( TMath::Power(fPion_MomX_RF , 2) + 
								   TMath::Power(fPion_MomY_RF , 2) ) / fPion_MomZ_RF );
    fPion_Phi_RF          = (180/fPi)*TMath::ATan2(fPion_MomY_RF,fPion_MomX_RF);

    if ( ( fPion_Theta_RF < 0 ) && ( fPion_Phi_RF <= 0 ) ) 
      { fPion_Theta_RF = 180.0 + fPion_Theta_RF; fPion_Phi_RF = 360 + fPion_Phi_RF;}
    if ( ( fPion_Theta_RF > 0 ) && ( fPion_Phi_RF < 0 )  ) 
      { fPion_Theta_RF = fPion_Theta_RF;         fPion_Phi_RF = 360 + fPion_Phi_RF;}
    if ( ( fPion_Theta_RF < 0 ) && ( fPion_Phi_RF > 0 )  ) 
      { fPion_Theta_RF = 180.0 + fPion_Theta_RF; fPion_Phi_RF = fPion_Phi_RF;}

    // ------------------------------------------------------------------------------------
    // Neutron
    // ------------------------------------------------------------------------------------
    fNeutron_Energy_RF           = fGamma_Col_RF * fNeutron_Energy_Col - 
      fGamma_Col_RF * fBetaX_Col_RF * fNeutron_MomX_Col - 
      fGamma_Col_RF * fBetaY_Col_RF * fNeutron_MomY_Col -
      fGamma_Col_RF * fBetaZ_Col_RF * fNeutron_MomZ_Col;

    fNeutron_MomX_RF             = -fGamma_Col_RF * fBetaX_Col_RF * fNeutron_Energy_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaX_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fNeutron_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fNeutron_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fNeutron_MomZ_Col;
    
    fNeutron_MomY_RF             = -fGamma_Col_RF * fBetaY_Col_RF * fNeutron_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fNeutron_MomX_Col + 
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaY_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fNeutron_MomY_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fNeutron_MomZ_Col;
    
    fNeutron_MomZ_RF             = -fGamma_Col_RF * fBetaZ_Col_RF * fNeutron_Energy_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fNeutron_MomX_Col + 
      ( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fNeutron_MomY_Col +
      ( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaZ_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fNeutron_MomZ_Col; 
    
    fNeutron_Mom_RF              = TMath::Sqrt( TMath::Power(fNeutron_MomX_RF,2) + 
						TMath::Power(fNeutron_MomY_RF,2) + 
						TMath::Power(fNeutron_MomZ_RF,2));

    fNeutron_Theta_RF        = (180/fPi) *  TMath::ATan( TMath::Sqrt( TMath::Power(fNeutron_MomX_RF , 2) + 
								      TMath::Power(fNeutron_MomY_RF , 2) ) / fNeutron_MomZ_RF );
    fNeutron_Phi_RF          = (180/fPi)*TMath::ATan2(fNeutron_MomY_RF,fNeutron_MomX_RF);

    if ( ( fNeutron_Theta_RF < 0 ) && ( fNeutron_Phi_RF <= 0 ) ) 
      { fNeutron_Theta_RF = 180.0 + fNeutron_Theta_RF; fNeutron_Phi_RF = 360 + fNeutron_Phi_RF;}
    if ( ( fNeutron_Theta_RF > 0 ) && ( fNeutron_Phi_RF < 0 )  ) 
      { fNeutron_Theta_RF = fNeutron_Theta_RF;         fNeutron_Phi_RF = 360 + fNeutron_Phi_RF;}
    if ( ( fNeutron_Theta_RF < 0 ) && ( fNeutron_Phi_RF > 0 )  ) 
      { fNeutron_Theta_RF = 180.0 + fNeutron_Theta_RF; fNeutron_Phi_RF = fNeutron_Phi_RF;}

    */

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                                                                                          //
    //                                                     End                                                                  //
    //                                                                                                                          //
    //            Transformation of e', pi- and recoil proton to target's rest frmae without energy loss                        //
    //                                                                                                                          //
    //                                                                                                                          //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    fElectron_Energy_RF_GeV              = fElectron_Energy_RF / 1000.0;
    fElectron_Mom_RF_GeV                 = fElectron_Mom_RF / 1000.0;
    fElectron_MomX_RF_GeV                = fElectron_MomX_RF / 1000.0;
    fElectron_MomY_RF_GeV                = fElectron_MomY_RF / 1000.0;
    fElectron_MomZ_RF_GeV                = fElectron_MomZ_RF / 1000.0;

    fScatElec_Energy_RF_GeV              = fScatElec_Energy_RF / 1000.0;
    fScatElec_Mom_RF_GeV                 = fScatElec_Mom_RF / 1000.0;
    fScatElec_MomX_RF_GeV                = fScatElec_MomX_RF / 1000.0;
    fScatElec_MomY_RF_GeV                = fScatElec_MomY_RF / 1000.0;
    fScatElec_MomZ_RF_GeV                = fScatElec_MomZ_RF / 1000.0;

    fPhoton_Energy_RF_GeV                = fPhoton_Energy_RF / 1000.0;
    fPhoton_Mom_RF_GeV                   = fPhoton_Mom_RF / 1000.0;
    fPhoton_MomX_RF_GeV                  = fPhoton_MomX_RF / 1000.0;
    fPhoton_MomY_RF_GeV                  = fPhoton_MomY_RF / 1000.0;
    fPhoton_MomZ_RF_GeV                  = fPhoton_MomZ_RF / 1000.0;

    fPion_Energy_RF_GeV                  = fPion_Energy_RF / 1000.0;
    fPion_Mom_RF_GeV                     = fPion_Mom_RF / 1000.0;
    fPion_MomX_RF_GeV                    = fPion_MomX_RF / 1000.0;
    fPion_MomY_RF_GeV                    = fPion_MomY_RF / 1000.0;
    fPion_MomZ_RF_GeV                    = fPion_MomZ_RF / 1000.0;

    fNeutron_Energy_RF_GeV               = fNeutron_Energy_RF / 1000.0;
    fNeutron_Mom_RF_GeV                  = fNeutron_Mom_RF / 1000.0;
    fNeutron_MomX_RF_GeV                 = fNeutron_MomX_RF / 1000.0;
    fNeutron_MomY_RF_GeV                 = fNeutron_MomY_RF / 1000.0;
    fNeutron_MomZ_RF_GeV                 = fNeutron_MomZ_RF / 1000.0;

    fRecoilProton_Energy_RF              = fNeutron_Energy_RF;
    fRecoilProton_Mom_RF                 = fNeutron_Mom_RF;
    fRecoilProton_MomX_RF                = fNeutron_MomX_RF;
    fRecoilProton_MomY_RF                = fNeutron_MomY_RF;
    fRecoilProton_MomZ_RF                = fNeutron_MomZ_RF;
    fRecoilProton_Energy_RF_GeV          = fNeutron_Energy_RF / 1000.0;
    fRecoilProton_Mom_RF_GeV             = fNeutron_Mom_RF / 1000.0;
    fRecoilProton_MomX_RF_GeV            = fNeutron_MomX_RF / 1000.0;
    fRecoilProton_MomY_RF_GeV            = fNeutron_MomY_RF / 1000.0;
    fRecoilProton_MomZ_RF_GeV            = fNeutron_MomZ_RF / 1000.0;
    fRecoilProton_Theta_RF               = fNeutron_Theta_RF;
    fRecoilProton_Theta_RF               = fNeutron_Phi_RF;
    
    fXMomConserve_RF = 0;
    fYMomConserve_RF = 0;
    fZMomConserve_RF = 0;    
    if ( ( fElectron_MomX_RF - fScatElec_MomX_RF - fPion_MomX_RF - fRecoilProton_MomX_RF ) < 0.1 ) { fXMomConserve_RF = 1; }
    if ( ( fElectron_MomY_RF - fScatElec_MomY_RF - fPion_MomY_RF - fRecoilProton_MomY_RF ) < 0.1 ) { fYMomConserve_RF = 1; }
    if ( ( fElectron_MomZ_RF - fScatElec_MomZ_RF - fPion_MomZ_RF - fRecoilProton_MomZ_RF ) < 0.1 ) { fZMomConserve_RF = 1; }

    // ------------------------------------------------------------------------------------
    // Transformation of Target Polarization in Collider (lab) Frame
    // ------------------------------------------------------------------------------------

    TLorentzVector lTarget_Pol_Col( -0.85, 0, 0, 0 );
    fTarget_Pol0_Col = lTarget_Pol_Col.T();
    fTarget_PolX_Col = lTarget_Pol_Col.X();
    fTarget_PolY_Col = lTarget_Pol_Col.Y();
    fTarget_PolZ_Col = lTarget_Pol_Col.Z();

    // ------------------------------------------------------------------------------------
    // Transformation of Target Polarization in Target's Rest Frame
    // ------------------------------------------------------------------------------------

    fTarget_Pol0_RF = ( fGamma_Col_RF                 * fTarget_Pol0_Col - 
			fGamma_Col_RF * fBetaX_Col_RF * fTarget_PolX_Col - 
			fGamma_Col_RF * fBetaY_Col_RF * fTarget_PolY_Col -
			fGamma_Col_RF * fBetaZ_Col_RF * fTarget_PolZ_Col );

    fTarget_PolX_RF = ( ( -fGamma_Col_RF * fBetaX_Col_RF )                                                                   * fTarget_Pol0_Col + 
			( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaX_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fTarget_PolX_Col + 
			( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fTarget_PolY_Col + 
			( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fTarget_PolZ_Col );
    
    fTarget_PolY_RF = ( ( -fGamma_Col_RF * fBetaY_Col_RF )                                                                   * fTarget_Pol0_Col + 
			( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaY_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fTarget_PolX_Col + 
			( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaY_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fTarget_PolY_Col + 
			( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fTarget_PolZ_Col );
    
    fTarget_PolZ_RF = ( ( -fGamma_Col_RF * fBetaZ_Col_RF )                                                                   * fTarget_Pol0_Col + 
			( fGamma_Col_RF - 1 )       * ( ( fBetaX_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fTarget_PolX_Col + 
			( fGamma_Col_RF - 1 )       * ( ( fBetaY_Col_RF * fBetaZ_Col_RF ) / TMath::Power(fBeta_Col_RF,2) )   * fTarget_PolY_Col +
			( 1 + ( fGamma_Col_RF - 1 ) * ( TMath::Power(fBetaZ_Col_RF,2)     / TMath::Power(fBeta_Col_RF,2) ) ) * fTarget_PolZ_Col ); 
    
    TLorentzVector lTarget_Pol_RF( fTarget_PolX_RF, fTarget_PolY_RF, fTarget_PolZ_RF, fTarget_Pol0_RF );
       
    // -------------------------------------------------------------------------------------------------------
    // Calculation of Phi  ( azimuthal angle of pion momentum w.r.t lepton plane in target's rest frame)
    // Calculation of PhiS ( azimuthal angle of target polarization w.r.t lepton plane in target's rest frame)
    // -------------------------------------------------------------------------------------------------------

    TVector3 v3Photon( fPhoton_MomX_RF_GeV , fPhoton_MomY_RF_GeV , fPhoton_MomZ_RF_GeV );
    TVector3 v3PhotonUnit = v3Photon.Unit();
    TVector3 v3Electron( fElectron_MomX_RF_GeV , fElectron_MomY_RF_GeV , fElectron_MomZ_RF_GeV );
    TVector3 v3Pion( fPion_MomX_RF_GeV , fPion_MomY_RF_GeV , fPion_MomZ_RF_GeV );
    // TVector3 v3S( 0 , -1 , 0 ); // This is the value for HERMES 
    TVector3 v3S( fTarget_PolX_RF , fTarget_PolY_RF , fTarget_PolZ_RF ); // This is the value for SoLID
    TVector3 v3QxL     = v3Photon.Cross(v3Electron);
    TVector3 v3QxP     = v3Photon.Cross(v3Pion);
    TVector3 v3QxS     = v3Photon.Cross(v3S);
    TVector3 v3LxP     = v3Electron.Cross(v3Pion);
    TVector3 v3LxS     = v3Electron.Cross(v3S);
    TVector3 v3PxL     = v3Pion.Cross(v3Electron);
    TVector3 v3QUnitxL = v3PhotonUnit.Cross(v3Electron);
    TVector3 v3QUnitxP = v3PhotonUnit.Cross(v3Pion);
    TVector3 v3QUnitxS = v3PhotonUnit.Cross(v3S);

    fCos_Phi_Pion_LeptonPlane_RF = ( ( v3QUnitxL.Dot( v3QUnitxP ) ) / ( v3QUnitxL.Mag() * v3QUnitxP.Mag() ) ); // hep-ph/0410050v2
    fSin_Phi_Pion_LeptonPlane_RF = ( ( v3LxP.Dot( v3PhotonUnit  ) ) / ( v3QUnitxL.Mag() * v3QUnitxP.Mag() ) ); // hep-ph/0410050v2
    if ( fSin_Phi_Pion_LeptonPlane_RF >= 0 )
      fPhi_Pion_LeptonPlane_RF    = ( 180.0 / fPi ) * TMath::ACos( ( v3QUnitxL.Dot( v3QUnitxP ) ) / ( v3QUnitxL.Mag() * v3QUnitxP.Mag() ) );
    if ( fSin_Phi_Pion_LeptonPlane_RF < 0 )
      fPhi_Pion_LeptonPlane_RF    = 360.0 - std::abs( ( 180.0 / fPi ) * TMath::ACos( ( v3QUnitxL.Dot( v3QUnitxP ) ) / ( v3QUnitxL.Mag() * v3QUnitxP.Mag() ) ) );

    fCos_Phi_TargPol_LeptonPlane_RF = ( ( v3QUnitxL.Dot( v3QUnitxS ) ) / ( v3QUnitxL.Mag() * v3QUnitxS.Mag() ) ); // hep-ph/0410050v2
    fSin_Phi_TargPol_LeptonPlane_RF = ( ( v3LxS.Dot( v3PhotonUnit  ) ) / ( v3QUnitxL.Mag() * v3QUnitxS.Mag() ) ); // hep-ph/0410050v2
    if ( fSin_Phi_TargPol_LeptonPlane_RF >= 0 )
      fPhi_TargPol_LeptonPlane_RF = ( 180.0 / fPi ) * TMath::ACos( ( v3QUnitxL.Dot( v3QUnitxS ) ) / ( v3QUnitxL.Mag() * v3QUnitxS.Mag() ) );
    if ( fSin_Phi_TargPol_LeptonPlane_RF < 0 )
      fPhi_TargPol_LeptonPlane_RF = 360.0 - std::abs( ( 180.0 / fPi ) * TMath::ACos( ( v3QUnitxL.Dot( v3QUnitxS ) ) / ( v3QUnitxL.Mag() * v3QUnitxS.Mag() ) ) );

    fTheta_Pion_Photon_RF       = ( 180.0 / fPi ) * TMath::ACos( ( v3Photon.Dot( v3Pion     ) ) / ( v3Photon.Mag()  * v3Pion.Mag()    ) );
    if ( fTheta_Pion_Photon_RF < 0 ) { fTheta_Pion_Photon_RF = 180.0 + fTheta_Pion_Photon_RF; }

    double fPhi   = fPhi_Pion_LeptonPlane_RF;
    double fPhiS  = fPhi_TargPol_LeptonPlane_RF;
    //double fTheta = fTheta_Pion_Photon_RF;
    //Theta seems to be:
    double fTheta = fPhoton_Theta_Col;
    //It is defined as the angle between q and l in the thesis

    if ( fT_GeV <= 1.5 ){    
      // -----------------------------------------------------------------------------------------
      // Zafar Ahmed Scattering Cross Section for unpolarized beam and unpolarized target, dsig_UU
      // -----------------------------------------------------------------------------------------
   
      fZASig_L  = MySigmaL(  fQsq_GeV , fT_GeV , fW_GeV );
      fZASig_T  = MySigmaT(  fQsq_GeV , fT_GeV , fW_GeV );
      fZASig_LT = MySigmaLT( fQsq_GeV , fT_GeV , fW_GeV );
      fZASig_TT = MySigmaTT( fQsq_GeV , fT_GeV , fW_GeV );

      if ( fZASig_L < 1e-12 &&  fZASig_T < 1e-12 ) {
	fZASig_LT = 0.0;
	fZASig_TT = 0.0;
      }

      //---------------------------------------------------------------------------------------------------------
      //---------------------------------------------------------------------------------------------------------
      // Due to our parametrization we are getting -ve value of total cross section. It happens when fZASig_TT
      // or fZASig_LT is -ve and greater than other two terms. We used the VR model to measure the ratio of
      // interference terms to fZASig_T from Qsq 0 to 5 GeV^2. In each bin of Qsq -t is scanned from 0.16 to 
      // 1.2 and we recorded the maxim ratio of fZASig_TT/fZASig_T and fZASig_LT/fZASig_T. Then if interference 
      // term icreases fZASig_T then we multiply the interference term by this ratio. After Qsq = 5 GeV^2 
      // the two ratios are almost same.

      if ( ( fZASig_TT < 0 ) &&  ( std::abs(fZASig_TT) > std::abs(fZASig_T) ) )
	fZASig_TT = correctionToSigTT( fZASig_TT, fZASig_T, fQsq_GeV );

      if ( ( fZASig_LT < 0 ) &&  ( std::abs(fZASig_LT) > std::abs(fZASig_T) ) )
	fZASig_LT = correctionToSigLT( fZASig_LT, fZASig_T, fQsq_GeV );    

    }
    
    if( fT_GeV > 1.5 ){
      //---------------------------------------------------------------------------------------------------------
      //---------------------------------------------------------------------------------------------------------
      // Our parametrization is valid upto -t = 1.2. It fails for -t > 2 so we need to make all four cross
      // sections very small.

      fZASig_L  = 1e-8;
      fZASig_T  = 1e-8;
      fZASig_LT = 1e-9;
      fZASig_TT = 1e-9;
    }
      
    
    //---------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------

    // If we have fermi momentum then epsilon should be in rest frame 
    // epsilon=1./(1.+ 2.*(pgam_restg**2)/q2g * *(tand(thscat_rest/2.))**2)

    fEpsilon = 1.0 / ( 1.0 + 2.0 * ( TMath::Power(fPhoton_Mom_RF_GeV,2)/fQsq_GeV ) *
		       TMath::Power( TMath::Tan( (fPi/180.0)*(fScatElec_Theta_RF)/2) ,2 ) );

    if ( fEpsilon > 0.99 )
      fEpsilon = 0.99;
    
    fZASigma_UUPara = fZASig_T + fEpsilon * fZASig_L;        

    fZASigma_UU = ( fZASig_T + fEpsilon * fZASig_L + 
		    fEpsilon * TMath::Cos( ( fPi / 180.0 ) * 2.0 * fPhi ) * fZASig_TT + 
		    TMath::Sqrt( 0.5 * fEpsilon * ( 1.0 + fEpsilon ) ) * 2.0 * TMath::Cos( ( fPi / 180.0 ) * fPhi ) * fZASig_LT );

    fRR = fZASig_T / fZASig_L;


    //---------------------------------------- Rory's Sigma -----------------------------    

    //  fZASig_L  = MySigmaL(  fQsq_GeV , fT_GeV , fW_GeV ); // Ahme's code

    double myTprime = (fT_GeV - fT_Para_GeV);

    // fAsymF1 = FunctionAsymF1( fQsq , myTprime ) // Rory's code

    // fAsymPhiMinusPhi_S   = asy(     fQsq , myTprime );
    // fAsymPhi_S           = asysfi(  fQsq , myTprime );
    // fAsym2PhiMinusPhi_S  = asy2fi(  fQsq , myTprime );
    // fAsymPhiPlusPhi_S    = asyfpfs( fQsq , myTprime );
    // fAsym3PhiMinusPhi_S  = asy3f(   fQsq , myTprime );

    // double fRorySigma0 = fAsymPhiMinusPhi_S  * fZASigma_UU;
    // double fRorySigma1 = fAsymPhi_S          * fZASigma_UU;
    // double fRorySigma2 = fAsym2PhiMinusPhi_S * fZASigma_UU;
    // double fRorySigma3 = fAsymPhiPlusPhi_S   * fZASigma_UU;
    // double fRorySigma4 = fAsym3PhiMinusPhi_S * fZASigma_UU;
    // double fRorySigma5 = 0;

    fRorySigma_UT = 0;
    // fRorySigma_UT += TMath::Sin(fPhi-fPhiS)   * fRorySigma0;
    // fRorySigma_UT += TMath::Sin(fPhiS)        * fRorySigma1;
    // fRorySigma_UT += TMath::Sin(2*fPhi-fPhiS) * fRorySigma2;
    // fRorySigma_UT += TMath::Sin(fPhi+fPhiS)   * fRorySigma3;
    // fRorySigma_UT += TMath::Sin(3*fPhi-fPhiS) * fRorySigma4;
    // fRorySigma_UT += TMath::Sin(2*fPhi+fPhiS) * fRorySigma5;

    //P_T is the component of the target polarization perpendicular to the beam direction.
    //In rest frame? Either fTarget_PolX_Col or fTargetPolX_RF

    fRorySigma_UT *= ( -fTarget_PolX_RF / TMath::Sqrt( 1 - TMath::Sin( fTheta ) * TMath::Sin( fTheta ) * 
						           TMath::Sin( fPhiS )  * TMath::Sin( fPhiS ) ) );
        
    //---------------------------------------- Rory's Sigma -----------------------------    

    // ----------------------------------------------------
    // Virtual Photon flux factor in units of 1/(GeV*Sr)
    // ----------------------------------------------------
    // fFlux_Factor_Col = (fAlpha/(2.0*TMath::Power(fPi,2))) * (fScatElec_Energy_Col_GeV / fElectron_Energy_Col_GeV) * 
    //   ( TMath::Power(fW_GeV,2) - TMath::Power(fProton_Mass_GeV,2) ) / (2.0*fProton_Mass_GeV*fQsq_GeV*(1.0 - fEpsilon));

    fFlux_Factor_Col = (fAlpha/(2.0*TMath::Power(fPi,2))) * (fScatElec_Corrected_Energy_Col_GeV / fElectron_Corrected_Energy_Col_GeV) * 
      ( TMath::Power(fW_GeV,2) - TMath::Power(fProton_Mass_GeV,2) ) / (2.0*fProton_Mass_GeV*fQsq_GeV*(1.0 - fEpsilon));
        
    fFlux_Factor_RF = ( fAlpha / ( 2.0 * TMath::Power( fPi , 2 ) ) ) * ( fScatElec_Energy_RF_GeV / fElectron_Energy_RF_GeV ) *
      ( TMath::Power( fW_GeV , 2 ) - TMath::Power( fProton_Mass_GeV , 2 ) ) /
      ( 2.0 * fProton_Mass_GeV * fQsq_GeV * ( 1.0 - fEpsilon ) );
    
    // ----------------------------------------------------
    //  Jacobian  dt/dcos(theta*)dphi in units of GeV2/sr
    // ----------------------------------------------------
    fJacobian_CM = ( fPhoton_Mom_RF_GeV - fBeta_CM_RF * fPhoton_Energy_RF_GeV ) / ( fGamma_CM_RF * ( 1.0 - TMath::Power(fBeta_CM_RF,2) ) );

    fA              = fJacobian_CM * fPion_Mom_CM_GeV / fPi;

    //fZASigma_Lab    = fZASigma_UU * fFlux_Factor_Col * fA;
    fZASigma_Lab    = ( fZASigma_UU + fRorySigma_UT) * fFlux_Factor_Col * fA;

    fZASigma_UUPara = fZASigma_UUPara * fFlux_Factor_Col * fA;

    // ----------------------------------------------------
    // Jacobian dOmega* / dOmega dimensionless
    // ----------------------------------------------------
    fJacobian_CM_RF  = ( TMath::Power(fPion_Mom_RF,2)*fW) / 
      ( fPion_Mom_CM * TMath::Abs( ( fProton_Mass + fPhoton_Energy_RF) * fPion_Mom_RF - 
				   ( fPion_Energy_RF * fPhoton_Mom_RF * TMath::Cos((fPi/180)*fPion_Theta_RF ) ) ) );

    // fJacobian_CM_Col = ( TMath::Power(fPion_Mom_Col,2)*fW) / 
    //   ( fPion_Mom_CM * TMath::Abs( ( fProton_Mass + fPhoton_Energy_Col ) * fPion_Mom_Col - 
    // 				   ( fPion_Energy_Col * fPhoton_Mom_Col * TMath::Cos((fPi/180)*fPion_Theta_Col ) ) ) );

    fJacobian_CM_Col = ( TMath::Power(fPion_Corrected_Mom_Col,2)*fW) / 
      ( fPion_Mom_CM * TMath::Abs( ( fProton_Mass + fPhoton_Energy_Col ) * fPion_Corrected_Mom_Col - 
				   ( fPion_Corrected_Energy_Col * fPhoton_Mom_Col * TMath::Cos((fPi/180)*fPion_Corrected_Theta_Col ) ) ) );


    fZASigma_Lab     = fZASigma_Lab  * fJacobian_CM_Col;
    fZASigma_UUPara  = fZASigma_UUPara * fJacobian_CM_Col;
    
    if ( fZASigma_Lab <= 0 ) { 
      fNSigmaNeg ++;
      continue;
    }

    // ------------------------------------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------------------------------------
    //             fZASigma_Lab          fPSF       fuBcm2         fLumi                  fNEvents
    // Hz        = ub / ( sr^2 * GeV ) * sr^2 * ( cm^2 / ub ) * ( # / ( cm^2 * sec ) ) / ( # )

    fEventWeight = fZASigma_Lab * fPSF * fuBcm2 * fLumi / fNEvents;   // in Hz

    if ( fEventWeightMax < fEventWeight )
      fEventWeightMax = fEventWeight;


    fSineAsym = ( 0.3686 * pow( ( fT_GeV - fT_Para_GeV ) , 0 ) + 
		  1.2910 * pow( ( fT_GeV - fT_Para_GeV ) , 1 ) - 
		  1.6970 * pow( ( fT_GeV - fT_Para_GeV ) , 2 ) );                

    if ( fT_GeV >= fT_Para_GeV )
      fSSAsym = 0.5468 * ( 1 - TMath::Exp( -1.0 * 30.62 * ( fT_GeV - fT_Para_GeV ) ) );
    if ( fT_GeV < fT_Para_GeV )
      fSSAsym = 0.0;
    

    // ------------------------------------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------------------------------------
    
    fNRecorded ++;
    fRatio = fNRecorded / fNGenerated;
    t1.Fill();

    // if ( fWSq_GeV > 0 ){
      
      fLundRecorded++;

      ppiOut << "3" << " \t " << fEventWeight << " \t " << fZASigma_Lab << " \t " << fSSAsym << " \t " << "0" << " \t "  
	     << fx << " \t " << fT_GeV << " \t " << fW_GeV << " \t " << fQsq_GeV  << " \t " << fEpsilon << endl;
      
      // Pion -
      ppiOut << setw(10) << "1" 
	     << setw(10) << "1" 
	     << setw(10) << "1" 
	     << setw(10) << "-211" 
	     << setw(10) << "0" 
	     << setw(10) << "0" 
	     << setw(16) << fPion_MomX_Col_GeV    
	     << setw(16) << fPion_MomY_Col_GeV   
	     << setw(16) << fPion_MomZ_Col_GeV  
	     << setw(16) << fPion_Energy_Col_GeV
	     << setw(16) << fPion_Mass_GeV
	     << setw(16) << fVertex_X
	     << setw(16) << fVertex_Y
	     << setw(16) << fVertex_Z
	     << endl;
    
      // Electron
      ppiOut << setw(10) << "2" 
	     << setw(10) << "-1" 
	     << setw(10) << "1" 
	     << setw(10) << "11" 
	     << setw(10) << "0" 
	     << setw(10) << "0" 
	     << setw(16) << fScatElec_MomX_Col_GeV  
	     << setw(16) << fScatElec_MomY_Col_GeV   
	     << setw(16) << fScatElec_MomZ_Col_GeV  
	     << setw(16) << fScatElec_Energy_Col_GeV
	     << setw(16) << fElectron_Mass_GeV
	     << setw(16) << fVertex_X
	     << setw(16) << fVertex_Y
	     << setw(16) << fVertex_Z
	     << endl;
	  
      // Proton
      ppiOut << setw(10) << "3" 
	     << setw(10) << "1" 
	     << setw(10) << "1" 
	     << setw(10) << "2212" 
	     << setw(10) << "0" 
	     << setw(10) << "0" 
	     << setw(16) << fRecoilProton_MomX_Col_GeV    
	     << setw(16) << fRecoilProton_MomY_Col_GeV   
	     << setw(16) << fRecoilProton_MomZ_Col_GeV  
	     << setw(16) << fRecoilProton_Energy_Col_GeV
	     << setw(16) << fRecoilProton_Mass_GeV
	     << setw(16) << fVertex_X
	     << setw(16) << fVertex_Y
	     << setw(16) << fVertex_Z
	     << endl;

    // }
  } // This is the loop over total events.

  ppiOut.close();
  t1.Write();
  tTime.Stop();
  tTime.Print();
  f->Close();
  TDatime deTime;
  cout << "End Time:   " << deTime.GetHour() << ":" << deTime.GetMinute() << endl;

  ppiDetails << "Total events tried                          " << setw(50) << fNGenerated   << endl;
  ppiDetails << "Total events recorded                       " << setw(50) << fNRecorded    << endl;
  ppiDetails << "Number of events with w less than threshold " << setw(50) << fWSqNeg       << endl;
  ppiDetails << "Number of events with mom not conserve      " << setw(50) << fNMomConserve << endl;
  ppiDetails << "Number of events with Sigma negative        " << setw(50) << fNSigmaNeg    << endl;
  ppiDetails << "Number of lund events                       " << setw(50) << fLundRecorded << endl;
  
  ppiDetails.close();
  
  return 0;
}
# endif
//---------------------------------------------------------