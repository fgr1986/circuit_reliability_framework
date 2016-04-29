/*
 * user_defined_function_statement.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#ifndef USER_DEFINED_FUNCTION_STATEMENT_H
#define USER_DEFINED_FUNCTION_STATEMENT_H

#include "statement.hpp"
#include "../scope.hpp"

class UserDefinedFunctionStatement : public Statement{
public:
	UserDefinedFunctionStatement();
	UserDefinedFunctionStatement(Statement* belonging_circuit, LogIO* log_io, Scope* belonging_scope);
	UserDefinedFunctionStatement(const UserDefinedFunctionStatement& orig);
	virtual ~UserDefinedFunctionStatement();
	virtual std::string ExportCircuitStatement(std::string indentation);
	virtual UserDefinedFunctionStatement* GetCopy();
	bool ParseUserDefinedFunctionStatement( Statement& global_scope_parent, std::ifstream* file,
		std::vector<std::string>& lineTockens, std::string & statementCode,
		std::string& currentReadLine, int& statementCount, bool& endOfFile, bool& parsingSpectreCode );
private:

	std::string kSpectreUserDefinedFunctionsGuides =
		"// Cadence user defined function guides.\n"
		"// When you define a function, follow these rules:\n"
		"// 1) Functions can be declared only at the top level and cannot be declared within subcircuits.\n"
		"// 2) Arguments to user-defined functions can only be real values, and the functions can only\n"
		"//	return real values. You must use the keyword real for data typing.\n"
		"// 3) The Spectre function syntax does not allow references to netlist parameters within the\n"
		"//	body of the function, unless the netlist parameter is passed in as a function argument.\n"
		"// 4) The function must contain a single return statement.\n";
};

#endif /* USER_DEFINED_FUNCTION_STATEMENT_H */
