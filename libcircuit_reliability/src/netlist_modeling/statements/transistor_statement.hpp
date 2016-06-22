/*
 * transistor_statement.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#ifndef TRANSISTOR_STATEMENT_H
#define TRANSISTOR_STATEMENT_H

#include "instance_statement.hpp"

class TransistorStatement : public InstanceStatement{
public:
	TransistorStatement();
	TransistorStatement(Statement* belonging_circuit, LogIO* log_io, Scope* belonging_scope);
	TransistorStatement(const TransistorStatement& orig);
	virtual ~TransistorStatement();
	virtual std::string ExportCircuitStatement( const std::string& indentation ) override;

	virtual TransistorStatement* GetCopy() override;
	bool ParseTransistorStatement( Statement& global_scope_parent, std::vector<std::string>& lineTockens,
		std::string & readLine, int& statementCount);

private:
};

#endif /* TRANSISTOR_STATEMENT_H */
