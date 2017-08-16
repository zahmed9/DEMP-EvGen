/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  This class uses GSL MathMore's MultiRootFinder to find the
  momenta of the produced particles. In this case, a proton,
  and a negative pion.
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <vector>
#include <stdio.h>
#include <iostream>

#include "ProductGen.hxx"
#include "CustomRand.hxx"

#include "TMath.h"

#include "RConfigure.h"
#ifndef R__HAS_MATHMORE
#error MathMore not installed. Rebuild root with GSL and MathMore.
#else
#include "Math/MultiRootFinder.h"
#include "Math/WrappedMultiTF1.h"
#endif

using namespace std;

double ProductGen::PconsX(double * x, double * p)
{
  return (p[0]+p[2])-(p[1]+x[0]+x[1]);
}

double ProductGen::PconsY(double * x, double * p)
{
  return (p[3]+p[5])-(p[4]+x[2]+p[3]);
}

double ProductGen::PconsZ(double * x, double * p)
{
  return (p[6]+p[8])-(p[7]+x[4]+x[5]);
}

double ProductGen::Econs(double * x, double * p)
{
  double Ee = TMath::Sqrt(p[0]*p[0]+p[3]*p[3]+p[6]*p[6]+p[9]*p[9]);
  double Eep= TMath::Sqrt(p[1]*p[1]+p[4]*p[4]+p[7]*p[7]+p[9]*p[9]);
  double Epi= TMath::Sqrt(x[0]*x[0]+x[2]*x[2]+x[4]*x[4]+p[10]*p[10]);
  double Ep = TMath::Sqrt(x[1]*x[1]+x[3]*x[3]+x[5]*x[5]+p[11]*p[11]);
  double En = TMath::Sqrt(p[2]*p[2]+p[5]*p[5]+p[8]*p[8]+p[12]*p[12]);
  //cout << p[6] << endl;
  return (Ee + En) - (Eep + Epi + Ep);
}

double ProductGen::AngleTheta(double * x, double * p)
{
  double Ppi = TMath::Sqrt(x[0]*x[0]+x[2]*x[2]+x[4]*x[4]);
  return Ppi*TMath::Cos(p[13]) - x[0];
}

double ProductGen::AnglePhi(double * x, double * p)
{
  return x[0]*TMath::Tan(p[14]) - x[2];
}

double ProductGen::W(double * x, double * p)
{
  double Ee = TMath::Sqrt(p[0]*p[0]+p[3]*p[3]+p[6]*p[6]+p[9]*p[9]);
  double Eep= TMath::Sqrt(p[1]*p[1]+p[4]*p[4]+p[7]*p[7]+p[9]*p[9]);
  double Epi= TMath::Sqrt(x[0]*x[0]+x[2]*x[2]+x[4]*x[4]+p[10]*p[10]);
  double Ep = TMath::Sqrt(x[1]*x[1]+x[3]*x[3]+x[5]*x[5]+p[11]*p[11]);
  double En = TMath::Sqrt(p[2]*p[2]+p[5]*p[5]+p[8]*p[8]+p[12]*p[12]);
  return TMath::Power(p[0]-p[1]+p[2],2)
    +TMath::Power(p[3]-p[4]+p[5],2)
    +TMath::Power(p[6]-p[7]+p[8],2)
    -TMath::Power(Ee-Eep+En,2)
    -(TMath::Power(x[0]+x[1],2)
      +TMath::Power(x[2]+x[3],2)
      +TMath::Power(x[4]+x[5],2)
      -TMath::Power(Epi+Ep,2));
}

double ProductGen::T(double * x, double * p)
{
  double Ee = TMath::Sqrt(p[0]*p[0]+p[3]*p[3]+p[6]*p[6]+p[9]*p[9]);
  double Eep= TMath::Sqrt(p[1]*p[1]+p[4]*p[4]+p[7]*p[7]+p[9]*p[9]);
  double Epi= TMath::Sqrt(x[0]*x[0]+x[2]*x[2]+x[4]*x[4]+p[10]*p[10]);
  double Ep = TMath::Sqrt(x[1]*x[1]+x[3]*x[3]+x[5]*x[5]+p[11]*p[11]);
  double En = TMath::Sqrt(p[2]*p[2]+p[5]*p[5]+p[8]*p[8]+p[12]*p[12]);
  return TMath::Power(p[0]-p[1]-x[0],2)
    +TMath::Power(p[3]-p[4]-x[2],2)
    +TMath::Power(p[6]-p[7]-x[4],2)
    -TMath::Power(Ee-Eep-Epi,2)
    -(TMath::Power(x[1]-p[2],2)
      +TMath::Power(x[3]-p[5],2)
      +TMath::Power(x[5]-p[8],2)
      -TMath::Power(Ep-En,2));
}

