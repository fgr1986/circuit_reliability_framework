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

#include "user_defined_function_statement.hpp"
 
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"
#include "../../global_functions_and_constants/netlist_words_constants.hpp"
 
UserDefinedFunctionStatement::UserDefinedFunctionStatement() {
	this->id = kNotDefinedInt;
	this->statement_type = kUserDefinedFunctionStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->has_parallel_statements = false;
	this->correctly_parsed = false;
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
	this->statement_type_description = kUserDefinedFunctionStatementDesc;
	// scope
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;
	
}


UserDefinedFunctionStatement::UserDefinedFunctionStatement(Statement* belonging_circuit,
		LogIO* log_io, Scope* belonging_scope) {
	this->id = kNotDefinedInt;
	this->statement_type = kUserDefinedFunctionStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->has_parallel_statements = false;
	this->correctly_parsed = false;
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
	this->statement_type_description = kUserDefinedFunctionStatementDesc;
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

UserDefinedFunctionStatement::UserDefinedFunctionStatement(const UserDefinedFunctionStatement& orig) {
	this->id = orig.id;
	this->statement_type = orig.statement_type;
	this->parallel_statements = orig.parallel_statements;
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
	this->statement_type_description = kUserDefinedFunctionStatementDesc;
	this->path_to_belonging_circuit = orig.path_to_belonging_circuit;
	this->belonging_circuit = orig.belonging_circuit;
	// scope
	// changed after deep copy
	this->belonging_scope = orig.belonging_scope;
	this->has_own_scope = has_own_scope;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;
	
	// logger
	this->log_io = log_io;
	deepCopyVectorOfPointers(orig.parameters, parameters);
}


UserDefinedFunctionStatement* UserDefinedFunctionStatement::GetCopy() {
	return new UserDefinedFunctionStatement(*this);
}

UserDefinedFunctionStatement::~UserDefinedFunctionStatement() {
}

std::string UserDefinedFunctionStatement::ExportCircuitStatement(std::string indentation){
	//real myfunc( real a, real b ) {
	//	return a+b*2+sqrt(a*sin(b));
	//}
	std::string cs;
	cs += indentation + kCommentWord1 + "User defined function " + name;
	cs += indentation + kSpectreUserDefinedFunctionsGuides;
	cs += kEmptyLine + indentation + kUserDefinedFunctionWord + kDelimiter + name ;
	cs += kParenthesisStartWord +  kDelimiter;
	if(parameters.size() > 0){
		//export parameters
		for(std::vector<Parameter*>::iterator it_param = parameters.begin() ; it_param !=  (--parameters.end()); it_param++){
			cs += kUserDefinedFunctionWord + kDelimiter + (*it_param)->get_value() + kArgumentDelimiter + kDelimiter;
		}		
	}
	cs += kUserDefinedFunctionWord + kDelimiter + parameters.back()->get_value() + kDelimiter;
	cs += kParenthesisEndWord + kBracketsStartWord;
	cs += kEmptyLine + indentation + kTab + raw_content;
	cs += kEmptyLine + indentation + kBracketsEndWord ;
	return cs;
}

bool UserDefinedFunctionStatement::ParseUserDefinedFunctionStatement( 
		Statement& global_scope_parent, std::ifstream* file, std::vector<std::string>& lineTockens,
		std::string & statementCode, std::string& currentReadLine,
		int& statementCount, bool& endOfFile,
		bool& parsingSpectreCode ){
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing user defined function: '" + statementCode + "'" );
	#endif
	bool firstBracketRemoved = false;
	correctly_parsed = false;
	std::string raw_content;
	this->global_scope_parent = &global_scope_parent;
	set_id(statementCount++);
	set_master_name( kStatementMasterName );
	// process user defined function name
	std::vector<std::string> udft;
	boost::algorithm::split_regex( udft, statementCode,
		boost::regex( kUserDefinedFunctionAndDelimiterWord ) ) ;
	
	boost::replace_last(udft.at(1), kParenthesisStartWord, kEmptyWord);
	set_name(udft.at(1));
	// process user defined function parameters
	if(udft.size() > 2){
		std::string arg = "";
		for(std::vector<std::string>::iterator it_arg = (++(++udft.begin()));
			it_arg != (--udft.end()); it_arg++){
			arg = *it_arg;
			boost::trim(arg);
			boost::replace_all(arg, kArgumentDelimiter, kEmptyWord);
			AddParameter(new Parameter(kArgumentWord, arg));
		}	
		arg = udft.back();
		boost::trim(arg);
		std::vector<std::string> udfat;
		boost::split(udfat, arg, boost::is_any_of(kParenthesisEndWord), boost::token_compress_on);
		AddParameter(new Parameter(kArgumentWord, udfat.front()));
	}

	//parse the raw_content
	statementCode = currentReadLine;
	if(!firstBracketRemoved && boost::starts_with(statementCode, kBracketsStartWord)){
		boost::replace_first(statementCode, kBracketsStartWord, kEmptyWord);
		firstBracketRemoved = true;
	}
	currentReadLine = "";
	while(!correctly_parsed && getline(*file, currentReadLine)) {
		// ProcessLine
		if(!firstBracketRemoved && boost::starts_with(currentReadLine, kBracketsStartWord)){
			boost::replace_first(currentReadLine, kBracketsStartWord, kEmptyWord);
			firstBracketRemoved = true;
		}
		if( ProcessLine(statementCode, currentReadLine, *this, statementCount, parsingSpectreCode) ){					
			continue;
		}else{
			if(statementCode.compare(kEmptyWord) != 0){
				// test end of user function	
				if(boost::starts_with(statementCode, kBracketsEndWord)){	
					set_raw_content(raw_content);
					correctly_parsed = true;
				}else{
					raw_content += statementCode;
				}
			}			
			// reset line Buffers	
			statementCode = currentReadLine;		
		}
	} //ends while
	if(!correctly_parsed){// end of file
		if(statementCode.compare(kEmptyWord) != 0){
			// test end of user function	
			if(boost::starts_with(statementCode, kBracketsEndWord)){	
				set_raw_content(raw_content);
				correctly_parsed = true;
			}else{
				raw_content += statementCode;
			}
		}
		endOfFile = true;
		statementCode = kEmptyWord;		
	} 
	if(correctly_parsed){
		#ifdef PARSING_VERBOSE_MIN
			log_io->ReportPlain2Log( "user_defined_function_statement: '" + name + "', '" + master_name + "' parsed" );
		#endif
	}
	return correctly_parsed;
}