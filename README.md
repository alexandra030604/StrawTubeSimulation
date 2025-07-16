# StrawTubeSimulation
Straw tube signal simulation based on Garfield++ framework

Garfield++ User Guide:
https://garfieldpp.web.cern.ch/doxygen/UserGuide.pdf

Main generation:
1. generate.C for gas file
2. signal.C for signal generation

Post-processing
1. get_shaper_signal_params.cpp
2. total_charge_2d_hist.cpp
3. PlotSignal.cpp
4. draw_before_spice.cpp
5. Geant4GarfieldCompare.cpp
6. GetAll90Percents.cpp
7. Energy_dep.cpp
# Main generation:
# 1. generate.C for gas file
This code generates a file '.gas` with transport parameters **of charged particles** for the Ar/CO₂ gas mixture (70%/30%) in electromagnetic fields.
## 1. Gas Mixture Setup
```cpp
const double pressure = AtmosphericPressure;  // 1 atm (760 Torr)
const double temperature = 293.15;           // 20°C (in Kelvin)
MediumMagboltz gas("Ar", 70., "CO2", 30.);   // Ar(70%) + CO₂(30%)
```
- Thermal motion of molecules:
```cpp
// gas.EnableThermalMotion(true);
```
Turn it on if you are simulating a gas at high temperatures.
## 2. Setting up the electric field
```cpp
const size_t nE = 20;         // Number of electric field sampling points
const double emin = 100.;     // Minimum field strength [V/cm]
const double emax = 100000.;   // Maximum field strength [V/cm] 
constexpr bool useLog = true;  // Enable logarithmic field scaling
```
## 3. Magnetic Field Configuration (Optional)
If Magnetic field requirement
```cpp
const size_t nB = 1;          // Number of B-field sampling points
const double bmin = 2.;       // Minimum field strength [Tesla]
const double bmax = 2.;       // Maximum field strength [Tesla] 
const size_t nA = 1;          // Number of angle sampling points
const double amin = HalfPi;   // 90° angle [rad]
const double amax = HalfPi;
```
- Usage Options:
Case 1: No magnetic field (simplified configuration)

```cpp
gas.SetFieldGrid(emin, emax, nE, useLog);
```
Case 2: With magnetic field (full E+B configuration)

```cpp
gas.SetFieldGrid(emin, emax, nE, useLog, bmin, bmax, nB, amin, amax, nA);
```
## 4. Simulation Parameters

```cpp
const int ncoll = 10;  // Number of collision samples
gas.GenerateGasTable(ncoll);
```
## 5. Results Output
```cpp
gas.WriteGasFile("Ar_70_co2_30_1atm_T20_2T.gas");
```
## 6. Visualization (Optional)
```cpp
// app.Run(kTRUE);  // Uncomment for ROOT interactive visualization
```
Usage:
Enables real-time inspection of simulation results through ROOT's graphical interface when uncommented.

# 2. signal.C for signal generation
This code models the operation of a gas detector (such as a drift tube) using Garfield++ and ROOT libraries.

Core Functions:
- Simulates muon passage through an Ar/CO₂ (70/30) gas mixture
- Calculates gas ionization processes by charged particles
- Models electron drift and avalanche multiplication in electric fields
- Records resulting electrode signals
  ## 1. Gas Medium Initialization
  ```cpp
  MediumMagboltz gas;
  gas.LoadGasFile("Ar_70_co2_30_1atm_T20_0T.gas"); // Precomputed gas properties
  // Penning effect configuration
  const double rPenning = 0.54; // Ionization probability
  const double lambdaPenning = 0.; // Mean free path
  gas.EnablePenningTransfer(rPenning, lambdaPenning, "ar");

Physical Parameters:
- Ar/CO₂ (70/30) mixture at 1 atm, 20°C
- Penning effect enhances ionization efficiency
- Gas parameters pre-generated via generate.C

Penning transfer Argon includes a number of excitation levels with an excitation energy exceeding the ionisation potential of CO2 (13.78 eV). These excited states can contribute to the gain, since (part of) the excitation energy can be transferred to a CO2 molecule through collisions or by photo-ionisation.
In the simulation, this so-called Penning effect can be described in terms of a probability 𝑟 that an excitation is converted to an ionising collision.
## 2. Detector Geometry Setup
```cpp
ComponentAnalyticField cmp;
cmp.SetMedium(&gas); // Gas medium assignment

