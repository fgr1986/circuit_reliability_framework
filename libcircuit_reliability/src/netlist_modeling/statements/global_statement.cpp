/*
 * primitive.cpp
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */


#include <boost/algorithm/string.hpp>

#include "global_statement.hpp"

#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"

GlobalStatement::GlobalStatement() {
	this->id = kNotDefinedInt;
	this->statement_type = kGlobalStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->correctly_parsed = false;
	this->has_parallel_statements = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = false;
	this->has_nodes = true;
	this->has_children = false;
	this->has_parameters = false;
	this->has_raw_content = false;
	this->raw_content = kNotDefinedString;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kGlobalStatementDesc;
	// scope
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

}

GlobalStatement::GlobalStatement(Statement* belonging_circuit,
		LogIO* log_io, Scope* belonging_scope) {
	this->id = kNotDefinedInt;
	this->statement_type = kGlobalStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->correctly_parsed = false;
	this->has_parallel_statements = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = false;
	this->has_nodes = true;
	this->has_children = false;
	this->has_parameters = false;
	this->has_raw_content = false;
	this->raw_content = kNotDefinedString;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kGlobalStatementDesc;
	this->belonging_circuit = belonging_circuit;
	//scope
	this->has_own_scope = false;
	this->belonging_scope = belonging_scope;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

	this->log_io = log_io;
}

GlobalStatement::GlobalStatement(const GlobalStatement& orig) {
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
	this->statement_type_description = kGlobalStatementDesc;
	this->path_to_belonging_circuit = orig.path_to_belonging_circuit;
	// scope
	// changed after deep copy
	this->belonging_scope = orig.belonging_scope;
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;


	// logger
	this->log_io = orig.log_io;
	deepCopyVectorOfPointers(orig.nodes, nodes);
	deepCopyVectorOfPointers(orig.parameters, parameters);
}

GlobalStatement* GlobalStatement::GetCopy() {
	return new GlobalStatement(*this);
}

GlobalStatement::~GlobalStatement() {
}

std::string GlobalStatement::ExportCircuitStatement( const std::string& indentation ){
	//name [(]node1 ... nodeN[)] master [[param1=value1] ...[paramN=valueN]]
	std::string cs = indentation + name + kDelimiter ;

	if(nodes.size() > 0){
		//export nodes
		for(std::vector<Node*>::iterator it_node = nodes.begin() ; it_node !=  nodes.end(); it_node++){
			cs +=  (*it_node)->get_name() + kDelimiter;
		}
	}
	return cs;
}

bool GlobalStatement::ParseGlobalStatement(Statement& global_scope_parent,
	std::vector<std::string>& lineTockens, std::string & statementCode, int& statementCount){

	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing global statement: '" + statementCode + "'" );
	#endif

	//std::string nameNodesMasterNameLine = statementCode;
	set_id(statementCount++);
	this->global_scope_parent = &global_scope_parent;
	// parse name
	set_name(lineTockens.front());
	set_master_name(lineTockens.front());

	set_has_brackets(false);
	//parse nodes
	// [name node1 ... nodeN ]
	if(lineTockens.size() > 2){
		for(std::vector<std::string>::iterator it_node = (++lineTockens.begin());
			it_node != lineTockens.end(); it_node++){
			AddNode( *it_node );
			AddGlobalNode( nodes.back() );
		}
	}
	correctly_parsed = true;
	if(correctly_parsed){
		#ifdef PARSING_VERBOSE_MIN
			log_io->ReportPlain2Log( "global statement: '" + name + "', '" + master_name + "' parsed" );
		#endif
	}
	return correctly_parsed;
}
