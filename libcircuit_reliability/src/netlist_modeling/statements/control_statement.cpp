/*
 * control_statement.cpp
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */


#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string.hpp>
 
#include "control_statement.hpp"

#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/regex_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"
#include "../../global_functions_and_constants/netlist_words_constants.hpp"

ControlStatement::ControlStatement() {
	this->id = kNotDefinedInt;
	this->statement_type = kControlStatement;
	this->correctly_parsed = false;
	this->parallel_statements = kDefaultParallelStatements;
	this->has_parallel_statements = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = true;
	this->has_nodes = true;
	this->has_children = false;
	this->has_parameters = true;
	this->has_raw_content = false;
	this->raw_content = kNotDefinedString;
	// Special control statements
	this->param_set_case = false;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->advanced_control_statement = false;
	this->special_syntax_control_statement = false;
	this->statement_type_description = kControlStatementDesc;
	/// scope
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;
	
}

ControlStatement::ControlStatement(Statement* belonging_circuit, 
		LogIO* log_io, Scope* belonging_scope, bool special_syntax_control_statement) {
	this->id = kNotDefinedInt;
	this->statement_type = kControlStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->correctly_parsed = false;
	this->has_parallel_statements = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = true;
	this->has_nodes = true;
	this->has_children = false;
	this->has_parameters = true;
	this->has_raw_content = false;
	this->raw_content = kNotDefinedString;
	// Special control statements
	this->param_set_case = false;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->advanced_control_statement = false;
	this->special_syntax_control_statement = special_syntax_control_statement;
	this->belonging_circuit = belonging_circuit;
	this->statement_type_description = kControlStatementDesc;
	// scope
	this->belonging_scope = belonging_scope;
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;
	
	// logger
	this->log_io = log_io;
}

ControlStatement::ControlStatement(const ControlStatement& orig) {
	// log_io->ReportPlain2Log( kTab + "ControlStatement copy constructor " + orig.name + " " + orig.master_name );
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
	// Radiation Properties
	this->unalterable = orig.unalterable;
	this->altered = orig.altered;
	this->can_be_injected = orig.can_be_injected;
	this->can_be_substituted = orig.can_be_substituted;
	this->substitute_master_name = orig.substitute_master_name;
	this->advanced_control_statement = orig.advanced_control_statement;
	this->special_syntax_control_statement = orig.special_syntax_control_statement;
	this->belonging_circuit = orig.belonging_circuit;
	this->statement_type_description = kControlStatementDesc;
	this->path_to_belonging_circuit = orig.path_to_belonging_circuit;
	// Special control statements
	this->param_set_case = orig.param_set_case;
	// scope
	// changed after deep copy of pointers
	this->belonging_scope = orig.belonging_scope;
	this->has_own_scope = orig.has_own_scope;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;
	
	// logger
	this->log_io = log_io;
	deepCopyVectorOfPointers(orig.nodes, nodes);
	if(advanced_control_statement){
		deepCopyOfChildren( orig.children );
	}
	deepCopyVectorOfPointers(orig.parameters, parameters);	
}

ControlStatement* ControlStatement::GetCopy() {
	return new ControlStatement(*this);
}

ControlStatement::~ControlStatement() {
}