// Anode wire configuration
const double rWire = 15.e-4; // 15 μm radius
cmp.AddWire(0., 0, 2*rWire, vWire, "s"); // Central anode at 1750V

// Cathode tube configuration
const double rTube = 0.5; // 0.5 cm radius
cmp.AddTube(rTube, vTube, 0, "t"); // Grounded cathode

// Magnetic field (disabled here)
cmp.SetMagneticField(0, 0, 0);
```
Design Features:
- Cylindrical chamber with central wire
- Analytic field solution (no FEM)
- Optional B-field support
  ## 3. Sensor Configuration
```cpp
Sensor sensor;
sensor.AddComponent(&cmp); // Register field component
sensor.AddElectrode(&cmp, "s"); // Readout electrode

// Time window settings
const unsigned int nbins = 2000;
sensor.SetTimeWindow(0, 0.125, nbins); // 250ns window, 0.125ns/bin
```
Key Aspects:

- Multi-component support
- Automatic weighting field calculation
- High time resolution (125ps/bin)

## 4. Particle Tracking Setup
```cpp
TrackHeed track;
track.SetParticle("muon"); // Particle type
track.SetEnergy(1.e9);     // 1 GeV energy
track.SetSensor(&sensor);  // Sensor binding

// Track generation parameters
double rTrack = 0.2; // 2 mm radial offset
double x0 = rTrack;
double y0 = -sqrt(rTube*rTube - rTrack*rTrack) + 0.001;
```

Let us consider a track that passes at a distance of 2 mm from the wire centre. After simulating
the passage of the charged particle, we loop over the “clusters” (i.e. the ionizing collisions of
the primary particle) along the track and calculate a drift line for each electron produced in the
cluster

The program Heed is an implementation of the photo-absorption ionization (PAI) model. It
was written by I. Smirnov. An interface to Heed is available through the class TrackHeed.
The Cluster objects returned by TrackHeed::GetClusters contain the position and time of
the ionizing collision (member variables x, y, z, t), the transferred energy (member variable
energy), and a vector of Electron objects corresponding to the conduction electrons associated
to the cluster.
We iterate over the clusters along a track and the conduction electrons
in each cluster.

## 5. Drift Process Configuration
```cpp
DriftLineRKF drift;
drift.SetSensor(&sensor);
drift.SetGainFluctuationsPolya(1, 57000, true); // Polya distribution (actually, not 57k, but 48k. why this happens is a mystery)
drift.EnableIonTail(); // Ion drift simulation
```
Gas gain refers to the charge multiplication factor in gaseous detectors, achieved through electron avalanche processes in strong electric fields.
Critical Parameters:

- 5th-order Runge-Kutta integration
- Polya parameters (θ=1, mean gain=48k)
- Ion tail contribution included

## 6. Event Processing Loop
``` cpp
for (unsigned int j = 0; j < 2000; ++j) {
    track.NewTrack(x0, y0, 0, 0, 0, 1., 0);
    while (track.GetCluster(xc, yc, zc, tc, nc, ec, extra)) {
        // Cluster processing
        for (int k = 0; k < nc; ++k) {
            drift.DriftElectron(xe, ye, ze, te);
            // ... drift analysis ...
        }
    }
    
    // Event data storage
    tree->Fill(); 
}
```
Implementation Notes:

 - 2000 independent events
 - Cluster-by-cluster electron drift
 - Energy thresholding possible

## 7. Visualization System
```cpp
ViewDrift driftView;
ViewSignal signalView;

if (plotDrift) {
    // Track display setup
    driftView.SetCanvas(new TCanvas());
    track.EnablePlotting(&driftView);
}
```

As a check whether the simulation is doing something sensible, it can be useful to visualize the
drift lines. Before simulating the charged particle track and the electron drift lines, we have to
instruct TrackHeed and DriftLineRKF to pass the coordinates of the clusters and the points
along the drift line to a ViewDrift object which then takes care of plotting them.

## 8. Data Output System
```cpp
TFile f("results.root", "RECREATE");
tree->Write(); // ROOT tree storage

// Text output for signals
std::ofstream out("signal.txt");
out << t << " " << amplitude << endl;
```
Output Formats:
- ROOT files for comprehensive analysis
- CSV export capability
