/*
 * section_statement.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#ifndef SECTION_STATEMENT_H
#define SECTION_STATEMENT_H

#include <string>
#include "statement.hpp"
 
class SectionStatement : public Statement{
public:
	SectionStatement();
	SectionStatement(Statement* belonging_circuit, LogIO* log_io, Scope* belonging_scope);
	SectionStatement(const SectionStatement& orig);
	virtual ~SectionStatement();
	
	virtual SectionStatement* GetCopy();
	virtual std::string ExportCircuitStatement(std::string indentation);

	bool ParseSectionStatement( Statement& global_scope_parent, std::ifstream* file,
		std::vector<std::string>& lineTockens, std::string & statementCode,
		std::string& currentReadLine, int& statementCount, bool& endOfFile,
		bool& parsingSpectreCode, const bool permissiveParsingMode);
private:
	
};

#endif /* SECTION_STATEMENT_H */