std::string ControlStatement::ExportCircuitStatement(std::string indentation){
	// save statement
	if( master_name.compare("save")==0 ){
		return indentation + master_name + kDelimiter + raw_content;
	}
	//Name [(]node1 ... nodeN[)] control_statement_Type parameter=value
	std::string cs;
	if(special_syntax_control_statement){
		cs = indentation + master_name;
		if( !advanced_control_statement ){
			cs += kDelimiter + name;
		}
	}else{
		cs = indentation + name + kDelimiter;
		if(nodes.size() > 0){
			if(has_brackets){
				cs += "( ";
			}
			//export nodes
			for(std::vector<Node*>::iterator it_node = nodes.begin() ; it_node !=  nodes.end(); it_node++){
				cs +=  (*it_node)->get_name() + kDelimiter;
			}
			if(has_brackets){
				cs += ") ";
			}
		}
		cs += master_name;
	}
	if(parameters.size() > 0){			
		//export parameters
		for(std::vector<Parameter*>::iterator it_parameter = parameters.begin();
			it_parameter !=  parameters.end(); it_parameter++){
			cs +=  kDelimiter + (*it_parameter)->ExportParameter();
		}
	}
	
	if( advanced_control_statement ){
		//export children
		if( children.size()>0 || param_set_case ){
			cs += kDelimiter + kBracketsStartWord;
			for(std::vector<Statement*>::iterator it_children = children.begin();
			 it_children !=  children.end(); it_children++){
				cs += kEmptyLine + (*it_children)->ExportCircuitStatement(indentation + kTab);
			}
			if( param_set_case ){
				cs += kEmptyLine + raw_content;
			}
			cs += kEmptyLine + indentation + kBracketsEndWord;
		}
	}
	
	#ifdef PARSING_VERBOSE
	cs += kEmptyLine + indentation + kCommentWord1
		+ " end of ( name: '" + name + "'' , master_name: '" + master_name + "' ) control_statement";
	#endif
	
	return cs;
}


