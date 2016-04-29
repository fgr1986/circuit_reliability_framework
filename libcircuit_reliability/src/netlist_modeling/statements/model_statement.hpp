/*
 * primitive.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#ifndef MODEL_STATEMENT_H
#define MODEL_STATEMENT_H
 
#include <string>

#include "statement.hpp"

class ModelStatement : public Statement{
public:
	ModelStatement();
	ModelStatement(Statement* belonging_circuit, LogIO* log_io, Scope* belonging_scope);
	ModelStatement(const ModelStatement& orig);
	virtual ~ModelStatement();

	virtual ModelStatement* GetCopy();
	virtual std::string ExportCircuitStatement(std::string indentation);

	bool ParseModelStatement( Statement& global_scope_parent, std::vector<std::string>& lineTockens,
		std::string& statementCode, int& statementCount);

	bool get_is_a_primitive_model() const{ return is_a_primitive_model; }
	void set_is_a_primitive_model(bool is_a_primitive_model){
		this->is_a_primitive_model = is_a_primitive_model;
	}

private:
	bool is_a_primitive_model;
};

#endif /* MODEL_STATEMENT_H */
