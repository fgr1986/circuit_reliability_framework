/*
 * analysis_statement.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#ifndef PARAM_TEST_STATEMENT_H
#define PARAM_TEST_STATEMENT_H

#include "statement.hpp"

class ParamTestStatement : public Statement{
public:
	ParamTestStatement();
	ParamTestStatement(Statement* belonging_circuit, LogIO* log_io, Scope* belonging_scope);
	ParamTestStatement(const ParamTestStatement& orig);
	virtual ~ParamTestStatement();

	virtual ParamTestStatement* GetCopy() override;
	virtual std::string ExportCircuitStatement( const std::string& indentation) override;

	bool ParseParamTestStatement( Statement& global_scope_parent, std::vector<std::string>& lineTockens,
		std::string & readLine, int& statementCount);
private:
};

#endif /* PARAM_TEST_STATEMENT_H */
