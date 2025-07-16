#include <iostream>
#include <vector>

void GetAll90Percents(){
  std::string Garfieldpp_path = "ParticlesSet/";
  std::string Garfieldpp_particles[] = { "electron", "kaon", "muon", "pion", "proton" };
  std::string distance_arr[] = { "0.1", "2", "4.8" };
  std::string energy_arr[] = {"0.1", "1", "10"};
  std::vector<TString> all_root_files;
  std::vector<TH1F *> all_histograms;

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
          //hist -> GetXaxis() -> Set(200, 0., range);
          all_histograms.push_back(hist);
      }
    }
  }
  std::vector<Double_t> x_coord;

  for (int i = 0; i < 45; i++){
    all_histograms[i] -> Scale(1 / (all_histograms[i] -> Integral()), "width");
    Double_t x;  // position where to compute the quantiles in [0,1]
    Double_t q = 0.5;  // array to contain the quantiles
    all_histograms[i] -> GetQuantiles(1, &x, &q);
    x_coord.push_back(x);
  }

  for (int i = 0; i < x_coord.size(); i++){
    if(all_root_files[i].Contains("2mm_")){
      std::cout << i << ": " << all_root_files[i] << " quantiles: " << x_coord[i] << std::endl;
    }
  }

}
