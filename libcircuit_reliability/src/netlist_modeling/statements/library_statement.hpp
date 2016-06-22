/*
 * library_statement.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#ifndef LIBRARY_STATEMENT_H
#define LIBRARY_STATEMENT_H

#include <string>
#include <vector>

#include "statement.hpp"

class LibraryStatement : public Statement{
public:
	LibraryStatement();
	LibraryStatement(Statement* belonging_circuit, LogIO* log_io, Scope* belonging_scope);
	LibraryStatement(const LibraryStatement& orig);
	virtual ~LibraryStatement();

	virtual LibraryStatement* GetCopy() override;
	virtual std::string ExportCircuitStatement( const std::string&  indentation ) override;

	bool ParseLibraryStatement( Statement& global_scope_parent, std::ifstream* file,
		std::vector<std::string>& lineTockens,
		std::string & statementCode, std::string& currentReadLine,
		int& statementCount, bool& endOfFile, bool& parsingSpectreCode, const bool permissiveParsingMode );
private:
};

#endif /* LIBRARY_STATEMENT_H */