bool ControlStatement::ParseControlStatement( Statement& global_scope_parent, std::ifstream* file,
		std::vector<std::string>& lineTockens, std::string & statementCode,
		std::string& currentReadLine, int& statementCount, bool& endOfFile,
		bool& parsingSpectreCode, const bool permissiveParsingMode ){

	//Name [(]node1 ... nodeN[)] control_statement_Type [parameter=value ...] {asdfasdfa} //1
	//Name [(]node1 ... nodeN[)] control_statement_Type [parameter=value ...] {
	//Name [(]node1 ... nodeN[)] control_statement_Type [parameter=value ...] 
	//Name [(]node1 ... nodeN[)] control_statement_Type [parameter=value ...] 
	// {

	// Special
	// master_name [raw] [parameter=value ... ]
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing control_statement: '" + statementCode + "'" );
	#endif
	bool childrenCorrectlyParsed = true;
	correctly_parsed = false;
	bool inlineChildren = false;
	bool firstBracketRemoved = false;
	bool parsingParamset = false;
	std::string paramSetRaw = "";
	set_id(statementCount++);
	this->global_scope_parent = &global_scope_parent;
	std::string nameNodesMasterNameLine = statementCode;
	if( special_syntax_control_statement ){
		// parse master_name
		set_master_name(lineTockens.front());
	}else{
		// parse name
		set_name(lineTockens.front());
	}
	// complex sentence addecuation (//1) 
	if(boost::contains( statementCode, kBracketsStartWord )){	
		//name sweep p1=v1... { bla bla bla }
		boost::regex ip_regex(kBracketedStatementRegEx);
		boost::match_results<std::string::const_iterator> results;
		if (boost::regex_search(statementCode, results, ip_regex))
		{
			std::string newStatement = results[0];
			#ifdef PARSING_VERBOSE
				log_io->ReportPlain2Log( kTab + "Parsing enclosed ({ child }):'" + newStatement + "'" );
			#endif
			boost::replace_first( newStatement, kBracketsStartWord, kEmptyWord );			
			boost::replace_last( newStatement, kBracketsEndWord, kEmptyWord );
			boost::algorithm::trim( newStatement ); //trim
			childrenCorrectlyParsed = ParseStatement( file, newStatement,
				*this, global_scope_parent, currentReadLine, statementCount, endOfFile,
				parsingSpectreCode, permissiveParsingMode);

			//remove parameters subcode in statementcode
			std::vector<std::string> statementTokens;
			boost::algorithm::split_regex( 
				statementTokens, statementCode, boost::regex( kBracketedStatementRegEx ) ) ;
			statementCode = statementTokens.front();
			inlineChildren = true;
			
		}else {		 //name sweep p1=v1... { 
			boost::replace_last( statementCode, kBracketsStartWord, kEmptyWord );	
		}
		firstBracketRemoved = true;
	}
	//trim post bracket removal
	boost::algorithm::trim( statementCode );
	boost::replace_all( statementCode, kTab, kEmptyWord);
	if( special_syntax_control_statement && statementCode.length() > master_name.length()+1 ){
		std::string raw = statementCode.substr( master_name.length() + 1 );
		#ifdef PARSING_VERBOSE
			log_io->ReportPlain2Log( kTab + "Raw content:'" + raw + "'" );
		#endif
		set_raw_content(raw);
	}else{
		// parse parameters
		if(boost::contains( statementCode, kEqualsWord )){
			ParseParameters( statementCode, false );
		}
		//remove parenthesis from statementCode and add brackets in future
		boost::replace_first(statementCode, kParenthesisStartWord, kEmptyWord);
		boost::replace_last(statementCode, kParenthesisEndWord, kEmptyWord);
		set_has_brackets(true);

		//parse without params
		boost::split(lineTockens, statementCode,
			boost::is_any_of(kDelimiter), boost::token_compress_on); 
		//parse masterName
		set_master_name(lineTockens.back());	
		parsingParamset = master_name.compare( "paramset" )==0;
		if( parsingParamset ){
			#ifdef PARSING_VERBOSE_MIN
				log_io->ReportPlain2Log( "Parsing a paramset control statement" );
			#endif
		}
		set_advanced_control_statement(
			TestIsControlStatementAdvanced(master_name) );
		//parse nodes
		// [name node1 ... nodeN ]
		if(lineTockens.size() > 2){
			for(std::vector<std::string>::iterator it_node = (++lineTockens.begin());
				it_node != (--lineTockens.end()); it_node++){
				AddNode( *it_node );
			}
		}	
	}
	//parse the childrens
	if(advanced_control_statement && !inlineChildren){
		statementCode = currentReadLine;
		currentReadLine = "";
		boost::algorithm::trim( statementCode );
		boost::replace_all( statementCode, kTab, kEmptyWord);
		if(!firstBracketRemoved && boost::starts_with(statementCode, kBracketsStartWord)){
			boost::replace_last( statementCode, kBracketsStartWord, kEmptyWord );	
			firstBracketRemoved = true;			
		}
		while(!correctly_parsed && getline(*file, currentReadLine)) {
			// ProcessLine
			if( ProcessLine(statementCode, currentReadLine, *this, statementCount, parsingSpectreCode) ){					
				continue;
			}else{
				if(statementCode.compare(kEmptyWord) != 0){
					// test end of control statement	
					if(boost::starts_with(statementCode, kBracketsEndWord)){	
						correctly_parsed = true;
					}else if( parsingParamset ){
						paramSetRaw += kEmptyLine + statementCode;
					}else{
						childrenCorrectlyParsed = childrenCorrectlyParsed
							&& ParseStatement(file, statementCode, *this, global_scope_parent,
								currentReadLine, statementCount, endOfFile, parsingSpectreCode, permissiveParsingMode);
					}
				}			
				// reset line Buffers	
				statementCode = currentReadLine;		
			}
		} //ends while
		if(!correctly_parsed){ //end of file
			if(statementCode.compare(kEmptyWord) != 0){
				// test end of control statement	
				if(boost::starts_with(statementCode, kBracketsEndWord)){	
					correctly_parsed = true;
				}else if( parsingParamset ){
					paramSetRaw += kEmptyLine + statementCode;
				}else{
					childrenCorrectlyParsed = childrenCorrectlyParsed
						&& ParseStatement(file, statementCode, *this, global_scope_parent,
							currentReadLine, statementCount, endOfFile, parsingSpectreCode, permissiveParsingMode);
				}
			}		
			endOfFile = true;
		}
	}
	if( parsingParamset ){
		param_set_case = true;
		set_raw_content( paramSetRaw );
	}
	correctly_parsed = childrenCorrectlyParsed;
	if(correctly_parsed){
		#ifdef PARSING_VERBOSE_MIN
			log_io->ReportPlain2Log( "END parsing control_statement: (name, master_name) ('"
				+ name + "', '" + master_name + "' )" );
		#endif
	}
	return correctly_parsed;
}
