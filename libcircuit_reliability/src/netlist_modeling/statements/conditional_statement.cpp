/*
 * user_defined_function_statement.cpp
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

#include "conditional_statement.hpp"

#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"
#include "../../global_functions_and_constants/netlist_words_constants.hpp"
#include "../../global_functions_and_constants/conditional_statement_constants.hpp"

ConditionalStatement::ConditionalStatement() {
	this->id = kNotDefinedInt;
	this->statement_type = kConditionalStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->has_parallel_statements = false;
	this->correctly_parsed = false;
	this->name = kNotDefinedString;
	this->master_name = kConditionalStatementDesc;
	this->description = kConditionalStatementDesc;
	this->has_brackets = true;
	this->has_nodes = false;
	this->has_children = true;
	this->has_parameters = false;
	this->has_raw_content = false;
	this->raw_content = kNotDefinedString;
	this->conditional_statement_type = kNotDefinedInt;
	this->condition = kNotDefinedString;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kConditionalStatementDesc;
	// scope
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

}

ConditionalStatement::ConditionalStatement(Statement* belonging_circuit,
		LogIO* log_io, Scope* belonging_scope) {
	this->id = kNotDefinedInt;
	this->statement_type = kConditionalStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->has_parallel_statements = false;
	this->correctly_parsed = false;
	this->name = kNotDefinedString;
	this->master_name = kConditionalStatementDesc;
	this->description = kConditionalStatementDesc;
	this->has_brackets = true;
	this->has_nodes = false;
	this->has_children = true;
	this->has_parameters = false;
	this->has_raw_content = false;
	this->raw_content = kNotDefinedString;
	this->conditional_statement_type = kNotDefinedInt;
	this->condition = kNotDefinedString;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kConditionalStatementDesc;
	this->belonging_circuit = belonging_circuit;
	// scope
	// change after deepCopyVectorOfPointers
	this->belonging_scope = belonging_scope;
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

	//logger manager
	this->log_io = log_io;
}

ConditionalStatement::ConditionalStatement(const ConditionalStatement& orig) {
	// log_io->ReportPlain2Log( kTab + "ConditionalStatement copy constructor " + orig.name + " " + orig.master_name );
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
	this->conditional_statement_type = orig.conditional_statement_type;
	this->condition = orig.condition;
	this->unalterable = orig.unalterable;
	this->can_be_substituted = orig.can_be_substituted;
	this->substitute_master_name = orig.substitute_master_name;
	this->altered = orig.altered;
	this->can_be_injected = orig.can_be_injected;
	this->belonging_circuit = orig.belonging_circuit;
	this->statement_type_description = kConditionalStatementDesc;
	this->path_to_belonging_circuit = orig.path_to_belonging_circuit;
	// scope
	// change after deepCopyVectorOfPointers
	this->belonging_scope = orig.belonging_scope;
	this->has_own_scope = orig.has_own_scope;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

	//logger manager
	this->log_io = orig.log_io;
	deepCopyOfChildren( orig.children );
}

ConditionalStatement* ConditionalStatement::GetCopy() {
	return new ConditionalStatement(*this);
}

ConditionalStatement::~ConditionalStatement() {
}

std::string ConditionalStatement::ExportCircuitStatement( const std::string& indentation ){
	//if (statement) {
	//	raw_content
	//}

	std::string cs = indentation;
	switch (conditional_statement_type){
		case kIfStatementType:
			cs += kIfStatementWord1
			+ kDelimiter + kParenthesisStartWord
			+ kDelimiter + condition + kDelimiter + kParenthesisEndWord;
		break;
		case kElseIfStatementType:
			cs += kElseIfStatementWord1
			+ kDelimiter + kParenthesisStartWord
			+ kDelimiter + condition + kDelimiter + kParenthesisEndWord;
		break;
		case kElseStatementType:
			cs += kElseStatementWord1;
		break;
	}
	cs += kBracketsStartWord + kEmptyLine;

	//export children
	if(children.size() > 0){
		for(std::vector<Statement*>::iterator it_children = children.begin();
			it_children !=  children.end(); it_children++){
			cs += (*it_children)->ExportCircuitStatement(indentation + kTab) + kEmptyLine;
		}
	}

	cs += indentation + kBracketsEndWord;
	return cs;
}

bool ConditionalStatement::ParseConditionalStatement(
	Statement& global_scope_parent, std::ifstream* file, std::vector<std::string>& lineTockens,
	std::string & statementCode, Statement& parent, std::string& currentReadLine,
	int conditionalStatementType, int& statementCount, bool& endOfFile,
	bool& parsingSpectreCode, const bool permissiveParsingMode ){
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing conditional function: '" + statementCode + "' "
			+ number2String( conditionalStatementType ) );
	#endif
	bool childrensCompleted = true;
	correctly_parsed = false;
	bool firstBracketRemoved = false;
	bool containsBrackets = false;
	std::string condition = "";
	this->global_scope_parent = &global_scope_parent;
	set_conditional_statement_type(conditionalStatementType);
	// process
	switch(conditionalStatementType){
		case kIfStatementType:
			description = kIfStatementWord1;
			boost::replace_first(statementCode, kIfStatementWord1, kEmptyWord);
		break;
		case kElseIfStatementType:
			description = kElseIfStatementWord1;
			boost::replace_first(statementCode, kElseIfStatementWord1, kEmptyWord);
		break;
		case kElseStatementType:
			description = kElseStatementWord1;
			boost::replace_first(statementCode, kElseStatementWord1, kEmptyWord);
		break;
	}
	containsBrackets= boost::contains(statementCode, kBracketsStartWord);

	boost::replace_first(statementCode, kParenthesisStartWord, kEmptyWord);
	boost::replace_last(statementCode, kParenthesisEndWord, kEmptyWord);
	if(containsBrackets){
		boost::replace_first(statementCode, kBracketsStartWord, kEmptyWord);
		firstBracketRemoved = true;
	}
	set_condition(statementCode);

	//parse the childrens
	statementCode = currentReadLine;
	currentReadLine = "";
	boost::algorithm::trim( statementCode );
	boost::replace_all( statementCode, kTab, kEmptyWord);
	if(!firstBracketRemoved && boost::starts_with(statementCode, kBracketsStartWord)){
		containsBrackets = true;
		boost::replace_first(statementCode, kBracketsStartWord, kEmptyWord);
		firstBracketRemoved = true;
	}
	// Add scope dependencies.
	// This way, the children can be scanned in order to
	// Find the global parent
	while(!correctly_parsed && getline(*file, currentReadLine)) {
		// ProcessLine
		if( ProcessLine(statementCode, currentReadLine, *this, statementCount, parsingSpectreCode) ){
			continue;
		}else if(containsBrackets){
			if(statementCode.compare(kEmptyWord) != 0){
				if(boost::starts_with(statementCode, kBracketsEndWord)){
					correctly_parsed = true;
					set_id(statementCount++);
					parent.AddStatement(this);
					this->set_parent( &parent );
					boost::replace_first(statementCode, kBracketsEndWord, kEmptyWord);
					ConditionalStatement* son = new ConditionalStatement( belonging_circuit, log_io, belonging_scope );
					if(boost::starts_with(statementCode, kElseIfStatementWord1)){
						son->ParseConditionalStatement( global_scope_parent, file, lineTockens,
							statementCode, parent, currentReadLine, kElseIfStatementType,
							statementCount, endOfFile,parsingSpectreCode, permissiveParsingMode);
					}else if(boost::starts_with(statementCode, kElseStatementWord1)){
						son->ParseConditionalStatement( global_scope_parent, file, lineTockens,
							statementCode, parent, currentReadLine, kElseStatementType, statementCount,
							endOfFile, parsingSpectreCode, permissiveParsingMode );
					}
				}else if(boost::starts_with(statementCode, kElseIfStatementWord1)){
					ConditionalStatement* son = new ConditionalStatement( belonging_circuit, log_io, belonging_scope );
					son->ParseConditionalStatement( global_scope_parent, file, lineTockens,
						statementCode, parent, currentReadLine, kElseIfStatementType, statementCount,
						endOfFile, parsingSpectreCode, permissiveParsingMode);
				}else if(boost::starts_with(statementCode, kElseStatementWord1)){
					ConditionalStatement* son = new ConditionalStatement( belonging_circuit, log_io, belonging_scope );
					son->ParseConditionalStatement( global_scope_parent, file, lineTockens,
						statementCode, parent, currentReadLine, kElseStatementType, statementCount,
						endOfFile, parsingSpectreCode, permissiveParsingMode);
				}else{
					childrensCompleted = childrensCompleted
						&& ParseStatement(file, statementCode, *this, global_scope_parent,
						currentReadLine, statementCount, endOfFile, parsingSpectreCode, permissiveParsingMode);
				}
			}
			// reset line Buffers
			statementCode = currentReadLine;
		} else{//ends else-if
			if(statementCode.compare(kEmptyWord) != 0){
				childrensCompleted = childrensCompleted
					&& ParseStatement(file, statementCode, *this,
						global_scope_parent, currentReadLine, statementCount, endOfFile,
						parsingSpectreCode, permissiveParsingMode);
				correctly_parsed = true;
				set_id(statementCount++);
				parent.AddStatement(this);
				this->set_parent( &parent );
			}
			// reset line Buffers
			statementCode = currentReadLine;
		}
	} //ends while
	// if we have reached the end of file
	if(!correctly_parsed){
		if(containsBrackets){
			if(statementCode.compare(kEmptyWord) != 0){
				// test end of user function
				if(boost::starts_with(statementCode, kBracketsEndWord)){
					ConditionalStatement* son = new ConditionalStatement( belonging_circuit, log_io, belonging_scope );
					correctly_parsed = true;
					set_id(statementCount++);
					parent.AddStatement(this);
					this->set_parent( &parent );
					boost::replace_first(statementCode, kBracketsEndWord, kEmptyWord);
					if(boost::starts_with(statementCode, kElseIfStatementWord1)){
						son->ParseConditionalStatement( global_scope_parent, file, lineTockens,
							statementCode, parent, currentReadLine, kElseIfStatementType, statementCount,
							endOfFile, parsingSpectreCode, permissiveParsingMode);
					}else if(boost::starts_with(statementCode, kElseStatementWord1)){
						son->ParseConditionalStatement( global_scope_parent, file, lineTockens,
							statementCode, parent, currentReadLine, kElseStatementType, statementCount,
							endOfFile, parsingSpectreCode, permissiveParsingMode);
					}
				}else if(boost::starts_with(statementCode, kElseIfStatementWord1)){
					ConditionalStatement* son = new ConditionalStatement( belonging_circuit, log_io, belonging_scope );
					son->ParseConditionalStatement( global_scope_parent, file, lineTockens,
						statementCode, parent, currentReadLine, kElseIfStatementType, statementCount,
						endOfFile, parsingSpectreCode, permissiveParsingMode);
				}else if(boost::starts_with(statementCode, kElseStatementWord1)){
					ConditionalStatement* son = new ConditionalStatement( belonging_circuit, log_io, belonging_scope );
					son->ParseConditionalStatement( global_scope_parent, file, lineTockens,
						statementCode, parent, currentReadLine, kElseStatementType, statementCount,
						endOfFile, parsingSpectreCode, permissiveParsingMode);
				}else{
					childrensCompleted = childrensCompleted
						&& ParseStatement(file, statementCode, *this,
							global_scope_parent, currentReadLine, statementCount, endOfFile,
							parsingSpectreCode, permissiveParsingMode);
				}
			}
			// reset line Buffers
			statementCode = currentReadLine;
		} else{//ends else-if

			if(statementCode.compare(kEmptyWord) != 0){
				childrensCompleted = childrensCompleted
					&& ParseStatement(file, statementCode, *this,
						global_scope_parent, currentReadLine, statementCount, endOfFile,
						parsingSpectreCode, permissiveParsingMode);
				correctly_parsed = true;
				set_id(statementCount++);
				parent.AddStatement(this);
				this->set_parent( &parent );
			}
			// reset line Buffers
			statementCode = currentReadLine;
		}
		endOfFile = true;
		statementCode = kEmptyWord;
	}
	if(correctly_parsed){
		#ifdef PARSING_VERBOSE_MIN
			log_io->ReportPlain2Log( "conditional statement." );
			switch(conditionalStatementType){
				case kIfStatementType:
					log_io->ReportPlain2Log( " IF conditional statement." );
				break;
				case kElseIfStatementType:
					log_io->ReportPlain2Log( " ELSE IF conditional statement." );
				break;
				case kElseStatementType:
					log_io->ReportPlain2Log( " ELSE conditional statement." );
				break;
			}
			log_io->ReportPlain2Log( "condition: '" + get_condition() + "' added" );
		#endif
	}
	return correctly_parsed;
}
