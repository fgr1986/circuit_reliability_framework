##!/bin/bash

############
# Version  #
############
echo "";
echo "Compile and Install Framework. Version $RELIABILITY_FRAMEWORK_VERSION";
TAR_EXTENSION=$RELIABILITY_FRAMEWORK_VERSION.tar.gz;

echo "TAR_EXTENSION=$TAR_EXTENSION"

echo "########################"
echo "Updating library"
echo "########################"
cd /simulation_data/circuit_reliability_framework/installers_last_version/
if [ ! -d lib_tmp ]; then
  mkdir lib_tmp
fi
cd lib_tmp/;
pwd
rm -rf src bin libcircuit_reliability.so_release_$TAR_EXTENSION;
cp ../libcircuit_reliability.so_release_$TAR_EXTENSION .;
tar xvf libcircuit_reliability.so_release_$TAR_EXTENSION;
echo "Compiling lib files"
cd src/; make clean; make; make clean;

echo "########################"
echo "Installing lib files Version $RELIABILITY_FRAMEWORK_VERSION";
echo "########################"
cd /simulation_data/circuit_reliability_framework/installers_last_version/installation_scripts/;
chmod a+x library_installation.sh;
./library_installation.sh;

echo "########################"
echo "Updating radiation simulator"
echo "########################"
cd /simulation_data/circuit_reliability_framework/installers_last_version/;
if [ ! -d radiation_tmp ]; then
  mkdir radiation_tmp
fi
cd radiation_tmp/;
pwd
rm -rf src bin radiation_simulator_release_$TAR_EXTENSION;
cp ../radiation_simulator_release_$TAR_EXTENSION .;
tar xvf radiation_simulator_release_$TAR_EXTENSION;
echo "Compiling radiation simulator"
cd src/; make clean; make;
if [ ! -d /simulation_data/circuit_reliability_framework/radiation_simulator/bin/ ]; then
  mkdir /simulation_data/circuit_reliability_framework/radiation_simulator/bin/
fi
cp ../bin/radiation_simulator_$RELIABILITY_FRAMEWORK_VERSION /simulation_data/circuit_reliability_framework/radiation_simulator/bin/;

echo "########################"
echo "Updating variability simulator"
echo "########################"
cd /simulation_data/circuit_reliability_framework/installers_last_version/;
if [ ! -d variability_tmp ]; then
  mkdir variability_tmp
fi
cd variability_tmp/;
pwd
rm -rf src bin variability_simulator_release_$TAR_EXTENSION;
cp ../variability_simulator_release_$TAR_EXTENSION .;
tar xvf variability_simulator_release_$TAR_EXTENSION;
echo "Compiling variability simulator"
cd src/; make clean; make;
if [ ! -d /simulation_data/circuit_reliability_framework/radiation_simulator/bin/ ]; then
  mkdir /simulation_data/circuit_reliability_framework/radiation_simulator/bin/
fi
cp ../bin/variability_simulator_$RELIABILITY_FRAMEWORK_VERSION /simulation_data/circuit_reliability_framework/variability_simulator/bin/;