ProductGen::ProductGen()
{
  TF1 * fPconsX = new TF1("fPconsX", this->PconsX, 0, 100000, 15);
  TF1 * fPconsY = new TF1("fPconsY", this->PconsY, 0, 100000, 15);
  TF1 * fPconsZ = new TF1("fPconsZ", this->PconsZ, 0, 100000, 15);

  TF1 * fEcons = new TF1("fEcons", Econs, 0, 100000, 15);

  TF1 * fAngleTheta = new TF1("fAngleTheta", this->AngleTheta, 0, 1000, 15);
  TF1 * fAnglePhi = new TF1("fAnglePhi", this->AnglePhi, 0, 1000, 15);
  TF1 * fW = new TF1("fW",this->W, 0, 1000, 15);

  TF1 * fT = new TF1("fT",this->T,0,1000,15);

  EqnSys.push_back(*fPconsX);
  EqnSys.push_back(*fPconsY);
  EqnSys.push_back(*fPconsZ);

  EqnSys.push_back(*fEcons);

  EqnSys.push_back(*fAngleTheta);
  EqnSys.push_back(*fAnglePhi);
  EqnSys.push_back(*fW);

  EqnSys.push_back(*fT);

  ROOT::Math::WrappedMultiTF1 wPconsX(*fPconsX, 6);
  ROOT::Math::WrappedMultiTF1 wPconsY(*fPconsY, 6);
  ROOT::Math::WrappedMultiTF1 wPconsZ(*fPconsZ, 6);

  ROOT::Math::WrappedMultiTF1 wEcons(*fEcons, 6);

  ROOT::Math::WrappedMultiTF1 wAngleTheta(*fAngleTheta, 6);
  ROOT::Math::WrappedMultiTF1 wAnglePhi(*fAnglePhi, 6);

  ROOT::Math::WrappedMultiTF1 wW(*fW, 6);

  ROOT::Math::WrappedMultiTF1 wT(*fT, 6);

  //WrappedEqns.push_back(wPconsX);
  //WrappedEqns.push_back(wPconsY);
  WrappedEqns.push_back(wPconsZ);

  WrappedEqns.push_back(wEcons);

  WrappedEqns.push_back(wAngleTheta);
  WrappedEqns.push_back(wAnglePhi);

  WrappedEqns.push_back(wW);

  WrappedEqns.push_back(wT);

  Finder = new ROOT::Math::MultiRootFinder();

  for (int i = 0 ; i<nEqns; i++)
    Finder->AddFunction(WrappedEqns[i]);

  char AngleGenName[100] = "AngleGen";
  double dummy[2] = {0,1};
  double ThetaRange[2] = {4*TMath::DegToRad(), 25*TMath::DegToRad()};
  double PhiRange[2] = {0, 360*TMath::DegToRad()};
  AngleGen = new CustomRand(AngleGenName, dummy,
                            ThetaRange, PhiRange);


}


void ProductGen::SetIncident(Particle * inIncident)
{
  Incident = inIncident;
}

void ProductGen::SetTarget(Particle * inTarget)
{
  Target = inTarget;
}



void ProductGen::SetPars()
{
  pars[0] = Incident->Px();
  pars[1] = Scattered->Px();
  pars[2] = Target->Px();
  pars[3] = Incident->Py();
  pars[4] = Scattered->Py();
  pars[5] = Target->Py();
  pars[6] = Incident->Pz();
  pars[7] = Scattered->Pz();
  pars[8] = Target->Pz();

  pars[9] = 0.511;
  pars[10]= 140;
  pars[11]= 938;
  pars[12]= 940;

  pars[13]= AngleGen->Theta();
  pars[14]= AngleGen->Phi();

}

void ProductGen::Solve(Particle * inScattered)
{
  Scattered = inScattered;

  SetPars();

  //cout << "Done" << endl;

  for(int i = 0; i<nEqns; i++){
    WrappedEqns[i].SetParameters(pars);
    EqnSys[i].SetParameters(pars);
    //cout <<EqnSys[i].GetParameter(6) << endl;
  }


  Finder->SetPrintLevel(1);

  //ostream bitBucket(0);

  //Finder->PrintState(bitBucket);

  double x0[6] = {10,10,10,100,1000};

  //cout<< Econs(x0, pars) << endl;

  Finder->Solve(x0, 1000, 10);

  //cout << Finder->Status()<<endl;

}

Particle * ProductGen::ProdPion()
{
  const double * result = Finder->X();
  return new Particle(140, result[0], result[1], result[2]);
}

Particle * ProductGen::ProdProton()
{
  const double * result = Finder->X();
  return new Particle(938, result[3], result[4], result[5]);
}

void ProductGen::PrintPars()
{
  for (int i = 0; i<15; i++){
    cout << WrappedEqns[1].Parameters()[i] << endl;
  }
  cout << endl;
}
