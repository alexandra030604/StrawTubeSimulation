#include <iostream>

void Energy_dep(){
  TFile * file_muon_tree = new TFile("4_8mm_muon_test_garf_45kGain_10mm_straw_1atm_wire_primaries_10k_events_no_penning.root");

  Double_t e_loss, gas_gain;
  Int_t total_charge, prim_charge;

  TTree * tree_muon = (TTree *)file_muon_tree -> Get("tree_1atm_20deg_temp_1750volts");

  tree_muon -> SetBranchAddress("energy_loss", &e_loss);
  tree_muon -> SetBranchAddress("gasgain", &gas_gain);
  tree_muon -> SetBranchAddress("total_charge_electron_tree", &total_charge);
  tree_muon -> SetBranchAddress("primary_charge_tree", &prim_charge);

  TH1D * hist_energy_dep = new TH1D("", "Compare of deposited energy and energy, spent on ion pair creation, muon 1 GeV, 4.8 mm to wire; Number of primaries", 150, 0., 6.);
  TH1D * hist_primary_elecs = new TH1D("", "Compare of deposited energy and energy, spent on ion pair creation, muon 1 GeV, 4.8 mm to wire; Number of primaries", 150, 0., 6.);
  TH1D * hist_total_charge = new TH1D("", "", 100, 0., 120);

  Int_t entries = (Int_t)tree_muon -> GetEntries();
  for (int i = 0; i < entries; i++){
    tree_muon -> GetEntry(i);
    hist_energy_dep -> Fill(e_loss);
    hist_primary_elecs -> Fill((double)prim_charge * 27.1/1000. );
    hist_total_charge -> Fill((double)total_charge / 45000. * 27.1/1000.);
  }

  TCanvas * c1 = new TCanvas("", "", 1600, 1200);
  c1 -> cd();
  gPad -> SetGrid();
  gStyle->SetOptFit(1);
  hist_energy_dep -> SetStats(1);
  //hist_energy_dep -> SetMaximum(290);
  hist_energy_dep -> Fit("landau", "", "", 0., 3.);
  hist_energy_dep -> SetLineColor(kBlue);
  hist_energy_dep -> SetLineWidth(2);
  hist_energy_dep -> GetFunction("landau") -> SetLineColor(kBlue);

  hist_primary_elecs -> SetLineColor(kGreen+2);
  hist_primary_elecs -> SetLineWidth(2);
  //hist_primary_elecs -> SetLineStyle(10);
  hist_primary_elecs -> Fit("landau", "", "", 0., 3.);
  hist_primary_elecs -> GetFunction("landau") -> SetLineColor(kGreen+2);

  hist_total_charge -> SetLineColor(kRed);
  hist_total_charge -> SetLineWidth(2);
  //hist_total_charge -> SetLineStyle(5);
  hist_total_charge -> Fit("landau", "", "", 0., 3.);
  hist_total_charge -> GetFunction("landau") -> SetLineColor(kRed);
  hist_primary_elecs -> Draw("hist ");
  hist_energy_dep -> Draw("hist SAMES");
//  hist_total_charge -> Draw("hist SAMES");
  hist_energy_dep -> GetFunction("landau") -> Draw("SAMES");
  hist_primary_elecs -> GetFunction("landau") -> Draw("SAMES");
  //hist_total_charge -> GetFunction("landau") -> Draw("SAMES");

  //std::cout << hist_energy_dep -> Integral(0, 16.) << std::endl;

  TF1 * function = (TF1 *)hist_energy_dep -> GetListOfFunctions()->FindObject("landau");
  Double_t sigma_edep = function -> GetParameter(2);
  Double_t MPV_edep = function -> GetParameter(1);

  TF1 * function1 = (TF1 *)hist_primary_elecs -> GetListOfFunctions()->FindObject("landau");
  Double_t sigma_prim = function1 -> GetParameter(2);
  Double_t MPV_prim = function1 -> GetParameter(1);

  TF1 * function2 = (TF1 *)hist_total_charge -> GetListOfFunctions()->FindObject("landau");
  Double_t sigma_total = function2 -> GetParameter(2);
  Double_t MPV_total = function2 -> GetParameter(1);

  auto legend1 = new TLegend(0.6, 0.9, 0.9, 0.7);
  //legend1 -> SetHeader("Compare of deposited energy and energy, spent on ion pair creation, muon 1 GeV, 2 mm to wire","C");
  legend1 -> AddEntry(hist_energy_dep, TString::Format("#splitline{Energy deposit * 1000 / 27}{MPV = %.3f, #sigma = %.3f}", MPV_edep, sigma_edep), "pl");
  legend1 -> AddEntry(hist_primary_elecs, TString::Format("#splitline{Number of primaries }{MPV = %.3f, #sigma = %.3f} ", MPV_prim, sigma_prim), "pl");
  //legend1 -> AddEntry(hist_total_charge, TString::Format("#splitline{Number of total electrons / Gas Gain * 27 / 1000}{MPV = %.3f, #sigma = %.3f}", MPV_total, sigma_total), "pl");
  legend1 -> Draw();

  c1 -> SaveAs("de_muon_4_8mm.png");

  std::cout << hist_energy_dep -> GetMaximumBin() << std::endl;
  std::cout << hist_energy_dep -> GetBinCenter(hist_energy_dep -> GetMaximumBin()) << std::endl;

  Int_t skipcontent = 0;
  for (int i = 1; i < hist_energy_dep -> GetMaximumBin(); i++){
    skipcontent += hist_energy_dep -> GetBinContent(i);
  }

  TH1D * fluctuations_hist = new TH1D("", "", 75, 0., 150.);

  for(Int_t i = skipcontent; i < (skipcontent+hist_energy_dep -> GetBinContent(hist_energy_dep -> GetMaximumBin())); i++){
    tree_muon -> GetEntry(i);
    fluctuations_hist -> Fill(prim_charge);
    std::cout << i << ": " << prim_charge << std::endl;
  }

  TCanvas * c2 = new TCanvas("c1", "c1", 1200, 1600);
  c2 -> cd();
  gPad -> SetGrid();
  //gStyle -> SetOptStats(0);
  TF1* func = new TF1("f", "[0] * TMath::Poisson((x/[2]), ([1]/[2]))",
                 fluctuations_hist->GetXaxis()->GetXmin(), fluctuations_hist->GetXaxis()->GetXmax());
  func->SetParameters(fluctuations_hist->GetBinContent(fluctuations_hist->GetMaximumBin()) / 0.18, // "Constant"
    fluctuations_hist->GetMean(), // "Mean"
    fluctuations_hist->GetMean() / 5.);
  func->SetParNames("Constant", "Mean", "XScaling");
  func->SetNpx(1000);

  gStyle -> SetOptFit(1);
  fluctuations_hist -> SetTitle("Fluctuations of energy: number of primaries in mean dE bin; ; N");
  fluctuations_hist -> Draw();
  fluctuations_hist -> Fit("landau");
  c2 -> SaveAs("Fluctuations_4_8mm.png");

}
