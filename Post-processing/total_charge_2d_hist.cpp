#include <iostream>
#include <algorithm>
using namespace std;

void total_charge_2d_hist(){
  ifstream filein;
  std::string filename_vitalii;

  Int_t total_charge_array[2000];

  Int_t overflow = 0;
  Int_t underflow = 0;

  TH2D * total_charge_vs_amplitude = new TH2D("total_charge_vs_amplitude", "Total charge vs Amplitude, proton, 0.3 GeV, 26 degrees, 25 ns peaking time; Charge [e]; Amplitude [mV]", 100, 0., 40000000., 100, 0., 1500.);
  TH1D * total_charge_hist = new TH1D("total_charge_hist", "Total charge, proton, 0.3 GeV, 90 degrees, 25 ns peaking time;  Charge [e]; ", 80, 0., 30000000.);
  TH1D * total_charge_hist_trimmed = new TH1D("total_charge_hist_trimmed", "Total charge trimmed mean, proton, 0.3 GeV, 26 degrees, 25 ns peaking time;  Charge [e]; ", 80, 0., 100000000.);
  TH1D * amplitude_hist = new TH1D("amplitude_hist", "Amplitude, proton, 1 GeV, 26 degrees, 25 ns peaking time; Amplitude [mV]; ", 100, 0., 1500.);
  Int_t total_charge;

  TFile * file = new TFile("2mm_muon_1GeV_26deg_test_garf_45kGain_10mm_straw_1atm_wire.root", "READ");
  TTree * tree = (TTree*)file -> Get("tree_1atm_20deg_temp_1750volts");
  tree -> SetBranchAddress("total_charge_electron_tree", &total_charge);
  Int_t nEntries = (Int_t)tree-> GetEntries();

  for (Int_t i = 0; i < nEntries; i++){
    tree -> GetEntry(i); //read the tree
    //read after SPICE files
    filename_vitalii = "26deg_results_1GeV_proton/straw_2mm_1GeV_26deg_min_proton_test_garf_bin2000_" + std::to_string(i) + ".sig.csv";
    filein.open(filename_vitalii.c_str(), ios::in);
    string line;
    getline(filein, line);
    getline(filein, line);
    Double_t time, current, voltage;
    Double_t signal_max = -1;
    //Double_t integral_after_spice = 0;

    while(!filein.eof()){
      filein >> time >> current >> voltage;
      if(signal_max < voltage)
      {
          signal_max = voltage;
      }
    }
    // cout << "Signal #" << i << "   time: " << time*1000000000 << "   voltage: " << voltage*1000 << "  total_charge: " << total_charge << endl;
    filein.close();
    filein.clear();
    if (signal_max*1000 >= 800){
      overflow++;
    } else if (signal_max*1000 <= 9){
      underflow++;
    }
    total_charge_vs_amplitude -> Fill(total_charge, signal_max*1000);
    total_charge_hist -> Fill(total_charge);
    total_charge_array[i] = total_charge;
    // total_charge_hist_trimmed -> Fill();

    amplitude_hist -> Fill(signal_max*1000);
  }

  std::sort(begin(total_charge_array), end(total_charge_array));

  for (int i = 0; i < 2000; i ++){
    if (i < 1334){
      total_charge_hist_trimmed -> Fill(total_charge_array[i]);
    }
  }

  TCanvas * c1 = new TCanvas("c1", "c1", 1600, 1200);
  c1 -> cd();
  gPad -> SetGrid();
  total_charge_vs_amplitude -> Draw("COLZ");
  c1 -> SaveAs("total_charge_25nc_muon_1GeV_26degrees.png");

  TCanvas * c2 = new TCanvas("c2", "c2", 1600, 1200);
  c2 -> Divide(2, 1);
  c2 -> cd(1);
  gPad -> SetGrid();
  total_charge_hist_trimmed -> Fit("landau", "", "", 0., 80000000);
  // TF1 *g = (TF1*)total_charge_hist->GetListOfFunctions()->FindObject("landau");
  // std::cout << "Sigma / MPV = " << g -> GetParameter(2) / g -> GetParameter(1) << std::endl;
  gStyle -> SetOptFit(1);
  total_charge_hist_trimmed -> Draw();
  c2 -> cd(2);
  gPad -> SetGrid();
  total_charge_hist -> Fit("landau", "", "", 3000000., 12000000.);
  gStyle -> SetOptFit(1);
  total_charge_hist -> Draw();
  c2 -> SaveAs("total_charge_and_voltage_25nc_muon_1GeV_90degrees.png");

  std::cout << "overflow: " << overflow << " underflow: " << underflow << std::endl;
}
