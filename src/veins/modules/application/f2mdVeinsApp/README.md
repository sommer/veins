# This a fork of Veins that integrates the *F2MDVeinsApp*

Other F2MD Modules of could be found in the [F2MD Repository](https://github.com/josephkamel/F2MD)

*Framework For Misbehavior Detection (F2MD)* aims to recreate within a simulation all the elements that form the chain of misbehavior detection.

## F2MD Features
* Basic Plausibility Checks on Received Beacons (mdChecks)
* Node Level Plausibility Investigation (mdApplications)
* Real Time Machine Learning for Plausibility Investigation (HTTP to a Python Server: PyMLServer-Local)
* Real Time Detection Status Output (mdStats)
* Support for Multiple Reporting Mechanisms (mdReport)
* Support for Global Reports Collection and Investigation (HTTP to a Python Server: MAServer-Global)
* Some Basic Psudonym Change Policies (mdPCPolicies)
* Some Local and Global Misbehavior Attacks Implementation (mdAttacks)

## Installation
1. Install Sumo 1.0.1
2. Install OMNeT++ 5.4.1
3. ~~Setup the latest version of Veins~~
4. ~~Add f2mdVeinsApp to >src/veins/modules/application~~
3. Setup ***this version*** of Veins (Follow the [Veins Tutorial](https://veins.car2x.org/tutorial/))

## F2MD Project Diagram

 ![alt text](https://github.com/josephkamel/F2MD/blob/master/F2MD-Diagram.jpg)