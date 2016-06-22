/*
 * SIMPLE_statement.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#ifndef SIMPLE_STATEMENT_H
#define SIMPLE_STATEMENT_H

#include "statement.hpp"

class SimpleStatement : public Statement{
public:
	SimpleStatement();
	SimpleStatement(Statement* belonging_circuit, LogIO* log_io, Scope* belonging_scope );
	SimpleStatement(const std::string raw_content, LogIO* log_io );
	SimpleStatement(const std::string raw_content );
	/// special for oceanEval
	SimpleStatement(const bool allows_mute, const std::string raw_content );
	SimpleStatement(const SimpleStatement& orig);
	virtual ~SimpleStatement();
	virtual std::string ExportCircuitStatement( const std::string& indentation ) override;

	virtual SimpleStatement* GetCopy() override;
	bool ParseSimpleStatement( Statement& global_scope_parent, std::string & statementCode, int& statementCount );

	bool get_allows_mute() const{ return allows_mute; }

private:
	bool allows_mute;
};

#endif /* SIMPLE_STATEMENT_H */
