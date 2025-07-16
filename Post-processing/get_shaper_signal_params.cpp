#include <iostream>
#include <cmath>
#include <string>
#include <fstream>

#include "TCanvas.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
//#include "TRandom2.h"

using namespace std;

int get_shaper_signal_params(string file_name_prefix="gamma_2mm_fe55", int signal_count = 1, string out_prefix = "gamma_2mm_fe55", int hist_bin_preset = 1)
{
    ifstream filein;
    string filename;

    double h1_l1 = 0., h1_l2 = 5., h1_r1 = 300., h1_r2 = 1200.;
    int h1_b1 = 250, h1_b2 = 300;
    double h4_l = 0, h4_r = 250;
    int h4_b = 50;
    double h5_l = 0, h5_r = 250;
    int h5_b = 50;
    double h6_l = 0, h6_r = 1500;
    int h6_b = 100;
    double fit1_l = h4_r * 0.1, fit1_r = h4_r * 0.9;
    double fit2_l = h5_r * 0.1, fit2_r = h5_r * 0.9;

    switch(hist_bin_preset)
    {
        case 1: //1 cm tube; noAngle; noField; 1 mm dist; peaking time 25 nsec
        h4_b = 50;
        h4_l = 0;
        h4_r = 200;
//borders and binning
        h5_b = 80;
        h5_l = -50.;
        h5_r = 200.;
//borders of fit
        fit2_l = 25;
        fit2_r = 60;
        break;
    }

    TH2D signal_collection("SPICE_signals", "Garfield+LTSpice signals;Time [nsec];Amplitude [mV]", h1_b1, h1_l1, h1_r1, h1_b2, h1_l2, h1_r2); //h1
    TH1D time_of_signal_max("time_distr", "Time of signal extremum;Time [nsec]", h4_b, h4_l, h4_r); //h4
    TH1D amp_spectra("peak_distr", "Maximum of amplitude;Amplitude [mV]", h6_b, h6_l, h6_r); //h6

    const int NofLowThr = 12;
    TH1D tOfLowThr[NofLowThr];
    //threshold scan -- from 0.1 mV to 15 mV
    float valOfLowThr[NofLowThr] = {0.0001, 0.001, 0.002, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009, 0.010, 0.015}; // thresholds [V]
    for(int ii=0; ii<NofLowThr; ii++){
        TString str  = "Moment of "; str+=int(valOfLowThr[ii]*1000); str+= " mV crossing;Time [nsec]";
        TString nstr = "tOfLowThr["; nstr+=ii; nstr+="]";
        tOfLowThr[ii]=TH1D(nstr.Data(), str.Data(), h5_b, h5_l, h5_r); //h5
    }

    TFile outfile("gamma_2mm_fe55.root", "recreate");


    for(int sigN = 0; sigN <= 1999; sigN++) // <= modified for debug
    {
        filename = "Signal_gamma_iron_results/straw_1atm_2mm_gamma_iron_test_garf_bin2000_" + std::to_string(sigN) + ".sig.csv";
        filein.open(filename.c_str(), ios::in);
        if(!filein.is_open())
        {
            cout << "File <<" << filename << ">> not open!" << endl;
            return 0;
        }

        string line;
        getline(filein, line);
        getline(filein, line);

        double time, current, voltage;
        double signal_max = -1, signal_max_time = 0;
        bool thr_rec[NofLowThr]; for (int ii=0; ii<NofLowThr; ii++) thr_rec[ii] = false;

        while(!filein.eof())
        {
            filein >> time >> current >> voltage;
            //cout << "Signal #" << sigN << "   time: " << time*1000000000 << "   voltage: " << voltage*1000 << endl;
            //all signals on one plot -- for debug
            signal_collection.Fill(time*1000000000 - 50, voltage*1000);

            if(signal_max < voltage)
            {
                signal_max = voltage;
                signal_max_time = time;
            }

            for(int ii=0; ii<NofLowThr; ii++){
                if(!thr_rec[ii]){
                    if(voltage > valOfLowThr[ii]){
                      //moment of threshold crossing distribution
                        tOfLowThr[ii].Fill(time*1000000000 - 50);
                        thr_rec[ii] = true;
                    }
                }
            }
        }
        //time of signal maximum
        time_of_signal_max.Fill(signal_max_time*1000000000 - 50);
        //maximum voltage value aka amplitude
        amp_spectra.Fill(signal_max*1000);
        //cout << "Signal #" << sigN << "   time for signal extremum: " << signal_max_time*1000000000 << endl << endl;

        filein.close();
        filein.clear();
    }

     outfile.WriteObject(tOfLowThr, "time_of_low_threshold");
     outfile.WriteObject(&signal_collection, "signal_collection" );
     outfile.WriteObject(&time_of_signal_max, "time_of_signal_max");
     outfile.WriteObject(&amp_spectra, "Maximum of amplitude");

    auto canv = new TCanvas("canv", "Find average output", 300, 150, 1000, 600);
    gStyle->SetOptStat(0);

    signal_collection.Draw("COL");
    canv->Update();
    filename = out_prefix + "_allSig.png";
    canv->SaveAs(filename.c_str());

    gStyle->SetOptStat(1110);
    gStyle->SetOptFit(1111);
    gStyle->SetStatX(0.95);
    gStyle->SetStatY(0.92);
    canv->SetGrid(0, 0);

    time_of_signal_max.Draw();
    canv->Update();
    filename = out_prefix + "_maxOfSig.png";
    canv->SaveAs(filename.c_str());

    TF1 * fit2[NofLowThr];
    TF1 * fit3[NofLowThr];
    for(int ii=0; ii<NofLowThr; ii++){
        tOfLowThr[ii].Draw();
        //fill moment of threshold crossing distribution with Gaussian function
        fit2[ii] = new TF1("fit_red2","gaus", 30, 80);
        fit2[ii]->SetLineStyle(4);
        fit2[ii]->SetLineWidth(3);
        tOfLowThr[ii].Fit(fit2[ii], "R");
        canv->Update();

        /// !!!TBD!!! get convergence
        double mean = fit2[ii]->GetParameter(1); double sigma = fit2[ii]->GetParameter(2);
        fit3[ii] = new TF1("fit_red3","gaus", mean-3*sigma, mean+3*sigma);
        fit3[ii]->SetLineWidth(3);
        tOfLowThr[ii].Fit(fit3[ii], "R");
        canv->Update();

        /// the grass -- left content noise
        double lowEdge = fit3[ii]->GetParameter(1)-3*fit3[ii]->GetParameter(2);
        int binN = 0;
        int stat = 0;
        while((tOfLowThr[ii].GetBinCenter(binN)<lowEdge) && (binN<tOfLowThr[ii].GetNbinsX())) {
            stat+=tOfLowThr[ii].GetBinContent(binN);
            binN++;
        };
        std::cout << "threshold " << valOfLowThr[ii] << " V; left content [%] " << stat/tOfLowThr[ii].Integral()*100 << std::endl;

        filename = out_prefix +"_"+(valOfLowThr[ii])*1000+"_thrCross.png";
        canv->SaveAs(filename.c_str());

    };

    gStyle->SetOptStat(111111);
    amp_spectra.Draw();
    canv->Update();
    filename = out_prefix + "_ampSpectra.png";
    canv->SaveAs(filename.c_str());

    return 0;
}
