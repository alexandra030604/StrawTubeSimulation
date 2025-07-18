#include <iostream>

void draw_before_spice(){
  TString all_file = "straw_1atm_2mm_gamma_iron_test_garf_bin2000_4721.sig";
  TString elec_file = "straw_1atm_2mm_gamma_iron_test_garf_bin2000_4722.sig";
  TString ion_file = "straw_1atm_2mm_gamma_iron_test_garf_bin2000_4723.sig";
// before spice:
  TGraph * grapf_all_file;
  grapf_all_file = new TGraph(all_file, "%lg %lg", ""); 

  TGraph * grapf_elec_file;
  grapf_elec_file = new TGraph(elec_file, "%lg %lg", "");

  TGraph * grapf_ion_file;
  grapf_ion_file = new TGraph(ion_file, "%lg %lg", "");
// after spice: 
  /* TGraph *grapf_all_file = new TGraph(all_file, "%lg %*lg %lg", "");
  TGraph *grapf_elec_file = new TGraph(elec_file, "%lg %*lg %lg", "");
  TGraph *grapf_ion_file = new TGraph(ion_file, "%lg %*lg %lg", ""); */
  TCanvas * c1 = new TCanvas("c1","c1", 1600, 800); 
  c1 -> SetGrid();
  gStyle -> SetOptStat(0);
  grapf_all_file -> SetTitle(" Signal before LTSpice plots, gamma, Fe-55, angle 90 degs; [s]; [A]");
  grapf_all_file -> SetLineColor(kBlack);
  grapf_all_file -> SetLineWidth(3);
  grapf_all_file -> Draw("APL");
  grapf_elec_file -> SetLineColor(kBlue);
  grapf_elec_file -> Draw("SAME");
  grapf_ion_file -> SetLineColor(kRed);
  grapf_ion_file -> Draw("SAME");

  auto legend = new TLegend(0.6, 0.9, 0.9, 0.6);
  legend -> SetHeader("Signal before LTSpice plots","C");
  legend -> AddEntry(grapf_elec_file, "Signal #4721", "pl");
  legend -> AddEntry(grapf_ion_file, "Signal #4722", "pl");
  legend -> AddEntry(grapf_all_file, "Signal #4723", "pl");
  legend -> Draw();

  c1 -> SaveAs("Signal_before_SPICE_muon_1GeV.png");
}
