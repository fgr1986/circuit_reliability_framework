/*
 * primitive.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#ifndef GLOBAL_STATEMENT_H
#define GLOBAL_STATEMENT_H

#include "statement.hpp"

class GlobalStatement : public Statement{
public:
	GlobalStatement();
	GlobalStatement(Statement* belonging_circuit, LogIO* log_io, Scope* belonging_scope);
	GlobalStatement(const GlobalStatement& orig);
	virtual ~GlobalStatement();
	virtual std::string ExportCircuitStatement( const std::string& indentation ) override;

	virtual GlobalStatement* GetCopy() override;
	bool ParseGlobalStatement(Statement& global_scope_parent, std::vector<std::string>& lineTockens,
		std::string & readLine, int& statementCount);
private:
};

#endif /* GLOBAL_STATEMENT_H */
