
echo "########################"
echo "Updating radiation simulator"
echo "########################"
cd /simulation_data/circuit_reliability_framework/installers_last_version/radiation_tmp/;
pwd
rm -rf src bin radiation_simulator_release_3.2.0.tar.gz;
cp ../radiation_simulator_release_3.2.0.tar.gz .;
tar xvf radiation_simulator_release_3.2.0.tar.gz;
echo "Compiling radiation simulator"
cd src/; make clean; make;
cp ../bin/radiation_simulator_3.2.0 /simulation_data/circuit_reliability_framework/radiation_simulator/bin/;
