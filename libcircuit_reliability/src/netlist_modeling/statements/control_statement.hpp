/*
 * control_statement.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#ifndef CONTROL_STATEMENT_H
#define CONTROL_STATEMENT_H

#include <vector>
#include <string>
#include <istream>
#include <fstream>

#include "statement.hpp"

class ControlStatement : public Statement{
public:
	ControlStatement();
	ControlStatement(Statement* belonging_circuit, LogIO* log_io,
		Scope* belonging_scope, bool special_syntax_control_statement);
	ControlStatement(const ControlStatement& orig);
	// void CopyStatement(const Statement& orig);
	virtual ~ControlStatement();
	bool get_advanced_control_statement() const {return advanced_control_statement; }
	void set_advanced_control_statement(bool advanced_control_statement) { this->advanced_control_statement = advanced_control_statement; }
	bool get_special_syntax_control_statement() const {return special_syntax_control_statement; }
	void set_special_syntax_control_statement(bool special_syntax_control_statement) { this->special_syntax_control_statement = special_syntax_control_statement; }

	virtual ControlStatement* GetCopy(); 
	virtual std::string ExportCircuitStatement(std::string indentation);

	bool ParseControlStatement( Statement& global_scope_parent, std::ifstream* file,
		std::vector<std::string>& lineTockens, std::string & statementCode,
		std::string& currentReadLine, int& statementCount, bool& endOfFile,
		bool& parsingSpectreCode, const bool permissiveParsingMode  );
	void set_param_set_case( bool param_set_case){this->param_set_case=param_set_case;}

private:
	bool advanced_control_statement;
	bool special_syntax_control_statement;
	bool param_set_case;
};

#endif /* CONTROL_STATEMENT_H */