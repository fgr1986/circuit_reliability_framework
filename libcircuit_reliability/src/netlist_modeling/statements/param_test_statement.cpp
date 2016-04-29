/*
 * param_test_statement.cpp
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

#include "param_test_statement.hpp"
 
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"
#include "../../global_functions_and_constants/netlist_words_constants.hpp"

ParamTestStatement::ParamTestStatement() {
	this->id = kNotDefinedInt;
	this->statement_type = kParamTestStatement;
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
	this->statement_type_description = kParamTestStatementDesc;
	// scope
	this->belonging_scope = belonging_scope;
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;
	
}

ParamTestStatement::ParamTestStatement(Statement* belonging_circuit, 
		LogIO* log_io, Scope* belonging_scope) {
	this->id = kNotDefinedInt;
	this->statement_type = kParamTestStatement;
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
	this->statement_type_description = kParamTestStatementDesc;
	this->belonging_circuit = belonging_circuit;
	// scope
	this->belonging_scope = belonging_scope;
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;
	
	// logger
	this->log_io = log_io;
}

ParamTestStatement::ParamTestStatement(const ParamTestStatement& orig) {
	this->id = orig.id;
	this->statement_type = orig.statement_type;
	this->parallel_statements = orig.parallel_statements;
	this->correctly_parsed = true;
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
	this->altered = orig.altered;
	this->can_be_injected = orig.can_be_injected;
	this->substitute_master_name = orig.substitute_master_name;
	this->statement_type_description = kParamTestStatementDesc;
	this->path_to_belonging_circuit = orig.path_to_belonging_circuit;
	this->belonging_circuit = orig.belonging_circuit;
	// scope
	this->belonging_scope = orig.belonging_scope;
	this->has_own_scope = orig.has_own_scope;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;
	
	// logger
	this->log_io = orig.log_io;
}

ParamTestStatement* ParamTestStatement::GetCopy() {
	return new ParamTestStatement(*this);
}


ParamTestStatement::~ParamTestStatement() {
}

std::string ParamTestStatement::ExportCircuitStatement(std::string indentation){
	//checkFreqs paramtest errorif=((bw/2-fc)>=0)
	// + message="center frequency must be greater than half the bandwidth"
	std::string cs = indentation + name + kDelimiter ;
	cs += kParamTestWord + kDelimiter + raw_content;
	return cs;
}

bool ParamTestStatement::ParseParamTestStatement(Statement& global_scope_parent, 
	std::vector<std::string>& lineTockens,
	std::string& statementCode, int& statementCount){
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing paramtest: '" + statementCode + "'" );	
	#endif
	std::string nameNodesMasterNameLine = statementCode;
	// parse name
	set_id(statementCount++);
	this->global_scope_parent = &global_scope_parent;
	set_name(lineTockens.front());	
	// parse raw_content
	boost::algorithm::split_regex( lineTockens, statementCode,
		boost::regex( kParamTestAndDelimiterWord ) ) ;
	set_raw_content(lineTockens.back());
	correctly_parsed = true;
	if(correctly_parsed){
		#ifdef PARSING_VERBOSE_MIN
			log_io->ReportPlain2Log( "paramtest: '" + name + "', '" + master_name + "' parsed" );
		#endif
	}
	return correctly_parsed;
}
