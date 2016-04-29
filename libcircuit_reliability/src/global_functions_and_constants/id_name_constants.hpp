#ifndef ID_NAME_CONSTANTS_H
#define ID_NAME_CONSTANTS_H

#include <string>

// Id's / names
static const std::string kAlterationSubcircuit = "radiation_subcircuit";
static const std::string kRadiatedCircuit = "altered_circuit";
static const std::string kRadiatedSufix = "_altered";
static const std::string kMainCircuitStringId ="main_circuit";
static const std::string kAlterationSubcircuitAHDLId = "radiation_subcircuit_AHDL";
static const std::string kAlterationSourcesCircuitStringId ="radiation_sources_circuit";
static const std::string kParametersCircuitStringId ="parameters_circuit";
static const std::string kRadiatedCircuitStringId ="altered_circuit";
static const std::string kAlteredStatementsCircuitStringId ="altered_statements_circuit";
static const std::string kIncludedPrefixId = "included_circuit_";
static const std::string kInstancePrefix = "instance_";
static const std::string kCircutFileSufix = "_netlist.scs";
static const std::string kAHDLNodePrefix = "AHDL_node_";
static const std::string kAHDLNodeId = "AHDL_injected_node";
static const std::string kRauxSourceId = "rAux_altered";
static const std::string kRauxPrefix = "kRauxPrefix";
static const std::string kResistor = "resistor";
static const std::string kRauxParam = "r";
static const std::string kRauxValue = "1";
static const std::string kSaveInjectionSufix = "_monitor";
static const std::string kSaveInjectionMasterName = "save";

#endif /* ID_NAME_CONSTANTS_H */