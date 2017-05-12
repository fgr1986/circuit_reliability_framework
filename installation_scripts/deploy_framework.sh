#!/bin/bash

############
# Version  #
############
echo "";
echo "Deploy Framework. Version $RELIABILITY_FRAMEWORK_VERSION";
TAR_EXTENSION=$RELIABILITY_FRAMEWORK_VERSION.tar.gz;

# to be changed
DESTINATION_MACHINE=IP
RELIABILITY_FW_PATH=/home/fgarcia/ownCloud/workspace/c++/circuit_reliability_framework;
BASH_ENVIRONMENT_CONF_PATH=/home/fgarcia/ownCloud/workspace/bash_environment_conf;

echo "************************";
echo "bash environment configuration";
echo "************************";
cd $BASH_ENVIRONMENT_CONF_PATH;
scp bash_fgarcia_workspace $DESTINATION_MACHINE:/home/fgarcia/;

echo "************************";
echo "installation_scripts";
echo "************************";
cd $RELIABILITY_FW_PATH;
scp -r installation_scripts/ $DESTINATION_MACHINE:/simulation_data/circuit_reliability_framework/installers_last_version/installation_scripts/;

echo "************************";
echo "libcircuit_reliability";
echo "************************";
cd $RELIABILITY_FW_PATH/libcircuit_reliability/src;
make clean; make dist; scp ../dist/libcircuit_reliability.so_release_$TAR_EXTENSION $DESTINATION_MACHINE:/simulation_data/circuit_reliability_framework/installers_last_version/;

echo "************************";
echo "variability_simulator";
echo "************************";
cd $RELIABILITY_FW_PATH/variability_simulator/src;
make clean; make dist; scp ../dist/variability_simulator_release_$TAR_EXTENSION $DESTINATION_MACHINE:/simulation_data/circuit_reliability_framework/installers_last_version/;

echo "************************";
echo "radiation_simulator";
echo "************************";
cd $RELIABILITY_FW_PATH/radiation_simulator/src;
make clean; make dist; scp ../dist/radiation_simulator_release_$TAR_EXTENSION $DESTINATION_MACHINE:/simulation_data/circuit_reliability_framework/installers_last_version/;
