#include <iostream>

#include <TCanvas.h>
#include <TROOT.h>
#include <TApplication.h>

#include "Garfield/MediumMagboltz.hh"
#include "Garfield/FundamentalConstants.hh"

using namespace Garfield;

int main(int argc, char * argv[]) {

  TApplication app("app", &argc, argv);

  const double pressure = AtmosphericPressure;
  const double temperature = 293.15; //20°C 

  // Setup the gas.
  MediumMagboltz gas("Ar", 70., "CO2", 30.);
  gas.SetTemperature(temperature);
  gas.SetPressure(pressure);
  //gas.EnableThermalMotion(true); // Enable thermal motion of gas molecules (for high-temperature simulations). Default: off (thermal motion negligible at room temperature)

  const size_t nE = 20; // number of Electric field values(points) 
  // Range of Electric field [V/cm]
  const double emin =    100.;
  const double emax = 100000.;
  // Flag to request logarithmic spacing.
  constexpr bool useLog = true; 
  
  // *** If Magnetic field requirement 
     //NOTE! The data below is the magnetic field directed along the axis of the tube wire.
  
  // Range of magnetic fields [Tesla]
  const size_t nB = 1; // number of Magnetic field values(points) 
  const double bmin = 2.;
  const double bmax = 2.;
  const size_t nA = 1; //number of Angles values(points)angles
  // Range of angles [rad]
  const double amin = HalfPi;
  const double amax = HalfPi;
  //*** 
   
  //gas.SetFieldGrid(emin, emax, nE, useLog); // Option 1: Electric field only (simpler/faster). Use when no magnetic field is present
  gas.SetFieldGrid(emin, emax, nE, useLog, bmin, bmax, nB, amin, amax, nA); // Option 2: Full E + B field at angle(complete). Use when B ≠ 0

  const int ncoll = 10;
  // Run Magboltz to generate the gas table.
  gas.GenerateGasTable(ncoll);
  // Save the table.
  gas.WriteGasFile("Ar_70_co2_30_1atm_T20_2T.gas");

   //app.Run(kTRUE);

}
