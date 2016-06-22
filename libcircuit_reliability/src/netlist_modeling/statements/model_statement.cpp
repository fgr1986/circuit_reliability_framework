/*
 * model.cpp
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#include <boost/algorithm/string.hpp>

#include "model_statement.hpp"

#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"
#include "../../global_functions_and_constants/netlist_words_constants.hpp"

ModelStatement::ModelStatement() {
	this->id = kNotDefinedInt;
	this->statement_type = kModelStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->correctly_parsed = false;
	this->has_parallel_statements = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = true;
	this->has_nodes = false;
	this->has_children = false;
	this->has_parameters = true;
	this->has_raw_content = false;
	this->raw_content = kNotDefinedString;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kModelStatementDesc;
	// scope
	this->has_own_scope = false;
	// Pointer to master_instance (analog) model/subck/primitive

	this->scanned_for_instances_dependency = false;
	this->consider_instances_dependency = true;
	this->is_a_primitive_model = false;
}

ModelStatement::ModelStatement(Statement* belonging_circuit,
		LogIO* log_io, Scope* belonging_scope) {
	this->id = kNotDefinedInt;
	this->statement_type = kModelStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->correctly_parsed = false;
	this->has_parallel_statements = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = true;
	this->has_nodes = false;
	this->has_children = false;
	this->has_parameters = true;
	this->has_raw_content = false;
	this->raw_content = kNotDefinedString;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kModelStatementDesc;
	this->belonging_circuit = belonging_circuit;
	// scope
	this->belonging_scope = belonging_scope;
	this->has_own_scope = false;
	// logger
	this->log_io = log_io;
	// Pointer to master_instance (analog) model/subck/primitive

	this->scanned_for_instances_dependency = false;
	this->consider_instances_dependency = true;
	this->is_a_primitive_model = false;
}

ModelStatement::ModelStatement(const ModelStatement& orig) {
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
	this->substitute_master_name = orig.substitute_master_name;
	this->altered = orig.altered;
	this->can_be_injected = orig.can_be_injected;
	this->belonging_circuit = orig.belonging_circuit;
	this->statement_type_description = kModelStatementDesc;
	this->path_to_belonging_circuit = orig.path_to_belonging_circuit;
	// scope
	// changed after deepCopyVectorOfPointers
	this->belonging_scope = orig.belonging_scope;
	this->has_own_scope = orig.has_own_scope;
	// logger
	this->log_io = orig.log_io;
	// Pointer to master_instance (analog) model/subck/primitive
	this->scanned_for_instances_dependency = orig.scanned_for_instances_dependency;
	this->consider_instances_dependency = orig.consider_instances_dependency;
	this->is_a_primitive_model = orig.is_a_primitive_model;
	deepCopyVectorOfPointers(orig.parameters, parameters);
}

ModelStatement* ModelStatement::GetCopy() {
	return new ModelStatement(*this);
}

ModelStatement::~ModelStatement() {
}


std::string ModelStatement::ExportCircuitStatement( const std::string& indentation ){
	//model name master [param1=value1 ... [param2=value2 ]]
	std::string cs = indentation + kModelWord + kDelimiter;
	cs += name + kDelimiter ;
	cs += master_name;
	//export parameters
	for(std::vector<Parameter*>::iterator it_parameter = parameters.begin();
	 it_parameter !=  parameters.end(); it_parameter++){
		cs +=  kDelimiter + (*it_parameter)->ExportParameter();
	}
	return cs;
}

bool ModelStatement::ParseModelStatement( Statement& global_scope_parent,
	std::vector<std::string>& lineTockens,
	std::string& statementCode, int& statementCount){
	// model model_name model_type {
	// 1:  type= params
	// ...
	// }
	// model model_name model_type
	// {
	// 1:  type= params
	// ...
	// }
	// model model_name model_type params
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing model: '" + statementCode + "'" );
	#endif

	set_id(statementCount++);
	this->global_scope_parent = &global_scope_parent;
	// parse name
	set_name(lineTockens.at(1));
	set_master_name(lineTockens.at(2));
	// Test unalterable statement
	set_unalterable( TestUnalterableStatement(master_name) );
	// Test injection/replacement
	set_can_be_injected( TestCanBeInjected(master_name) );
	set_can_be_substituted( TestCanBeSubstituted(master_name, substitute_master_name) );
	// Instantiates a Primitive
	set_is_a_primitive_model( TestIsInstanceOfPrimitive(master_name) );
	if( is_a_primitive_model ){
		scanned_for_instances_dependency = true;
		dependency_parent = this;
		#ifdef PARSING_VERBOSE
			log_io->ReportPlain2Log( "model '" + name + "' is model of primitive: '" + master_name + "'" );
		#endif
	}
	// parse parameters
	if(boost::contains(statementCode, kEqualsWord)){
		ParseParameters( statementCode, false );
	}
	correctly_parsed = true;
	if(correctly_parsed){
		#ifdef PARSING_VERBOSE_MIN
			log_io->ReportPlain2Log( "model: '" + name + "', '" + master_name + "' parsed" );
		#endif
	}
	return correctly_parsed;
}
