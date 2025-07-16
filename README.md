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

