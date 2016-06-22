/*
 * primitive.cpp
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */


#include <boost/algorithm/string.hpp>

#include "instance_statement.hpp"

#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"
#include "../../global_functions_and_constants/netlist_words_constants.hpp"

InstanceStatement::InstanceStatement() {
	this->id = kNotDefinedInt;
	this->statement_type = kInstanceStatement;
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
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	// Description
	this->statement_type_description = kInstanceStatementDesc;
	// scope
	this->has_own_scope = false;
	// Acts as a primitive?
	this->is_a_primitive_instance = false;
	// Pointer to master_instance (analog) model/subck/primitive

	this->scanned_for_instances_dependency = false;
	this->consider_instances_dependency = true;
}
InstanceStatement::InstanceStatement(Statement* belonging_circuit,
		LogIO* log_io, Scope* belonging_scope) {
	this->id = kNotDefinedInt;
	this->statement_type = kInstanceStatement;
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
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kInstanceStatementDesc;
	this->belonging_circuit = belonging_circuit;
	// scope
	this->belonging_scope = belonging_scope;
	this->has_own_scope = false;
	// logger
	this->log_io = log_io;
	// Acts as a primitive?
	this->is_a_primitive_instance = false;
	// Pointer to master_instance (analog) model/subck/primitive

	this->scanned_for_instances_dependency = false;
	this->consider_instances_dependency = true;
}

InstanceStatement::InstanceStatement(const InstanceStatement& orig) {
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
	this->altered = orig.altered;
	this->unalterable = orig.unalterable;
	this->can_be_injected = orig.can_be_injected;
	this->can_be_substituted = orig.can_be_substituted;
	this->substitute_master_name = orig.substitute_master_name;
	this->belonging_circuit = orig.belonging_circuit;
	this->statement_type_description = orig.statement_type_description;
	this->path_to_belonging_circuit = orig.path_to_belonging_circuit;
	// scope
	this->belonging_scope = orig.belonging_scope;
	this->has_own_scope = orig.has_own_scope;
	// logger
	this->log_io = orig.log_io;
	// Acts as a primitive?
	this->is_a_primitive_instance = orig.is_a_primitive_instance;
	// Pointer to master_instance (analog) model/subck/primitive
	this->dependency_parent = orig.dependency_parent;
	this->scanned_for_instances_dependency = orig.scanned_for_instances_dependency;
	this->consider_instances_dependency = orig.consider_instances_dependency;
	deepCopyVectorOfPointers(orig.nodes, nodes);
	deepCopyVectorOfPointers(orig.parameters, parameters);
}

InstanceStatement* InstanceStatement::GetCopy() {
	return new InstanceStatement(*this);
}

InstanceStatement::~InstanceStatement() {
}

std::string InstanceStatement::ExportCircuitStatement( const std::string& indentation ){
	//name [(]node1 ... nodeN[)] master [[param1=value1] ...[paramN=valueN]]
	std::string cs = indentation + name + kDelimiter ;
	if(nodes.size() > 0){
		if(has_brackets){
			cs += "( ";
		}
		//export nodes
		for(std::vector<Node*>::iterator it_node = nodes.begin() ; it_node !=  nodes.end(); it_node++){
			cs +=  (*it_node)->get_name() + kDelimiter;
		}
		if(has_brackets){
			cs += ")" +kDelimiter ;
		}
	}
	cs += master_name;
	//export parameters
	if(parameters.size() > 0){
		for(std::vector<Parameter*>::iterator it_parameter = parameters.begin();
		 it_parameter !=  parameters.end(); it_parameter++){
			cs +=  kDelimiter + (*it_parameter)->ExportParameter();
		}
	}
	//parallel instances
	if(has_parallel_statements){
		cs += " m=" + parallel_statements;
	}
	return cs;
}

bool InstanceStatement::ParseInstanceStatement( Statement& global_scope_parent,
	std::vector<std::string>& lineTockens, std::string & statementCode, int& statementCount){
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing instance: '" + statementCode + "'" );
	#endif
	//std::string nameNodesMasterNameLine = statementCode;
	set_id(statementCount++);
	this->global_scope_parent = &global_scope_parent;
	// parse name
	set_name(lineTockens.front());
	// parse parameters
	if(boost::contains(statementCode, kEqualsWord)){
		ParseParameters( statementCode, false );
	}
	//remove brakets from statementCode and add brackets in future
	boost::replace_first(statementCode, kParenthesisStartWord, kEmptyWord);
	boost::replace_last(statementCode, kParenthesisEndWord, kEmptyWord);
	set_has_brackets(true);
	//parse masterName
	boost::split(lineTockens, statementCode, boost::is_any_of(kDelimiter),
		boost::token_compress_on);
	set_master_name(lineTockens.back());
	// Test Unalterable
	#ifdef PARSING_VERBOSE
	log_io->ReportPlain2Log( "Test Unalterable for '" + statementCode + "'" );
	#endif
	set_unalterable( TestUnalterableStatement(master_name) );
	// Test injection/replacement
	#ifdef PARSING_VERBOSE
	log_io->ReportPlain2Log( "Test injection/replacement for '" + statementCode + "'" );
	#endif
	set_can_be_injected( TestCanBeInjected(master_name) );
	set_can_be_substituted( TestCanBeSubstituted(master_name, substitute_master_name) );
	// Instanciate a primitive
	set_is_a_primitive_instance( TestIsInstanceOfPrimitive(master_name) );
	if( is_a_primitive_instance ){
		scanned_for_instances_dependency = true;
		dependency_parent = this;
		#ifdef PARSING_VERBOSE
			log_io->ReportPlain2Log( "instance '" + name + "' is instance of primitive: '" + master_name + "'" );
		#endif
	}
	//parse nodes
	// [name node1 ... nodeN ]
	if(lineTockens.size() > 2){
		for(std::vector<std::string>::iterator it_node = (++lineTockens.begin());
			it_node != (--lineTockens.end()); it_node++){
			AddNode( *it_node );
		}
	}
	correctly_parsed = true;
	if(correctly_parsed){
		#ifdef PARSING_VERBOSE_MIN
			log_io->ReportPlain2Log( "instance: '" + name + "', '" + master_name + "' parsed" );
		#endif
	}
	return correctly_parsed;
}
