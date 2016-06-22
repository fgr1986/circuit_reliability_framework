#ifndef NETLIST_WORD_CONSTANTS_H
#define NETLIST_WORD_CONSTANTS_H

// Parsing strings
// Parameters, equals, and comment word1 in globa_constants
static const std::string kContinuesNextLineChar = "\\";
//we first trim the statement, so it is not needed
//static const std::string kContinuesNextLineChar2 = "\\ ";
static const std::string kContinuationPreviousLineChar = "+";
static const std::string kNextLevelInInstanceChar = ".";
static const std::string kParenthesisStartWord = "(";
static const std::string kParenthesisEndWord = ")";
static const std::string kTerminalChar = ":";
static const std::string kCommentWord2 = "*";

static const std::string kArgumentDelimiter = ",";
static const std::string kDelimiterEqualsWord = " =";
static const std::string kEqualsDelimiterWord = "= ";

static const std::string kSimulatorWord = "simulator";
static const std::string kLangWord = "lang";
static const std::string kSpectreWord = "spectre";
static const std::string kCircuitWord = "circuit";
static const std::string kBSourceStatementWord =  "bsource";
static const std::string kArgumentWord = "argument";
static const std::string kGlobal = "global";
static const std::string kInLineSubcircuitWord = "inline";
static const std::string kModelWord = "model";
static const std::string kAnalogModelWord = "analogmodel";
static const std::string kAnalogModelNameWord = "modelname";
static const std::string kAnalogModelSplitWord = " analogmodel modelname=";
static const std::string kLibraryWord = "library";
static const std::string kEndLibraryWord = "endlibrary";
static const std::string kSectionWord = "section";
static const std::string kEndSectionWord = "endsection";
static const std::string kSubcircuitWord = "subckt";
static const std::string kEndSubcircuitWord = "ends";
static const std::string kEndSubcircuitAndDelimiterWord = "ends ";
static const std::string kParamTestAndDelimiterWord = " paramtest ";
static const std::string kBracketsStartWord = "{";
static const std::string kBracketsEndWord = "}";
static const std::string kUserDefinedFunctionWord = "real";
static const std::string kUserDefinedFunctionAndDelimiterWord = "real ";
static const std::string kParamTestWord = "paramtest";
static const std::string kMontecarloNumRuns = "numruns";

#endif /* NETLIST_WORD_CONSTANTS_H */
