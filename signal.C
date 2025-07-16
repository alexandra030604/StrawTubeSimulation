#include <iostream>
#include <fstream>
#include <cstdlib>
#include <filesystem>

#include <TCanvas.h>
#include <TROOT.h>
#include <TApplication.h>
#include <TRandom.h>
#include <TTree.h>

#include "Garfield/ViewCell.hh"
#include "Garfield/ViewDrift.hh"
#include "Garfield/ViewSignal.hh"

#include "TFile.h"
#include "TH2D.h"
#include "TMath.h"

//#include  "Garfield/Microscopic.hh""
#include "Garfield/ComponentAnalyticField.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/Sensor.hh"
#include "Garfield/DriftLineRKF.hh"
#include "Garfield/TrackHeed.hh"
#include "Garfield/AvalancheMC.hh"
#include "Garfield/ViewMedium.hh"

using namespace Garfield;

int main(int argc, char * argv[]) {
    // Creating a ROOT application
  TApplication app("app", &argc, argv)
    // File for recording charges;
  std::ofstream chargefile;

  // Variables for data storage
  double chrg_first = 0.;

  Double_t energy_loss, gasgain, ne_hist, nc_hist, ni_hist, first_cluster_time, second_cluster_time;
  Int_t total_charge_electron_tree, total_charge_ion_tree, primary_charge_tree;

  // Creating a tree for storing results
  TTree * tree = new TTree("tree_1atm_20deg_temp_1750volts", "tree_1atm_20deg_temp_1750volts");
  // Adding branches to the tree
  tree -> Branch("energy_loss", &energy_loss, "energy_loss/D");
  tree -> Branch("gasgain", &gasgain, "gasgain/D");
  tree -> Branch("total_charge_electron_tree", &total_charge_electron_tree, "total_charge_electron_tree/I");
  tree -> Branch("total_charge_ion_tree", &total_charge_ion_tree, "total_charge_ion_tree/I");
  tree -> Branch("primary_charge_tree", &primary_charge_tree, "primary_charge_tree/I");
  tree -> Branch("ne_hist", &ne_hist, "ne_hist/D");
  tree -> Branch("nc_hist", &nc_hist, "nc_hist/D");
  tree -> Branch("ni_hist", &ni_hist, "ni_hist/D");
  tree -> Branch("first_cluster_time", &first_cluster_time, "first_cluster_time/D");
  tree -> Branch("second_cluster_time", &second_cluster_time, "second_cluster_time/D");

  // Creating a gas environment
  MediumMagboltz gas;

  // Loading the gas file after generate.C
  gas.LoadGasFile("Ar_70_co2_30_1atm_T20_0T.gas");

  // Getting the Garfield installation path
  auto installdir = std::getenv("GARFIELD_INSTALL");
  if (!installdir) {
    std::cerr << "GARFIELD_INSTALL variable not set.\n";
    return 1;
  }
  // Loading ion mobility data. Indicate your path here.
  gas.LoadIonMobility("../ garfield / garfieldpp - master / install / share / Garfield / Data / IonMobility_Ar + _Ar.txt");
  
  // Setting up the Penning effect
  const double rPenning = 0.54;
  // Mean distance from the point of excitation.
  const double lambdaPenning = 0.;
  gas.EnablePenningTransfer(rPenning, lambdaPenning, "ar");

  // Make a component with analytic electric field.

  ComponentAnalyticField cmp;
  cmp.SetMedium(&gas);
  // Wire radius [cm]
  const double rWire = 15.e-4;
  // Outer radius of the xtube [cm]
  const double rTube = 0.5;
  // Voltages
  const double vWire = 1750.;
  const double vTube = 0.;
  // Add the wire in the centre.
  cmp.AddWire(0., 0, 2 * rWire, vWire, "s"); // (x, y, diameter, voltage, label)
  // Add the tube.
  cmp.AddTube(rTube, vTube, 0, "t"); // (radius, voltage, thickness, label)
  // Request calculation of the weighting field.
  cmp.AddReadout("s");
  // Set the magnetic field
  cmp.SetMagneticField(0, 0, 0); //(Bx, By, Bz)
  // Make a sensor.
  Sensor sensor;
  sensor.AddComponent(&cmp);
  sensor.AddElectrode(&cmp, "s");
  // Set the signal time window.
  const unsigned int nbins = 2000;
  const double tmin = 0;
  const double tmax = 250.;
  const double tstep = (tmax - tmin) / nbins;
 sensor.SetTimeWindow(tmin, tstep, nbins);

  TH1D hsignal("Name","Tittle", nbins, tmin, tmax);

  // Set up Heed.
  TrackHeed track;
  track.SetParticle("muon");
  track.SetEnergy(1.e9);
  track.SetSensor(&sensor);
  //track.EnableMagneticField();

  DriftLineRKF drift;

drift.SetSensor(&sensor); // Assigns the sensor
drift.SetGainFluctuationsPolya(1, 57000, true); // Polya model for gain fluctuations (actually, not 57k, but 48k)
drift.EnableIonTail(); // Enables ion tail simulation (slow ions drifting to cathode)
drift.EnableSignalCalculation(); // Enables signal induction calculation
  TCanvas* cD = nullptr;
  ViewCell cellView;
  ViewDrift driftView;
  constexpr bool plotDrift = false; // If plotDrift = true, the code generates a plot showing: 1. Detector geometry(wires, cathode) via ViewCell. 2.Particle tracks(from TrackHeed) and drift lines(from DriftLineRKF).
  if (plotDrift) {
    cD = new TCanvas("cD", "", 600, 600);
    cellView.SetCanvas(cD);
    cellView.SetComponent(&cmp);
    driftView.SetCanvas(cD);
    track.EnablePlotting(&driftView);
      drift.EnablePlotting(&driftView);
  }

  TCanvas* cS = nullptr;
  ViewSignal signalView;
  constexpr bool plotSignal = false; // If plotSignal=true, the code would plot electrode signals 
  if (plotSignal) {
    cS = new TCanvas("cS", "", 600, 600);
    signalView.SetCanvas(cS);
    signalView.SetSensor(&sensor);
    signalView.SetLabelY("signal [#muA]");
  }

  double chrg = 0.;

  // Open event data file
  std::ofstream eventfile;
  std::string outname_eventfile = "eventfile_Ar_70_co2_30_1750V_B0_1_atm_test_2mm_muon_test_garf_bin2000.txt";
  eventfile.open(outname_eventfile, std::ios::out);

  // Track parameters:
  const unsigned int nTracks = 2000;
  double rTrack = 0.2; // Track radius (offset from center). 2 mm
  double x0 = rTrack; // Initial x
  double y0 = -sqrt(rTube * rTube - rTrack * rTrack)+0.001; // Initial y (ensures gas volume penetration)

  for (unsigned int j = 0; j < nTracks; ++j) {
    Double_t n_events = 0.;
    track.NewTrack(x0, y0, 0, 0, 0, 1., 0); // angle 40 deg = 1.19175; angle 26 deg = 2.0503
    sensor.ClearSignal();
    // Cluster data variables:
    double xc = 0., yc = 0., zc = 0., tc = 0., ec = 0., extra = 0.;
    int nc = 0;
    int primary_charge = 0.;
    int total_charge_electron = 0.;
    int total_charge_ion = 0.;
    Double_t energy_deposit = 0.;
    double tmin = std::numeric_limits<double>::max();
    double sec_min = std::numeric_limits<double>::max();

  while (track.GetCluster(xc, yc, zc, tc, nc, ec, extra)) {
    //std::cout << "nc = " << nc << std::endl;
    primary_charge += nc;
    energy_deposit += ec;
    //std::cout << "Energy: " << ec << " XCoord: " << xc << " YCoord: " << yc << " ZCoord: " << zc << std::endl;
    n_events++;
    //std::cout << n_events << std::endl;
    //coordinate_change -> Fill(yc);
     
    // Process each electron in cluster:
     for (int k = 0; k < nc; ++k) {
        double xe = 0., ye = 0., ze = 0., te = 0., ee = 0.;
        double dx = 0., dy = 0., dz = 0.;
        track.GetElectron(k, xe, ye, ze, te, ee, dx, dy, dz);
        drift.DriftElectron(xe, ye, ze, te);
        double xf = 0., yf = 0., zf = 0., tf = 0.;
        int stat = 0;
        drift.GetEndPoint(xf, yf, zf, tf, stat);
        tmin = std::min(tmin, tf);
        if((tf > tmin) && (tf < sec_min) ){
          sec_min = tf;
        }

        double ne = 0., ni = 0.;
        drift.GetAvalancheSize(ne, ni);
        total_charge_electron += ne;
        total_charge_ion += ni;
        ne_hist = ne;
        ni_hist = ni;
    }
  }
  std::cout << "first cluster: " << tmin << " Second cluster: " << sec_min << std::endl;
  first_cluster_time = tmin;
  second_cluster_time = sec_min;
  Double_t dx = sqrt(rTube * rTube - rTrack * rTrack);
  // Convert energy to keV:
  energy_loss = energy_deposit / 1000;
  //eventfile << n_events << std::endl;
  // Calculate gas gain:
  if (primary_charge != 0){
    gasgain = total_charge_electron / primary_charge;
    total_charge_electron_tree = total_charge_electron;
    primary_charge_tree = primary_charge;
    total_charge_ion_tree = total_charge_ion;
  }
  // Write signals to file (before running the code, create the directory 'Signal_1atm_1GeV_2000V'):
  std::string outname_all = "Signal_1atm_1GeV_2000V/straw_1_74atm_4_8mm_1GeV_min_muon_test_garf_bin2000_" + std::to_string(j)+".sig";
  std::ofstream outfile_all;

  //if (energy_deposit / 1000 > 0.1){
  std::cout << energy_deposit / 1000 << std::endl;
  outfile_all.open(outname_all, std::ios::out);

   for (unsigned int i = 0; i < nbins; ++i) {
     const double t = (i + tmin) * tstep;
       double f = sensor.GetSignal("s", i);
       outfile_all << t/1000000000.0<< " " << f / 1000000.0 << std::endl;
   }
 //}
  //signalView.PlotSignal("s");
  tree -> Fill();
  outfile_all.close();
}
chargefile.close();
eventfile.close();

TFile f("total_charge_1GeV_90degrees_2mm.root","new");
tree -> Write();

TCanvas s(" "," ",600,600);
std::cout<<"done"<<std::endl;
}
