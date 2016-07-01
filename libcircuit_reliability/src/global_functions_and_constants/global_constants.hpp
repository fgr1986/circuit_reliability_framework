#ifndef GLOBAL_CONSTANTS_H
#define GLOBAL_CONSTANTS_H

#include <string>

// #define GCC_OLD ; // versions 4.x

// Logger configuration:
// Parsing logs
#define PARSING_VERBOSE ;
#define PARSING_VERBOSE_MIN ;
// Regex 2 log
// #define VERBOSE_REGEX ;
// Radiation logs
// #define RADIATION_VERBOSE ;
// Dependency logs
// #define DEPENDENCY_VERBOSE ;
// Netlist Exporting logs
// #define NETLIST_EXPORT_VERBOSE ;
// XML exporting logs
// #define XML_EXPORT_VERBOSE ;
// PSFASCII processing log
// #define PSFASCII_VERBOSE ;
// Spectre handler log
// #define SPECTRE_SIMULATIONS_VERBOSE ;
// Result analysis verbose
// #define RESULTS_ANALYSIS_VERBOSE ; // for each transient etc
// #define RESULTS_POST_PROCESSING_VERBOSE ; // for montecarlo etc
// Result destructors verbose
// #define DESTRUCTORS_VERBOSE ;

static const double kSpectreMaxAllowedSimDiffCoeff = 1e-12;

static const std::string kSpectreHandlerLibTitle = "Spectre Handler Library";
static const std::string kSpectreHandlerLibVersion = "v3.1.0";
static const std::string kAuthor = "Fernando García, fgarcia@die.upm.es";
static const std::string kSpectreHandlerLibVersionDate = "23/06/2016";

// Remove windows special chars
static const std::string kWindowsEOL = "\r";
// static const char kWindowsEOL = '^M';
// static const std::string kWindowsChars[] = { kWindowsEOL };

const int kNotDefinedInt = -666;
const std::string kNotDefinedString = "Not defined";

// Reliability results
static const unsigned int kScenarioNotAnalyzed = 0;
static const unsigned int kScenarioNotSensitive = 1;
static const unsigned int kScenarioSensitive = 2;
static const std::string kScenarioResultString[] = {"Not analyzed", "Not sensitive to maximum critical parameter value", "Sensitive to critical parameter."};

// Simulation Modes
static const unsigned int kStandardMode = 0;
static const unsigned int kCriticalValueMode = 1;
static const unsigned int kCriticalParameterNDParametersSweepMode = 2;
static const unsigned int kMontecarloNDParametersSweepMode = 3;
static const unsigned int kMontecarloCriticalParameterNDParametersSweepMode = 4;
// Simulation modes keyWords
static std::string kStandardModeWord = "standard_analysis";
static std::string kCriticalValueModeWord = "critical_parameter_value_analysis";
static std::string kCriticalParameterNDParametersSweepModeWord = "critical_parameter_n_d_parameters_sweep_analysis";
static std::string kMontecarloNDParametersSweepModeWord = "montecarlo_n_d_parameters_sweep_analysis";
static std::string kMontecarloCriticalParameterNDParametersSweepModeWord = "montecarlo_critical_parameter_n_d_parameters_sweep_analysis";
// Montecarlo firstrun should be 1
static const unsigned int kMontecarloCountStart = 1;
static const std::string kMontecarloIterationsParameterWord = "montecarlo_iterations";

// Simulation parameter step/control
// <!--0. lineal increments in both sweep and critical value modes.-->
// <!--1. X(n) = 10^( (log10(b)-log10(a))/(N-1)*index ), a < b in both critical value and sweep modes. -->
// <!--2. X(n) = b - 10^( (log10(b)-log10(a))/(N-1)*index ), a < b in both critical value and sweep modes. -->

static const unsigned int kSPLineal = 0;
static const unsigned int kSPLogSmaller = 1;
static const unsigned int kSPLogBigger = 2;


// Netlist useful words
static const std::string kLanguageSentence = "simulator lang=spectre\n";
static const std::string kParametersWord = "parameters";
static const std::string kCommentWord1 = "//";
static const std::string kEqualsWord = "=";
static const std::string kDot = ".";
static const std::string kSCS = "scs";
static const std::string kXML = "xml";

// Const strings
static const std::string kDelimiter = " ";
static const std::string kDoubleDelimiter = "  ";
static const std::string kEmptyWord = "";
static const std::string kEmptyLine = "\n";
static const std::string kTab = "\t";
static const std::string k2Tab = "\t\t";
static const std::string k3Tab = "\t\t\t";
static const std::string k4Tab = "\t\t\t\t";
// static const std::string kTab = "    ";
// static const std::string k2Tab = "      ";
// static const std::string k3Tab = "        ";
// static const std::string k4Tab = "          ";
static const std::string kShortDelimiter = "-------------------------------------------";
static const std::string kLongDelimiter = kShortDelimiter + kShortDelimiter ;
static const std::string kQuotes = "\"";

// // Parameter const
// static const int kDouble = 0;
// static const int kString = 1;
// static const int kExpresion = 2;
// static const int kList = 3;
// Instance const
static const std::string kStatementMasterName = "STATEMENT_MASTER";
// 	statement ids and descriptors
static const int kCircuitStatement = -1;
// injected source id's (one per circuit)
static const int kInjectedSourceId = -2;
// injected language
static const int kInjectedLanguageId = -3;
// save injected current
static const int kInjectedSaveMonitorId = -4;

// Id's / names
static const std::string kAlterationSubcircuit = "alteration_subcircuit";
static const std::string kRadiatedCircuit = "altered_circuit";
static const std::string kRadiatedSufix = "_altered";
static const std::string kMainCircuitStringId ="main_circuit";
static const std::string kAlterationSubcircuitAHDLId = "alteration_subcircuit_AHDL";
static const std::string kAlterationSourcesCircuitStringId ="alteration_sources_circuit";
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

// integer ids
static const int kMainCircuitId = 0;
static const int kAlterationSourcesCircuitId = -1;
static const int kAlteredStatementsCircuitId = -2;
// Modified_sufix
static const std::string kModifiedFileSufix = "_rs";


#endif /* GLOBAL_CONSTANTS_H */
