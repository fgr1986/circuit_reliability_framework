/*
 * analog_model_statement.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */


#ifndef ANALOG_MODEL_STATEMENT_H
#define ANALOG_MODEL_STATEMENT_H

#include <vector>
#include <string>

#include "statement.hpp"

class AnalogModelStatement : public Statement{
public:
	AnalogModelStatement();
	AnalogModelStatement(Statement* belonging_circuit, LogIO* log_io, Scope* belonging_scope);
	AnalogModelStatement(const AnalogModelStatement& orig);
	virtual ~AnalogModelStatement();
	virtual std::string ExportCircuitStatement( const std::string& indentation) override;
	virtual AnalogModelStatement* GetCopy() override;

	bool ParseAnalogModelStatement(
		Statement& global_scope_parent,std::vector<std::string>& lineTockens,
		std::string & statementCode, int& statementCount);

	bool get_is_a_primitive_model() const{ return is_a_primitive_model; }
	void set_is_a_primitive_model(bool is_a_primitive_model){
		this->is_a_primitive_model = is_a_primitive_model;
	}

private:
	bool is_a_primitive_model;
};

#endif /* ANALOG_MODEL_STATEMENT_H */
