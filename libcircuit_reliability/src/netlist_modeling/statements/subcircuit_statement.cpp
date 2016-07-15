/*
 * subcircuit_statement.cpp
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */


#include <boost/algorithm/string.hpp>

#include "subcircuit_statement.hpp"

#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"
#include "../../global_functions_and_constants/netlist_words_constants.hpp"

// Needed by RadiationSourceSubcircuitStatement()
SubcircuitStatement::SubcircuitStatement() {
	this->id = kNotDefinedInt;
	this->statement_type = kSubcircuitStatement;
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
	this->statement_type_description = kSubcircuitStatementDesc;
	this->is_inline = false;
 	// scope
 	this->global_scope_parent = this;
	this->has_own_scope = true;
	this->own_scope = new Scope(
		 "subcircuit_" + number2String(id) + "_scope", name , true );
	this->belonging_scope = own_scope;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

}

SubcircuitStatement::SubcircuitStatement( Statement* belonging_circuit, LogIO* log_io ) {
	this->id = kNotDefinedInt;
	this->statement_type = kSubcircuitStatement;
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
	// Scope
	this->has_own_scope = true;
	this->statement_type_description = kSubcircuitStatementDesc;
	this->belonging_circuit = belonging_circuit;
	this->is_inline = false;
	// scope
	this->global_scope_parent = this;
	this->has_own_scope = true;
	this->own_scope = new Scope(
		 "subcircuit_" + number2String(id) + "_scope", name , true );
	this->belonging_scope = own_scope;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

	// logger
	this->log_io = log_io;
}

SubcircuitStatement::SubcircuitStatement(const SubcircuitStatement& orig) {
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
	this->can_be_substituted = orig.can_be_substituted;
	this->substitute_master_name = orig.substitute_master_name;
	this->unalterable = orig.unalterable;
	this->altered = orig.altered;
	this->can_be_injected = orig.can_be_injected;
	this->statement_type_description = kSubcircuitStatementDesc;
	this->path_to_belonging_circuit = orig.path_to_belonging_circuit;
	this->belonging_circuit = orig.belonging_circuit;
	this->is_inline = orig.is_inline;
	// scope
	this->global_scope_parent = this;
	this->has_own_scope = true;
	this->own_scope = new Scope(
		 "subcircuit_" + number2String(orig.id) + "_scope", orig.name , true );
	this->belonging_scope = own_scope;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

	// logger
	this->log_io = orig.log_io;

	deepCopyVectorOfPointers(orig.nodes, nodes);
	deepCopyOfChildren( orig.children );
	deepCopyVectorOfPointers(orig.parameters, parameters);
}

SubcircuitStatement* SubcircuitStatement::GetCopy() {
	return new SubcircuitStatement(*this);
}

SubcircuitStatement::~SubcircuitStatement() {
}

void SubcircuitStatement::AddProgeny( InstanceStatement* instance_of_subcircuit ){
	progeny.push_back( instance_of_subcircuit );
}

