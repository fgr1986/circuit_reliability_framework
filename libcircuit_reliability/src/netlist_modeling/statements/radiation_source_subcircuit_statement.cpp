/*
 * subcircuit_statement.cpp
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */


#include <boost/algorithm/string.hpp>

#include "radiation_source_subcircuit_statement.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"
#include "../../global_functions_and_constants/netlist_words_constants.hpp"

RadiationSourceSubcircuitStatement::RadiationSourceSubcircuitStatement() {
	this->id = kNotDefinedInt;
	this->statement_type = kAlterationSourceSubcircuitStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->has_parallel_statements = false;
	this->correctly_parsed = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = true;
	this->has_nodes = true;
	this->has_children = true;
	this->has_parameters = true;
	this->has_raw_content = false;
	this->raw_content = kNotDefinedString;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kAlterationSourceSubcircuitStatementDesc;
	this->radiation_source_substitute_statement = false;
 	// scope 
	this->has_own_scope = true;
	this->own_scope = new Scope( kSubcircuitStatementDesc + "Scope", kSubcircuitStatementDesc, true);
	this->belonging_scope = own_scope;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;	
}

RadiationSourceSubcircuitStatement::RadiationSourceSubcircuitStatement(Statement* belonging_circuit,
		LogIO* log_io, Scope* belonging_scope) {
	this->id = kNotDefinedInt;
	this->statement_type = kAlterationSourceSubcircuitStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->has_parallel_statements = false;
	this->correctly_parsed = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = true;
	this->has_nodes = true;
	this->has_children = true;
	this->has_parameters = true;
	this->has_raw_content = false;
	this->raw_content = kNotDefinedString;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kAlterationSourceSubcircuitStatementDesc;
	this->belonging_circuit = belonging_circuit;
	this->radiation_source_substitute_statement = false;
	// scope
	this->has_own_scope = true;
	this->own_scope = new Scope( number2String(id) + "_scope", number2String(id), true );
	this->belonging_scope = own_scope;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;
	
	// logger
	this->log_io = log_io;
}


RadiationSourceSubcircuitStatement::RadiationSourceSubcircuitStatement(
		const RadiationSourceSubcircuitStatement& orig) {
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
	this->statement_type_description = kAlterationSourceSubcircuitStatementDesc;
	this->path_to_belonging_circuit = orig.path_to_belonging_circuit;
	this->belonging_circuit = orig.belonging_circuit;
	this->radiation_source_substitute_statement = orig.radiation_source_substitute_statement;
	// scope
	this->parent = orig.parent;
	this->global_scope_parent = orig.global_scope_parent;
	this->has_own_scope = true;
	this->own_scope = new Scope( number2String(id) + "_scope", number2String(id), true );
	this->belonging_scope = own_scope;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;
	
	// logger
	this->log_io = orig.log_io;
	deepCopyVectorOfPointers(orig.nodes, nodes);
	deepCopyOfChildren( orig.children );
	deepCopyVectorOfPointers(orig.parameters, parameters);
	// deepCopyVectorOfPointers(orig.children_instance_nodes, children_instance_nodes);
}

RadiationSourceSubcircuitStatement* RadiationSourceSubcircuitStatement::GetCopy() {
	return new RadiationSourceSubcircuitStatement(*this);
}

RadiationSourceSubcircuitStatement::~RadiationSourceSubcircuitStatement() {
}

std::string RadiationSourceSubcircuitStatement::ExportCircuitStatement(std::string indentation){
	//subckt SubcircuitName [(] node1 ... nodeN [)]
	//	[ parameters name1=value1 ... [nameN=valueN]]
	//	.
	//	.
	//	.
	//	instance, model, ic, or nodeset statements—or
	//	further subcircuit definitions
	//	.
	//	.
	//	.
	// ends [SubcircuitName]
	std::string cs =  kEmptyLine + indentation + kCommentWord1 + "Radiation Modeling";
	cs += kEmptyLine + indentation + kCommentWord1 + name + kDelimiter + kSubcircuitWord;
	cs += kEmptyLine + indentation + kSubcircuitWord + kDelimiter + name + kDelimiter;
	if(has_brackets){
		cs += "( ";
	}
	//export nodes
	for(std::vector<Node*>::iterator it_node = nodes.begin(); it_node !=  nodes.end(); it_node++){
		cs +=  (*it_node)->get_name() + kDelimiter;
	}
	if(has_brackets){
		cs += ")" + kEmptyLine;
	}
	//export parameters
	// if( !export_parameters ){
	// 	cs += kEmptyLine + indentation + kCommentWord1 + " Parameters in external parameters file";
	// }else{		
	if( parameters.size() > 0 ){
		cs += kEmptyLine +  indentation + kDelimiter + kParametersWord + kDelimiter;
		for(std::vector<Parameter*>::iterator it_parameter = parameters.begin();
			it_parameter !=  parameters.end(); it_parameter++){
			cs +=  kDelimiter + (*it_parameter)->ExportParameter();
		}
	}
	// }
	//export children
	for(std::vector<Statement*>::iterator it_children = children.begin();
	 it_children !=  children.end(); it_children++){
		cs += kEmptyLine + (*it_children)->ExportCircuitStatement(indentation + kTab);
	}
	
	cs += kEmptyLine + indentation + kEndSubcircuitWord + kDelimiter + name;
	cs += kEmptyLine + indentation + kCommentWord1
		+ " end of " + name + " subcircuit";
	cs += kEmptyLine;
	return cs;
}
