/*
 * primitive.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#ifndef PRIMITIVE_STATEMENT_H
#define PRIMITIVE_STATEMENT_H

#include "statement.hpp"
 
class InstanceStatement : public Statement{
public:
	InstanceStatement();
	InstanceStatement(Statement* belonging_circuit, LogIO* log_io, Scope* belonging_scope);
	InstanceStatement(const InstanceStatement& orig);
	virtual ~InstanceStatement();
	virtual std::string ExportCircuitStatement(std::string indentation);

	virtual InstanceStatement* GetCopy();
	bool ParseInstanceStatement( Statement& global_scope_parent, 
		std::vector<std::string>& lineTockens,
		std::string & readLine, int& statementCount);

	bool get_is_a_primitive_instance() const{ return is_a_primitive_instance; }
	void set_is_a_primitive_instance(bool is_a_primitive_instance){
		this->is_a_primitive_instance = is_a_primitive_instance;
	}

protected:
	bool is_a_primitive_instance;

};

#endif /* PRIMITIVE_STATEMENT_H */
