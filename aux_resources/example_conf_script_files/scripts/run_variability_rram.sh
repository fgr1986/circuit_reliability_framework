#!/bin/bash
PROGRAM_VERSION=$VARIABILITY_SIMULATOR_VERSION

# 1R set
CONF_DIR=/simulation_data/circuit_reliability_framework/variability_simulator/experiments/tcas_1t1r_RRAM/conf_files
CADENCE_CONF=$CONF_DIR/xml/cadence_conf.xml
TECHNOLOGY_CONF=$CONF_DIR/xml/technology_arizona_rram_65nm_bulk.xml
EXPERIMENT_CONF=$CONF_DIR/xml/1t1r_RRAM_variability.xml
CIRCUIT_NETLIST=$CONF_DIR/netlists/scheme.scs
TITLE=rram_test_top_tt_$PROGRAM_VERSION
PERMISSIVE_MODE=permissive
PROGRAM_BIN=/simulation_data/circuit_reliability_framework/variability_simulator/bin/variability_simulator_$PROGRAM_VERSION

# $PROGRAM_BIN $CADENCE_CONF $TECHNOLOGY_CONF $EXPERIMENT_CONF $CIRCUIT_NETLIST $TITLE $PERMISSIVE_MODE;
echo -en "c\n1\na\na\n" | $PROGRAM_BIN $CADENCE_CONF $TECHNOLOGY_CONF $EXPERIMENT_CONF $CIRCUIT_NETLIST $TITLE $PERMISSIVE_MODE;
