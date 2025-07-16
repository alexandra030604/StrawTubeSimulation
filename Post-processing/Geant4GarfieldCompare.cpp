#include <iostream>

void Geant4GarfieldCompare(){
  Double_t range = 20.;
  std::string Garfieldpp_path = "ParticlesSet/";
  std::string Garfieldpp_particles[] = { "electron", "kaon", "muon", "pion", "proton" };

  std::string distance_arr[] = { "0.1", "2", "4.8" };
  std::string energy_arr[] = {"0.1", "1", "10"};

  std::vector<TString> all_root_files;
  std::vector<TH1F *> all_histograms;
  //std::cout << filename_Garfieldpp << std::endl;

  for (auto momentum : energy_arr){
    for (auto particle : Garfieldpp_particles){
      for (auto distance : distance_arr){

          TString filename_Garfieldpp = Garfieldpp_path + "Momentum_" + momentum + "GeV/" +
          particle + "/" + distance + "mm_" + particle +
          "_" + momentum + "GeV_No_Magnetic.root";

          //std::cout << filename_Garfieldpp << std::endl;
          all_root_files.push_back(filename_Garfieldpp);

          TFile * file = TFile::Open(filename_Garfieldpp);
          TString histo_name = "Energy_loss";
          TH1F * hist = (TH1F *)file->Get(histo_name);
          hist -> GetXaxis() -> Set(200, 0., range);
          all_histograms.push_back(hist);
      }
    }
  }

  for (int i = 0; i < all_root_files.size(); i++){
    std::cout << "i: " << i << "   " << all_root_files[i] << std::endl;
  }

  std::string mms_gf[all_root_files.size()];
  for (int i = 0; i < all_root_files.size(); i++){
    //std::cout << all_root_files[i] << std::endl;
    if (i % 3 == 0){
      mms_gf[i] = "0.1";
    }  else if (i % 3 == 1) {
      mms_gf[i] = "2";
    } else if (i % 3 == 2){
      mms_gf[i] = "4.8";
    }
  }

  TCanvas * c1 = new TCanvas("c1", "c1", 1600, 900);
  c1 -> cd();
  c1 -> SetGrid();
  //gStyle -> SetOptStats(0);
  auto legend1 = new TLegend(0.3, 0.9, 0.9, 0.3);

  for (int i = 27; i < 30; i++){
    all_histograms[i] -> Scale(1 / (all_histograms[i] -> Integral()), "width");
    all_histograms[i] -> SetTitle(" ; dE [keV]; ");
    all_histograms[i] -> SetStats(0);
    all_histograms[i] -> SetMaximum(1.5);
    all_histograms[i] -> GetXaxis() -> SetRangeUser(0., range);
    all_histograms[i] -> SetLineColor(i - 22);
    all_histograms[i] -> SetLineWidth(3);
    all_histograms[i] -> Draw("SAME HIST");

    all_histograms[i] -> Fit("landau", "", "", 0., 10.);
    all_histograms[i] -> GetFunction("landau") -> SetLineColor(i - 22);
    all_histograms[i] -> GetFunction("landau") -> Draw("SAME");

    TF1 * function = (TF1 *)all_histograms[i] -> GetListOfFunctions()->FindObject("landau");
    Double_t sigma = function -> GetParameter(2);
    Double_t MPV = function -> GetParameter(1);

   std::string Legend_entry = "Distance = " + mms_gf[i] + " mm ";
    legend1 -> AddEntry(all_histograms[i], TString::Format( "#splitline{%s}{MPV = %.2f, #sigma = %.2f, Garfield++}", Legend_entry.c_str(), MPV, sigma), "pl");
  }

  //try to Get Geant4 particles
  std::vector<TString> all_root_files_G4;
  std::vector<TH1F *> all_histograms_G4;
  std::string Geant4_path = "22.04.2024_compilation/";
  std::string Geant4_dirs[] = { "01mm/", "2mm/", "4_8mm/" };
  std::string Geant4_particles[] = { "elec", "kaon_minus", "mu", "pi_minus", "proton" };

  for (auto dirs : distance_arr){
    for (auto energy : energy_arr){
      for (auto particle : Geant4_particles){
        TString file_Geant4_name = Geant4_path + dirs + "mm/" + particle + "_" + energy + "Gev_mom_charge_" + dirs + "mm_10k.root";
        //std::cout << file_Geant4_name << std::endl;
        all_root_files_G4.push_back(file_Geant4_name);
        TFile * file = TFile::Open(file_Geant4_name);
        TTree * tree_charge = (TTree *)file->Get("Scoring");
        TH1F * histo_Geant4_charge = new TH1F("", "", 200, 0., range);
        Double_t n_charges = 0;
        tree_charge -> SetBranchAddress("EventEdep", &n_charges);
        for (int i = 0; i < tree_charge -> GetEntries(); i++){
          tree_charge -> GetEntry(i);
          histo_Geant4_charge -> Fill(n_charges);
        }
        histo_Geant4_charge -> GetXaxis() -> Set(200, 0., range);
        all_histograms_G4.push_back(histo_Geant4_charge);
      }
    }
  }

  std::string mms[all_root_files_G4.size()];
  for (int i = 0; i < all_root_files_G4.size(); i++){
    std::cout << all_root_files_G4[i] << std::endl;
    if (i < 15){
      mms[i] = "0.1";
    }  else if (i >= 15 && i < 30) {
      mms[i] = "2";
    } else if (i >= 30){
      mms[i] = "4.8";
    }
  }

  for (int i = 0; i < all_root_files_G4.size(); i++){
    if(all_root_files_G4[i].Contains("proton_") && all_root_files_G4[i].Contains("_1Gev")){
      std::cout << all_root_files_G4[i] << std::endl;
      all_histograms_G4[i] -> GetXaxis() -> Set(200, 0., range);
      all_histograms_G4[i] -> Scale(1 / (all_histograms_G4[i] -> Integral()), "width");
      //std::cout << all_histograms_G4[i] -> Integral() << std::endl;
      all_histograms_G4[i] -> SetLineColor(i / 10 + 1);
      all_histograms_G4[i] -> SetStats(0);
      all_histograms_G4[i] -> SetLineWidth(3);
      all_histograms_G4[i] -> Draw("SAME HIST");

      all_histograms_G4[i] -> Fit("landau", "", "", 0., 10.);
      all_histograms_G4[i] -> GetFunction("landau") -> SetLineColor(i / 10 + 1 );
      all_histograms_G4[i] -> GetFunction("landau") -> Draw("SAME");

      TF1 * function = (TF1 *)all_histograms_G4[i] -> GetListOfFunctions()->FindObject("landau");
      Double_t sigma = function -> GetParameter(2);
      Double_t MPV = function -> GetParameter(1);
      std::string Legend_entry = "Distance " + mms[i] + " mm";
      legend1 -> AddEntry(all_histograms_G4[i], TString::Format( "#splitline{%s}{MPV = %.2f, #sigma = %.2f, Geant4}", Legend_entry.c_str(), MPV, sigma), "pl");
    }
  }

  legend1 -> Draw();
  c1 -> SaveAs("Proton_dedx_1GeV_G4_Garfpp.png");

}