std::string SubcircuitStatement::ExportCircuitStatement( const std::string&  indentation ){
	// [inline] subckt SubcircuitName [(] node1 ... nodeN [)]
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
	std::string cs;
	if( is_inline ){
		cs = kEmptyLine + indentation + kCommentWord1 + name + kDelimiter + kDelimiter
			+ kInLineSubcircuitWord + kDelimiter + kSubcircuitWord;
		cs += kEmptyLine + indentation + kInLineSubcircuitWord + kDelimiter + kSubcircuitWord + kDelimiter + name + kDelimiter ;
	}else{
		cs = kEmptyLine + indentation + kCommentWord1 + name + kDelimiter + kSubcircuitWord;
		cs += kEmptyLine + indentation + kSubcircuitWord + kDelimiter + name + kDelimiter ;
	}
	if(has_brackets){
		cs += "( ";
	}
	//export nodes
	for(std::vector<Node*>::iterator it_node = nodes.begin() ; it_node !=  nodes.end(); it_node++){
		cs +=  (*it_node)->get_name() + kDelimiter;
	}
	if(has_brackets){
		cs += ")" + kEmptyLine ;
	}
	if(parameters.size() > 0){
		cs += kEmptyLine + kTab + kParametersWord + kDelimiter;
		//export parameters
		for(std::vector<Parameter*>::iterator it_parameter = parameters.begin();
	 	it_parameter !=  parameters.end(); it_parameter++){
			cs +=  kDelimiter + (*it_parameter)->ExportParameter();
		}
		//cs += kEmptyLine;
	}
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

void SubcircuitStatement::UpdateOwnScope(){
	own_scope->set_name( "subcircuit_" + number2String(id) + "_scope" );
	own_scope->set_main_statement_name( name );
}

bool SubcircuitStatement::ParseSubcircuitStatement(
		Statement& global_scope_parent, std::ifstream* file,
		std::vector<std::string>& lineTockens, std::string & statementCode,
		std::string& currentReadLine, int& statementCount, bool& endOfFile,
		bool& parsingSpectreCode, const bool permissiveParsingMode ){
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing subcircuit: '" + statementCode + "'" );
	#endif
	is_inline = false;
	bool childrensCompleted = true;
	correctly_parsed = false;
	set_id(statementCount++);
	//remove brakets from statementCode and add brackets in future
	boost::replace_all(statementCode, kParenthesisStartWord, kEmptyWord);
	boost::replace_all(statementCode, kParenthesisEndWord, kEmptyWord);
	set_has_brackets(true);
	//parse masterName
	boost::split(lineTockens, statementCode, boost::is_any_of(kDelimiter), boost::token_compress_on);
	set_name(lineTockens.at(1));
	set_master_name( name );
	// Test Unalterable
	set_unalterable( TestUnalterableStatement(master_name) );
	// update scope name
	UpdateOwnScope();
	//parse nodes (pins)
	// [name node1 ... nodeN ]
	if(lineTockens.size() > 3){
		for(std::vector<std::string>::iterator it_node = (++(++lineTockens.begin()));
			it_node != lineTockens.end(); it_node++){
			AddNode( *it_node, true );
		}
	}
	//parse the parameters and childrens
	statementCode = currentReadLine;
	currentReadLine = "";
	while(!correctly_parsed && getline(*file, currentReadLine)) {
		// ProcessLine
		if( ProcessLine( statementCode, currentReadLine, *this, statementCount, parsingSpectreCode ) ){
			continue;
		}else{
			if(statementCode.compare(kEmptyWord) != 0){
				// test end of subcircuit
				// ends
				if(statementCode.compare(kEndSubcircuitWord) == 0){
					correctly_parsed = true;
				} else if(boost::starts_with(statementCode, kEndSubcircuitAndDelimiterWord)){
					std::vector<std::string> ist;
					boost::split(ist, statementCode, boost::is_any_of(kDelimiter), boost::token_compress_on);
					// check syntax
					// ends subcircuitName
					if(ist.front().compare(kEndSubcircuitWord) == 0
						&& ist.at(1).compare(get_name()) == 0) {
						correctly_parsed = true;
					}else {
						log_io->ReportError2AllLogs( "Malformed netlist: subcircuit '"
							+ get_name() + "' does not end as expected." );
						log_io->ReportError2AllLogs( ist.at(1) + " vs " + get_name()
							);
						return false;
						// throw new std::exception();
					}
				}else{
					childrensCompleted = childrensCompleted
						&& ParseStatement(file, statementCode, *this, *this,
							currentReadLine, statementCount, endOfFile,
							parsingSpectreCode, permissiveParsingMode);
				}
			}
			// reset line Buffers
			statementCode = currentReadLine;
		}
	} //ends while
	if(!correctly_parsed){ //end of file
		if(statementCode.compare(kEmptyWord) != 0){
				// test end of subcircuit
				// ends
			if(statementCode.compare(kEndSubcircuitWord) == 0){
				correctly_parsed = true;
			} else if(boost::starts_with(statementCode, kEndSubcircuitAndDelimiterWord)){
				std::vector<std::string> ist;
				boost::split(ist, statementCode, boost::is_any_of(kDelimiter), boost::token_compress_on);
					// check syntax
					// ends subcircuitName
				if(ist.front().compare(kEndSubcircuitWord) == 0
					&& ist.at(1).compare(get_name()) == 0) {
					correctly_parsed = true;
				}else {
					log_io->ReportError2AllLogs( "Malformed netlist: subcircuit '"
					+ get_name() + "' does not end as expected." );
					log_io->ReportError2AllLogs( ist.at(1) + " vs " + get_name()
					);
					return false;
					// throw new std::exception();
				}
			} else {
				childrensCompleted = childrensCompleted
				&& ParseStatement(file, statementCode, *this, *this,
					currentReadLine, statementCount, endOfFile,
					parsingSpectreCode, permissiveParsingMode);
			}
		}
		endOfFile = true;
	}
	correctly_parsed = correctly_parsed && childrensCompleted;
	if(correctly_parsed){
		#ifdef PARSING_VERBOSE_MIN
			log_io->ReportPlain2Log( "subcircuit: '" + name + "', '" + master_name + "' parsed" );
		#endif
	}
	return correctly_parsed;
}

bool SubcircuitStatement::ParseInlineSubcircuitStatement(Statement& global_scope_parent,
	std::ifstream* file, std::vector<std::string>& lineTockens, std::string& statementCode,
	std::string& currentReadLine, int& statementCount, bool& endOfFile,
	bool& parsingSpectreCode, const bool permissiveParsingMode ){
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing inline subcircuit: '" + statementCode + "'" );
	#endif
	is_inline = true;
	bool childrensCompleted = true;
	correctly_parsed = false;
	set_id(statementCount++);
	//remove brakets from statementCode and add brackets in future
	boost::replace_all(statementCode, kParenthesisStartWord, kEmptyWord);
	boost::replace_all(statementCode, kParenthesisEndWord, kEmptyWord);
	set_has_brackets(true);
	//parse masterName
	boost::split(lineTockens, statementCode, boost::is_any_of(kDelimiter), boost::token_compress_on);
	set_name(lineTockens.at(2));
	set_master_name( name );
	// Test Unalterable
	set_unalterable( TestUnalterableStatement(master_name) );
	// update scope name
	UpdateOwnScope();
	//parse nodes (pins)
	// [name node1 ... nodeN ]
	if(lineTockens.size() > 4){
		for(std::vector<std::string>::iterator it_node = (++(++(++lineTockens.begin())));
			it_node != lineTockens.end(); it_node++){
			AddNode( *it_node, true );
		}
	}
	//parse the parameters and childrens
	statementCode = currentReadLine;
	currentReadLine = "";
	while(!correctly_parsed && getline(*file, currentReadLine)) {
		// ProcessLine
		if( ProcessLine(statementCode, currentReadLine, *this, statementCount, parsingSpectreCode) ){
			continue;
		}else{
			if(statementCode.compare(kEmptyWord) != 0){
				// test end of subcircuit
				//ends
				if(statementCode.compare(kEndSubcircuitWord) == 0){
					correctly_parsed = true;
				} else if(boost::starts_with(statementCode, kEndSubcircuitAndDelimiterWord)){
					std::vector<std::string> ist;
					boost::split(ist, statementCode, boost::is_any_of(kDelimiter), boost::token_compress_on);
					// check syntax
					//ends inline subckname
					if(ist.front().compare(kEndSubcircuitWord) == 0
						&& ist.at(1).compare(get_name()) == 0) {
						correctly_parsed = true;
					}else {
						log_io->ReportError2AllLogs( "Malformed netlist: inline subcircuit '"
							+ get_name() + "' does not end as expected." );
						log_io->ReportError2AllLogs( ist.at(1) + " vs " + get_name()
							);
						// throw new std::exception();
						return false;
					}
				}else{
					childrensCompleted = childrensCompleted
						&& ParseStatement(file, statementCode, *this, *this,
							currentReadLine, statementCount, endOfFile,
							parsingSpectreCode, permissiveParsingMode);
				}
			}
			// reset line Buffers
			statementCode = currentReadLine;
		}
	} //ends while
	if(!correctly_parsed){ //end of file
		if(statementCode.compare(kEmptyWord) != 0){
			// test end of subcircuit
			if(statementCode.compare(kEndSubcircuitWord) == 0){
				correctly_parsed = true;
			} else if(boost::starts_with(statementCode, kEndSubcircuitAndDelimiterWord)){
				std::vector<std::string> ist;
				boost::split(ist, statementCode, boost::is_any_of(kDelimiter), boost::token_compress_on);
				// check syntax
				if(ist.front().compare(kEndSubcircuitWord) == 0
					&& ist.at(1).compare(get_name()) == 0) {
					correctly_parsed = true;
				}else {
					log_io->ReportError2AllLogs( "Malformed netlist: inline subcircuit '"
						+ get_name() + "' does not end as expected." );
					log_io->ReportError2AllLogs( ist.at(1) + " vs " + get_name()
						);
					// throw new std::exception();
					return false;
				}
			}else{
				childrensCompleted = childrensCompleted
					&& ParseStatement(file, statementCode, *this, *this,
						currentReadLine, statementCount, endOfFile,
						parsingSpectreCode, permissiveParsingMode);
			}
		}
		endOfFile = true;
	}
	correctly_parsed = correctly_parsed && childrensCompleted;
	if(correctly_parsed){
		#ifdef PARSING_VERBOSE_MIN
			log_io->ReportPlain2Log( "inline subcircuit: '" + name + "' parsed" );
		#endif
	}
	return correctly_parsed;
}
