#ifndef STATEMENTS_CONSTANTS_H
#define STATEMENTS_CONSTANTS_H

// statndard statements
static const int kInstanceStatement = 0;
static const int kModelStatement = 1;
static const int kAnalogModelStatement = 2;
static const int kSubcircuitStatement = 3;
static const int kAnalysisStatement = 6;
static const int kControlStatement = 7;
static const int kTransistorStatement = 8;
static const int kCommentStatement = 9;
static const int kUserDefinedFunctionStatement = 10;
static const int kSimpleStatement = 11;
static const int kConditionalStatement = 12;
static const int kIncludeStatement = 13;
static const int kLibraryStatement = 14;
static const int kSectionStatement = 15;
static const int kParamTestStatement = 16;
static const int kBSourceStatement = 17;
static const int kAlterationSourceSubcircuitStatement = 18;
static const int kGlobalStatement = 19;
static const std::string kCircuitStatementDesc = "circuit";
static const std::string kInstanceStatementDesc = "primitive";
static const std::string kModelStatementDesc = "model";
static const std::string kAnalogModelStatementDesc = "analog_model";
static const std::string kSubcircuitStatementDesc = "subcircuit";
static const std::string kAnalysisStatementDesc = "analysis";
static const std::string kControlStatementDesc = "control";
static const std::string kTransistorStatementDesc = "transistor";
static const std::string kCommentStatementDesc = "comment";
static const std::string kUserDefinedFunctionStatementDesc =  "user_defined_function";
static const std::string kSimpleStatementDesc =  "simple";
static const std::string kConditionalStatementDesc =  "conditional";
static const std::string kIncludeStatementDesc =  "include";
static const std::string kLibraryStatementDesc =  "library";
static const std::string kSectionStatementDesc =  "section";
static const std::string kParamTestStatementDesc =  "param_test";
static const std::string kBSourceStatementDesc =  "bsource";
static const std::string kAlterationSourceSubcircuitStatementDesc =  "radiation source subcircuit";
static const std::string kGlobalStatementDesc =  "global nodes";
// Default Paralell Statements
static const std::string kDefaultParallelStatements = "1";

#endif /* STATEMENTS_CONSTANTS_H */
