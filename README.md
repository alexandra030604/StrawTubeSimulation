# StrawTubeSimulation
Straw tube signal simulation based on Garfield++ framework

Garfield++ User Guide:
https://garfieldpp.web.cern.ch/doxygen/UserGuide.pdf

Main generation:
1. generate.C for gas file
2. signal.C for signal generation

Post-processing
1. get_shaper_signal_params.cpp
Post-LTSpice processing code. Provides threshold scan (from 0.1 mV to 15 mV) (_thrCross postfix), amplitude distribution (_ampSpectra postfix), time of signal maximum (_maxOfSig postfix) and set of all signals fo visualisation (_allSig postfix). 

 LTSpice returns time in seconds, voltage in Volts. During LTSpice proceeding, we add 50 ns to all times due to simulation details, so we subtract 50 ns in signals processing. We plot histograms starting from -50 ns.
 
This code also estimates left content (noise) percentage and writes it into terminal. Used for later cutting off signals with huge noises (more than 15% usually). 

2. total_charge_2d_hist.cpp
Draws 2D charge-amplitude plot. Requires ROOT file after garfield++ simulation and set of signals after LTSpice. Also returns two projections also known as charge and amplitude. Part for trimmed mean evaluation is included.
3. draw_before_spice.cpp
Draw three signals before LTSpice processing as reference. Also may be used for drawing singals after LTSpice
4. PlotSignal.cpp
Upgraded version of previous script. Integrates signal from every single file and plots distribution.  
5. Geant4GarfieldCompare.cpp
Was used to draw a set of Garfield++ and Geant4 histograms of energy loss distribution
6. GetAll90Percents.cpp
Gets 50% percentiles from Garfield++ dE histograms
7. Energy_dep.cpp
This script was used for comparison of energy loss, number of created electrons and number of total electrons after an avalanche

LTSpice simulation
1. Needed two files -- prepare_sigs.py and batch_new.py
2. prepare_sigs.py prepares Garfield++ signals for LTSpice processing, adds 50 ns to all time values
3. batch_new.py does generation of prepared signals.
4. template_asc_filename = "PNPI_sim_25ns_G3.asc" (line #17 in batch_new.py) is responsible for model name 
