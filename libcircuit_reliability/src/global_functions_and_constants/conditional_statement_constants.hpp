#ifndef CONDITIONAL_STATEMENT_CONSTANTS_H
#define CONDITIONAL_STATEMENT_CONSTANTS_H

// Conditional Statement types
static const int kIfStatementType = 0;
static const int kElseIfStatementType = 1;
static const int kElseStatementType = 2;
static const std::string kIfStatementWord1 = "if";
static const std::string kIfStatementWord2 = "if(";
static const std::string kElseIfStatementWord1 = "else if";
static const std::string kElseIfStatementWord2 = "else if(";
static const std::string kElseStatementWord1 = "else";
static const std::string kElseStatementWord2 = "else{";
static const std::string kConditionalWords[] = { kIfStatementWord1, kIfStatementWord2, 
	kElseIfStatementWord1, kElseIfStatementWord2, kElseStatementWord1, kElseStatementWord2 };
static const int kConditionalWordsSize = sizeof(kConditionalWords) / sizeof(std::string);

#endif /* CONDITIONAL_STATEMENT_CONSTANTS_H */