#ifndef INCLUDE_STATEMENT_CONSTANTS_H
#define INCLUDE_STATEMENT_CONSTANTS_H

// Include Statement types
static const int kIncludeStatementType = 0;
static const int kIncludeAnalogIfStatementType = 1;
static const int kIncludeVCDStatementType = 2;
static const int kIncludeEVCDStatementType = 3;
static const int kIncludeVectorStatementType = 4;

static const int kIncludeTechnologyModelStatementType = 5;

static const std::string kIncludeStatementWord = "include";
static const std::string kIncludeAnalogIfStatementWord = "ahdl_include";
static const std::string kIncludeVCDStatementWord = "vcd_include";
static const std::string kIncludeEVCDStatementWord = "evcd_include";
static const std::string kIncludeVectorStatementWord = "vec_include";
static const std::string kIncludeTechnologyModelStatementWord = "include";
static const std::string kIncludeWords[] = { kIncludeStatementWord, kIncludeAnalogIfStatementWord, 
	kIncludeVCDStatementWord, kIncludeEVCDStatementWord, kIncludeVectorStatementWord, kIncludeTechnologyModelStatementWord };
	static const int kIncludeWordsSize = sizeof(kIncludeWords) / sizeof(std::string);

#endif /* INCLUDE_STATEMENT_CONSTANTS_H */