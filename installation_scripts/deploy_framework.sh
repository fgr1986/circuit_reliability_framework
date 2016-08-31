#!/bin/bash

############
# Version  #
############
echo "Version $RELIABILITY_FRAMEWORK_VERSION";
TAR_EXTENSION=$RELIABILITY_FRAMEWORK_VERSION.tar.gz;

RELIABILITY_FW_PATH=/home/fgarcia/ownCloud/workspace/c++/circuit_reliability_framework;

echo "************************";
echo "installation_scripts";
echo "************************";
cd $RELIABILITY_FW_PATH/installation_scripts;
scp *.sh 10.2.220.9:/simulation_data/circuit_reliability_framework/installers_last_version/;

echo "************************";
echo "libcircuit_reliability";
echo "************************";
cd $RELIABILITY_FW_PATH/libcircuit_reliability/src;
make clean; make dist; scp ../dist/libcircuit_reliability.so_release_$TAR_EXTENSION 10.2.220.9:/simulation_data/circuit_reliability_framework/installers_last_version/;

echo "************************";
echo "variability_simulator";
echo "************************";
cd $RELIABILITY_FW_PATH/variability_simulator/src;
make clean; make dist; scp ../dist/variability_simulator_release_$TAR_EXTENSION 10.2.220.9:/simulation_data/circuit_reliability_framework/installers_last_version/;

echo "************************";
echo "radiation_simulator";
echo "************************";
cd $RELIABILITY_FW_PATH/radiation_simulator/src;
make clean; make dist; scp ../dist/radiation_simulator_release_$TAR_EXTENSION 10.2.220.9:/simulation_data/circuit_reliability_framework/installers_last_version/;
