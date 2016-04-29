/*
 * simple_statement.cpp
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#include "simple_statement.hpp"
/// constants
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"

SimpleStatement::SimpleStatement() {
	this->id = kNotDefinedInt;
	this->statement_type = kSimpleStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->correctly_parsed = false;
	this->has_parallel_statements = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = false;
	this->has_nodes = false;
	this->has_children = false;
	this->has_parameters = false;
	this->has_raw_content = true;
	this->raw_content = kNotDefinedString;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kSimpleStatementDesc;
	// scope
	this->belonging_scope = belonging_scope;
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

}
SimpleStatement::SimpleStatement(Statement* belonging_circuit,
		LogIO* log_io, Scope* belonging_scope) {
	this->id = kNotDefinedInt;
	this->statement_type = kSimpleStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->correctly_parsed = false;
	this->has_parallel_statements = false;
	this->name = kNotDefinedString;
	this->master_name = kSimpleStatementDesc;
	this->description = kSimpleStatementDesc;
	this->has_brackets = false;
	this->has_nodes = false;
	this->has_children = false;
	this->has_parameters = false;
	this->has_raw_content = true;
	this->raw_content = kNotDefinedString;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kSimpleStatementDesc;
	this->belonging_circuit = belonging_circuit;
	// scope
	this->belonging_scope = belonging_scope;
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

	//logger manager
	this->log_io = log_io;
}

SimpleStatement::SimpleStatement(const std::string raw_content, LogIO* log_io) {
	this->id = kNotDefinedInt;
	this->statement_type = kSimpleStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->correctly_parsed = false;
	this->has_parallel_statements = false;
	this->name = kNotDefinedString;
	this->master_name = kSimpleStatementDesc;
	this->description = kSimpleStatementDesc;
	this->has_brackets = false;
	this->has_nodes = false;
	this->has_children = false;
	this->has_parameters = false;
	this->has_raw_content = true;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kSimpleStatementDesc;
	this->raw_content = raw_content;
	// scope
	this->belonging_scope = belonging_scope;
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

	// logger
	this->log_io = log_io;
}

SimpleStatement::SimpleStatement( const std::string raw_content ) {
	this->id = kNotDefinedInt;
	this->statement_type = kSimpleStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->correctly_parsed = false;
	this->has_parallel_statements = false;
	this->name = kNotDefinedString;
	this->master_name = kSimpleStatementDesc;
	this->description = kSimpleStatementDesc;
	this->has_brackets = false;
	this->has_nodes = false;
	this->has_children = false;
	this->has_parameters = false;
	this->has_raw_content = true;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kSimpleStatementDesc;
	this->raw_content = raw_content;
	// scope
	this->belonging_scope = belonging_scope;
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

	// logger
	this->log_io = nullptr;
}

SimpleStatement::SimpleStatement(const SimpleStatement& orig) {
	this->id = orig.id;
	this->statement_type = orig.statement_type;
	this->parallel_statements = orig.parallel_statements;
	this->has_parallel_statements = orig.has_parallel_statements;
	this->correctly_parsed = orig.correctly_parsed;
	this->name = orig.name;
	this->master_name = orig.master_name;
	this->has_brackets = orig.has_brackets;
	this->has_nodes = orig.has_nodes;
	this->has_children = orig.has_children;
	this->has_parameters = orig.has_parameters;
	this->has_raw_content = orig.has_raw_content;
	this->raw_content = orig.raw_content;
	this->unalterable = orig.unalterable;
	this->can_be_substituted = orig.can_be_substituted;
	this->substitute_master_name = orig.can_be_substituted;
	this->altered = orig.altered;
	this->can_be_injected = orig.can_be_injected;
	this->belonging_circuit = orig.belonging_circuit;
	this->path_to_belonging_circuit = orig.path_to_belonging_circuit;
	// scope
	this->belonging_scope = orig.belonging_scope;
	this->has_own_scope = orig.has_own_scope;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

	//logger manager
	this->log_io = orig.log_io;
}

SimpleStatement* SimpleStatement::GetCopy() {
	return new SimpleStatement(*this);
}

SimpleStatement::~SimpleStatement() {
}

std::string SimpleStatement::ExportCircuitStatement(std::string indentation){
	// statement
	// std::string cs = indentation + raw_content;
	return indentation + raw_content;
}

bool SimpleStatement::ParseSimpleStatement( Statement& global_scope_parent,
		std::string & statementCode, int& statementCount ){
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing simple statement: '" + statementCode + "'" );
	#endif
	raw_content = statementCode;
	set_id(statementCount++);
	this->global_scope_parent = &global_scope_parent;
	correctly_parsed = true;
	#ifdef PARSING_VERBOSE_MIN
		log_io->ReportPlain2Log( "simpleStatement: '" + raw_content + "' parsed" );
	#endif
	return correctly_parsed;
}
