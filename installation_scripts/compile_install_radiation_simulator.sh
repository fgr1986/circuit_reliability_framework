
TAR_EXTENSION=$RELIABILITY_FRAMEWORK_VERSION.tar.gz

echo "########################"
echo "Updating radiation simulator"
echo "########################"
cd /simulation_data/circuit_reliability_framework/installers_last_version/radiation_tmp/;
pwd
rm -rf src bin radiation_simulator_release_$TAR_EXTENSION;
cp ../radiation_simulator_release_$TAR_EXTENSION .;
tar xvf radiation_simulator_release_$TAR_EXTENSION;
echo "Compiling radiation simulator"
cd src/; make clean; make;
cp ../bin/radiation_simulator_3.2.2 /simulation_data/circuit_reliability_framework/radiation_simulator/bin/;
