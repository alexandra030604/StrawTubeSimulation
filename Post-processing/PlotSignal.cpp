#include <iostream>
#include <cmath>

void PlotSignal(){
  std::string all_file = "Signal_gamma_iron/";

  TString filename = "straw_1atm_2mm_gamma_iron_test_garf_bin2000_";

  TGraph * grapf_all_file;
  TH1F * integral_all_file = new TH1F("integral_all_file", "integral_all_file", 120, 0., 2000);

  std::vector<Double_t> v_all_file;
  std::vector<Double_t> v_elec_file;

  for (int i = 0; i < 2000; i++){
    TString full_filename = all_file + filename + to_string(i) + ".sig";
    grapf_all_file = new TGraph(full_filename, "%lf %lf");
    Double_t integral = grapf_all_file -> Integral();
    integral_all_file -> Fill(integral * 1e15 );
    std::cout << "All parts: " << integral * 1e15 << std::endl;
    v_all_file.push_back(integral * 1e15);
  }

  std::cout << "Part of all: " << integral_all_file -> Integral() << " Electron: " <<
  integral_elec_file -> Integral() << " ion: " << integral_ion_file -> Integral() << std::endl;

  TFile * charge_file = new TFile("2mm_muon_test_garf_45kGain_10mm_straw_20000ns.root");
  TH1F * total_charge_histo = new TH1F("total_charge_histo", "total_charge_histo", 120, 0., 10000000.);
  total_charge_histo = (TH1F*)charge_file -> Get("gasgain_total_charge_electron");

  TCanvas * c1 = new TCanvas("c1","c1", 1600, 800);
  c1 -> cd();
  c1 -> SetGrid();
  //gPad -> SetLogy();
  gStyle -> SetOptStat(0);
  grapf_all_file -> SetTitle(" Signal plots; [s]; [mkA]");
  grapf_all_file -> SetLineColor(kBlack);
  grapf_all_file -> SetLineWidth(3);
  grapf_all_file -> Draw("APL");

  auto legend = new TLegend(0.6, 0.1, 0.9, 0.4);
  legend -> SetHeader("Signal plots","C");
  legend -> AddEntry(grapf_all_file, "Sum of electron and ion signals", "pl");
  legend -> Draw();

  c1 -> SaveAs("Signal_250ns.png");

  TCanvas * c2 = new TCanvas("c2", "c2", 1600, 800);
  c2 -> SetGrid();
  gStyle -> SetOptStat(0);
  integral_all_file -> SetTitle(" ; Q [fC]; ");
  integral_all_file -> SetLineColor(kBlack);
  integral_all_file -> SetLineWidth(3);
  integral_all_file -> Draw();
  auto legend1 = new TLegend(0.6, 0.9, 0.9, 0.7);
  legend1 -> SetHeader("Charge plots (Integrated Signals)","C");
  legend1 -> AddEntry(integral_all_file, "Charge of electron and ion signals, fC", "pl");
  legend1 -> Draw();
  c2 -> SaveAs("Integral_inElectrons_250ns.png");
}
