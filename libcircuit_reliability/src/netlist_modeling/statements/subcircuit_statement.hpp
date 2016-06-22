/*
 * subcircuit_statement.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#ifndef SUBCIRCUIT_STATEMENT_H
#define SUBCIRCUIT_STATEMENT_H

#include <istream>
#include <fstream>

#include "statement.hpp"
#include "instance_statement.hpp"

class SubcircuitStatement : public Statement{
public:
	SubcircuitStatement();
	SubcircuitStatement(Statement* belonging_circuit, LogIO* log_io );
	SubcircuitStatement( const SubcircuitStatement& orig );
	virtual ~SubcircuitStatement();

	virtual SubcircuitStatement* GetCopy() override;
	virtual std::string ExportCircuitStatement( const std::string& indentation ) override;

	std::vector<InstanceStatement*>* get_progeny() {return &progeny; }

	bool ParseSubcircuitStatement( Statement& global_scope_parent, std::ifstream* file,
		std::vector<std::string>& lineTockens, std::string & statementCode,
		std::string& currentReadLine, int& statementCount, bool& endOfFile,
		bool& parsingSpectreCode, const bool permissiveParsingMode );
	bool ParseInlineSubcircuitStatement(Statement& global_scope_parent,
		std::ifstream* file, std::vector<std::string>& lineTockens, std::string& statementCode,
		std::string& currentReadLine, int& statementCount, bool& endOfFile,
		bool& parsingSpectreCode, const bool permissiveParsingMode );
	void AddProgeny( InstanceStatement* instance_of_subcircuit );

	//Update Scope
	void UpdateOwnScope();
private:
	bool is_inline;
	// InstanceStatements whose
	// global_dependency_parent is (this) subcircuit
	std::vector<InstanceStatement*> progeny;

};

#endif /* SUBCIRCUIT_STATEMENT_H */
