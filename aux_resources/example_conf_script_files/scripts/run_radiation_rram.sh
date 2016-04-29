#!/bin/bash
PROGRAM_VERSION=$RADIATION_SIMULATOR_VERSION

# 1R set
CONF_DIR=/simulation_data/circuit_reliability_framework/radiation_simulator/experiments/tcas_1t1r_RRAM/conf_files
CADENCE_CONF=$CONF_DIR/xml/cadence_conf.xml
TECHNOLOGY_CONF=$CONF_DIR/xml/technology_arizona_rram_65nm_bulk.xml
EXPERIMENT_CONF=$CONF_DIR/xml/1t1r_RRAM_radiation.xml
CIRCUIT_NETLIST=$CONF_DIR/netlists/scheme.scs
TITLE=rram_test_top_tt_$PROGRAM_VERSION
PERMISSIVE_MODE=permissive
PROGRAM_BIN=/simulation_data/circuit_reliability_framework/radiation_simulator/bin/radiation_simulator_$PROGRAM_VERSION

# $PROGRAM_BIN $CADENCE_CONF $TECHNOLOGY_CONF $EXPERIMENT_CONF $CIRCUIT_NETLIST $TITLE $PERMISSIVE_MODE;

# critical_nd_...
# echo -en "c\n1\na\na\n" | $PROGRAM_BIN $CADENCE_CONF $TECHNOLOGY_CONF $EXPERIMENT_CONF $CIRCUIT_NETLIST $TITLE $PERMISSIVE_MODE;

# montecarlo_critical_nd...
echo -en "d\n1\na\na\n" | $PROGRAM_BIN $CADENCE_CONF $TECHNOLOGY_CONF $EXPERIMENT_CONF $CIRCUIT_NETLIST $TITLE $PERMISSIVE_MODE;
