/*
 * primitive.cpp
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */


#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

#include "analog_model_statement.hpp"

#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"
#include "../../global_functions_and_constants/netlist_words_constants.hpp"

AnalogModelStatement::AnalogModelStatement() {
	this->id = kNotDefinedInt;
	this->statement_type = kAnalogModelStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->has_parallel_statements = false;
	this->correctly_parsed = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = true;
	this->has_nodes = true;
	this->has_children = false;
	this->has_parameters = true;
	this->has_raw_content = false;
	this->raw_content = kNotDefinedString;
	// Radiation Properties
	this->unalterable = true;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->altered = false;
	this->statement_type_description = kAnalogModelStatementDesc;
	// scope
	this->has_own_scope = false;
	// Pointer to master_instance (analog) model/subck/primitive

	this->scanned_for_instances_dependency = false;
	this->consider_instances_dependency = true;
	this->is_a_primitive_model = false;
}

AnalogModelStatement::AnalogModelStatement(Statement* belonging_circuit,
		LogIO* log_io, Scope* belonging_scope) {
	this->id = kNotDefinedInt;
	this->statement_type = kAnalogModelStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->has_parallel_statements = false;
	this->correctly_parsed = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = true;
	this->has_nodes = true;
	this->has_children = false;
	this->has_parameters = true;
	this->has_raw_content = false;
	this->raw_content = kNotDefinedString;
	// Radiation Properties
	this->unalterable = true;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->altered = false;
	this->belonging_circuit = belonging_circuit;
	this->statement_type_description = kAnalogModelStatementDesc;
	// scope
	this->belonging_scope = belonging_scope;
	this->has_own_scope = false;
	//logger manager
	this->log_io = log_io;
	// Pointer to master_instance (analog) model/subck/primitive

	this->scanned_for_instances_dependency = false;
	this->consider_instances_dependency = true;
	this->is_a_primitive_model = false;
}


AnalogModelStatement::AnalogModelStatement(const AnalogModelStatement& orig) {
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
	this->statement_type_description = kAnalogModelStatementDesc;
	this->path_to_belonging_circuit = orig.path_to_belonging_circuit;
	// scope
	// changed after deepCopyVectorOfPointers
	this->belonging_scope = orig.belonging_scope;
	this->has_own_scope = orig.has_own_scope;
	//logger manager
	this->log_io = orig.log_io;
	// Pointer to master_instance (analog) model/subck/primitive
	this->scanned_for_instances_dependency = orig.scanned_for_instances_dependency;
	this->consider_instances_dependency = orig.consider_instances_dependency;
	this->is_a_primitive_model = orig.is_a_primitive_model;
	deepCopyVectorOfPointers(orig.nodes, nodes);
	deepCopyVectorOfPointers(orig.parameters, parameters);
}

AnalogModelStatement* AnalogModelStatement::GetCopy() {
	return new AnalogModelStatement(*this);
}

AnalogModelStatement::~AnalogModelStatement() {
}

std::string AnalogModelStatement::ExportCircuitStatement(std::string indentation){
	//name [(]node1 ... nodeN[)] analogmodel modelname=mastername [[param1=value1] ...[paramN=valueN]]
	std::string cs = indentation + name + kDelimiter ;
	if(has_brackets){
		cs += "( ";
	}
	//export nodes
	for( auto const &node : nodes ){
		cs += node->get_name() + kDelimiter;
	}
	if(has_brackets){
		cs += ") ";
	}
	cs += kAnalogModelWord + kDelimiter;
	cs += kAnalogModelNameWord + kEqualsWord + master_name;
	//export parameters
	if(parameters.size() > 0){
		for( auto const &p : parameters ){
			cs +=  kDelimiter + p->ExportParameter();
		}
	}
	//parallel instances
	if(has_parallel_statements){
		cs += " m=" + parallel_statements;
	}
	return cs;
}

bool AnalogModelStatement::ParseAnalogModelStatement(
	Statement& global_scope_parent, std::vector<std::string>& lineTockens,
	std::string & statementCode, int& statementCount){
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing analog model: '" + statementCode + "'" );
	#endif
	//name [(]node1 ... nodeN[)] analogmodel modelname=mastername [[param1=value1] ...[paramN=valueN]]
	this->global_scope_parent = &global_scope_parent;
	// parse name
	set_id(statementCount++);
	set_name(lineTockens.front());
	std::vector<std::string> amt;
	boost::algorithm::split_regex( amt, statementCode,
	boost::regex( kAnalogModelSplitWord ) ) ;
	// parse parameters
	if(boost::contains(amt.back(), kEqualsWord)){
		ParseParameters( amt.back(), false );
	}
	set_master_name(amt.back());
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
			log_io->ReportPlain2Log( "analog model '" + name + "' is analog model of primitive: '" + master_name + "'" );
		#endif
	}
	//remove brakets from statementCode and add brackets in future
	boost::replace_first(amt.front(), kParenthesisStartWord, kEmptyWord);
	boost::replace_last(amt.front(), kParenthesisEndWord, kEmptyWord);
	set_has_brackets(true);
	//parse masterName
	boost::split(lineTockens, amt.front(), boost::is_any_of(kDelimiter), boost::token_compress_on);
	//parse nodes
	// [name node1 ... nodeN ]
	if(lineTockens.size() > 2){
		for(std::vector<std::string>::iterator it_node = (++lineTockens.begin());
			it_node != lineTockens.end(); it_node++){
			AddNode( *it_node );
		}
	}
	correctly_parsed = true;
	if(correctly_parsed){
		#ifdef PARSING_VERBOSE_MIN
			log_io->ReportPlain2Log( "analogmodel: '" + name + "', '" + master_name + "' parsed" );
		#endif
	}
	return correctly_parsed;
}
