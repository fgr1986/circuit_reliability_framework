/*
 * conditional_statement.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#ifndef CONDITIONAL_STATEMENT_H
#define CONDITIONAL_STATEMENT_H

#include <vector>
#include <string>
#include <istream>
#include <fstream>

#include "statement.hpp"

class ConditionalStatement : public Statement{
public:
	ConditionalStatement();
	ConditionalStatement(Statement* belonging_circuit, LogIO* log_io, Scope* belonging_scope );
	ConditionalStatement(const ConditionalStatement& orig);
	// void CopyStatement(const Statement& orig) ;
	int get_conditional_statement_type() const {return conditional_statement_type; }
	void set_conditional_statement_type(int conditional_statement_type) { this->conditional_statement_type = conditional_statement_type; }
	std::string get_condition() const {return condition; }
	void set_condition(std::string condition) { this->condition = condition; }
	virtual ~ConditionalStatement();
	virtual std::string ExportCircuitStatement( const std::string& indentation ) override;

	virtual ConditionalStatement* GetCopy() override;
	bool ParseConditionalStatement(
		Statement& global_scope_parent, std::ifstream* file, std::vector<std::string>& lineTockens,
		std::string & statementCode, Statement& parent, std::string& currentReadLine,
		int conditionalStatementType, int& statementCount, bool& endOfFile,
		bool& parsingSpectreCode, const bool permissiveParsingMode  );

private:
	int conditional_statement_type;
	std::string condition;

};

#endif /* CONDITIONAL_STATEMENT_H */
