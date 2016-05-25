##!/bin/bash

echo "########################"
echo "Updating library"
echo "########################"
cd /simulation_data/circuit_reliability_framework/installers_last_version/lib_tmp/;
pwd
rm -rf src bin libcircuit_reliability.so_release_2.9.2_beta.tar.gz;
cp ../libcircuit_reliability.so_release_2.9.2_beta.tar.gz .;
tar xvf libcircuit_reliability.so_release_2.9.2_beta.tar.gz;
echo "Compiling lib files"
cd src/; make clean; make;
echo "Instaling lib files"
cd ..;
chmod a+x library_installation.sh;
sudo ./library_installation.sh;

echo "########################"
echo "Updating radiation simulator"
echo "########################"
cd /simulation_data/circuit_reliability_framework/installers_last_version/radiation_tmp/;
pwd
rm -rf src bin radiation_simulator_release_2.9.2_beta.tar.gz;
cp ../radiation_simulator_release_2.9.2_beta.tar.gz .;
tar xvf radiation_simulator_release_2.9.2_beta.tar.gz;
echo "Compiling radiation simulator"
cd src/; make clean; make;
cp ../bin/radiation_simulator_2.9.2_beta /simulation_data/circuit_reliability_framework/radiation_simulator/bin/;

echo "########################"
echo "Updating variability simulator"
echo "########################"
cd /simulation_data/circuit_reliability_framework/installers_last_version/variability_tmp/;
pwd
rm -rf src bin variability_simulator_release_2.9.2_beta.tar.gz;
cp ../variability_simulator_release_2.9.2_beta.tar.gz .;
tar xvf variability_simulator_release_2.9.2_beta.tar.gz;
echo "Compiling variability simulator"
cd src/; make clean; make;
cp ../bin/variability_simulator_2.9.2_beta /simulation_data/circuit_reliability_framework/variability_simulator/bin/;
