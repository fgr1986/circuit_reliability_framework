#ifndef REGEX_GLOBAL_H
#define REGEX_GLOBAL_H
// Regular Expressions Patterns
// Parameters regular expression

static const std::string kAllowedEndOfExpresion = "(\"|\\b|\\)|\\})"; // added '"' for p="1"
static const std::string kSpecialCharsRegEx = "\\.\\{\\}\\(\\)\\\\\\*\\-\\+\\?\\|\\^\\$";

// standard
static const std::string kSuportedNamesCharsRegEx = "[\\wçÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/" + kSpecialCharsRegEx + "]+";
static const std::string kSuportedValuesCharsRegEx	 = "[\\s\"\\wçÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/"	+ kSpecialCharsRegEx + "(==)(>=)(<=)]+";
// debug
// static const std::string kSuportedNamesCharsRegEx = "[\\w]+";
// static const std::string kSuportedValuesCharsRegEx	 = "[\\s\\w"	+ kSpecialCharsRegEx + "]+";

static const std::string kSimpleListRegEx	 = "\\[" + kSuportedValuesCharsRegEx + "?" + kAllowedEndOfExpresion + "\\]";
static const std::string kDeepListRegEx	 = "\\[(" + kSuportedValuesCharsRegEx + "|(" + kSimpleListRegEx + "))+\\]";

// Regex main expression
static const std::string kParameterRegEx = "(?:^|\\s)"							// starts string or space before, not catched
	"("																																// group of the parameter or parameter-value
		"(\\b|\\\\)" + kSuportedNamesCharsRegEx + "\\b" 								// 		simple names
		"|" 																														// 		or
		"(\\b|\\\\)" + kSuportedNamesCharsRegEx + "\\b\\s*=\\s*\"*" + kSuportedValuesCharsRegEx + "?" + kAllowedEndOfExpresion			// 		name-value
		+ "|" 																														// 		or
		"(\\b|\\\\)" + kSuportedNamesCharsRegEx + "\\b\\s*=\\s*\"*" + kSimpleListRegEx 	// name-list-value
		+ "|" 																														// 		or
		"(\\b|\\\\)" + kSuportedNamesCharsRegEx + "\\b\\s*=\\s*\"*" + kDeepListRegEx		// name-deep-list-value
	+ ")"																															// end group
	"(?="																															// followed by group of
		"\\s*$"																													// end of string
		"|" 																														// or
		"\\s+(\\b|\\\\)" + kSuportedNamesCharsRegEx + "\\b\\s*=" 				// new parameter with value
		"|" 																														// or
		"\\s+(\\b|\\\\)" + kSuportedNamesCharsRegEx + "\\b"							// new parameter without value
	")";																															// end of following group

static const std::string kParameterRegExNotUnvalued = "(?:^|\\s)"																	// starts string or space before, not catched
	"("																																// group of the parameter or parameter-value
		"(\\b|\\\\)" + kSuportedNamesCharsRegEx + "\\b\\s*=\\s*" + kSuportedValuesCharsRegEx + "?" + kAllowedEndOfExpresion			// 		name-value
		+ "|" 																														// 		or
		"(\\b|\\\\)" + kSuportedNamesCharsRegEx + "\\b\\s*=\\s*" + kSimpleListRegEx 												// 		name-list-value
		+ "|" 																													// 		or
		"(\\b|\\\\)" + kSuportedNamesCharsRegEx + "\\b\\s*=\\s*" + kDeepListRegEx													// 		name-deep-list-value
	+ ")"																																// end group
	"(?="																															// followed by group of
		"\\s*$"																														// 		end of string
		"|" 																														// 		or
		"\\s+(\\b|\\\\)" + kSuportedNamesCharsRegEx + "\\b\\s*=" 																	//		new parameter with value
		"|" 																														// 		or
		"\\s+(\\b|\\\\)" + kSuportedNamesCharsRegEx + "\\b"																			// 		new parameter without value
	")";																															// end of following group

// {statement} regular expression
static const std::string kBracketedStatementRegEx =
	//"\\{" + kSuportedValuesCharsRegEx + "\\}";
	"\\{.+\\}";

#endif /* REGEX_GLOBAL_H */
