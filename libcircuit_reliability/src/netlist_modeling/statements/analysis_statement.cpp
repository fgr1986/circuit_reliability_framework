/*
 * analysis_statement.cpp
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */


#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "analysis_statement.hpp"
#include "simple_statement.hpp"

#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/regex_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"
#include "../../global_functions_and_constants/netlist_words_constants.hpp"

AnalysisStatement::AnalysisStatement() {
	this->id = kNotDefinedInt;
	this->statement_type = kAnalysisStatement;
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
	this->mute_exportation = false;
	this->essential_analysis = false;
	this->main_transient = false;
	// Radiation properties
	this->unalterable = true;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->altered = false;
	this->substitute_master_name = kNotDefinedString;
	this->advanced_analysis = false;
	this->statement_type_description = kAnalysisStatementDesc;
	// scope
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;
}

AnalysisStatement::AnalysisStatement(Statement* belonging_circuit,
		LogIO* log_io, Scope* belonging_scope) {
	log_io->ReportPlain2Log("AnalysisStatement constructor");
	this->id = kNotDefinedInt;
	this->statement_type = kAnalysisStatement;
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
	this->mute_exportation = false;
	this->essential_analysis = false;
	this->main_transient = false;
	// Radiation properties
	this->unalterable = true;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->altered = false;
	this->substitute_master_name = kNotDefinedString;
	this->advanced_analysis = false;
	this->belonging_circuit = belonging_circuit;
	this->statement_type_description = kAnalysisStatementDesc;
	// scope
	this->belonging_scope = belonging_scope;
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;
	//logger manager
	this->log_io = log_io;
}

AnalysisStatement::AnalysisStatement(const AnalysisStatement& orig) {
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
	this->mute_exportation = orig.mute_exportation;
	this->essential_analysis = orig.essential_analysis;
	this->main_transient = orig.main_transient;
	this->path_to_belonging_circuit = orig.path_to_belonging_circuit;
	// Radiation properties
	this->unalterable = orig.unalterable;
	this->can_be_substituted = orig.can_be_substituted;
	this->substitute_master_name = orig.substitute_master_name;
	this->altered = orig.altered;
	this->can_be_injected = orig.can_be_injected;
	this->advanced_analysis = orig.advanced_analysis;
	this->belonging_circuit = orig.belonging_circuit;
	this->statement_type_description = kAnalysisStatementDesc;
	// scope
	this->belonging_scope = orig.belonging_scope;
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;
	//logger manager
	this->log_io = orig.log_io;
	deepCopyVectorOfPointers(orig.nodes, nodes);
	if(advanced_analysis){
		deepCopyOfChildren( orig.children );
	}
	deepCopyVectorOfPointers(orig.parameters, parameters);
}


AnalysisStatement* AnalysisStatement::GetCopy() {
	return new AnalysisStatement(*this);
}

AnalysisStatement::~AnalysisStatement() {
}

std::string AnalysisStatement::ExportCircuitStatement( const std::string& indentation){
	//Name [(]node1 ... nodeN[)] Analysis Type parameter=value
	std::string prefix = "";
	if ( mute_exportation ){
		prefix = kCommentWord1;
	}
	std::string cs = kEmptyLine + indentation + kCommentWord1
		+ " Start of " + name + " (" + master_name + ") analysis";
	cs += kEmptyLine + indentation + prefix + name + kDelimiter ;
	if(nodes.size() > 0){
		if(has_brackets){
			cs += "( ";
		}
		//export nodes
		for( auto const& n: nodes){
			cs +=  n->get_name() + kDelimiter;
		}
		if(has_brackets){
			cs += ") ";
		}
	}
	cs += master_name;
	//export parameters
	if( parameters.size() > 0 ){
		for( auto const& p: parameters){
			cs +=  kDelimiter + p->ExportParameter();
		}
	}
	if(advanced_analysis && children.size()>0 ){
		cs += kDelimiter + kBracketsStartWord;
		//export children
		for( auto const& s: children){
			if( s->get_statement_type()!=kSimpleStatement ){
				cs += kEmptyLine + prefix + s->ExportCircuitStatement( indentation + kTab );
			}else{
				auto pSS = static_cast<SimpleStatement*>(s);
				if( pSS->get_allows_mute() ){
					cs += kEmptyLine + prefix + pSS->ExportCircuitStatement( indentation + kTab );
				}else{ // no mute
					cs += kEmptyLine + pSS->ExportCircuitStatement( indentation + kTab );
				}
			}
		}
		cs += kEmptyLine + indentation + prefix + kBracketsEndWord;
	}
	cs += kEmptyLine + indentation + kCommentWord1
		+ " End of " + name + " (" + master_name + " ) analysis";
	cs += kEmptyLine;
	return cs;
}


bool AnalysisStatement::ParseAnalysisStatement(
		Statement& global_scope_parent, std::ifstream* file,
		std::vector<std::string>& lineTockens, std::string & statementCode,
		std::string& currentReadLine, int& statementCount, bool& endOfFile,
		bool& parsingSpectreCode, const bool permissiveParsingMode  ){

	//Name [(]node1 ... nodeN[)] Analysis Type [parameter=value ...] {asdfasdfa}
	//Name [(]node1 ... nodeN[)] Analysis Type [parameter=value ...] {
	//Name [(]node1 ... nodeN[)] Analysis Type [parameter=value ...]
	//Name [(]node1 ... nodeN[)] Analysis Type [parameter=value ...]
	// {
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing analysis: '" + statementCode + "'" );
	#endif
	bool childrenCorrectlyParsed = true;
	bool inlineChildren = false;
	correctly_parsed = false;
	bool firstBracketRemoved = false;
	std::string nameNodesMasterNameLine = statementCode;
	this->global_scope_parent = &global_scope_parent;
	// parse name
	set_id(statementCount++);
	set_name(lineTockens.front());
	// complex sentence addecuation
	if(boost::contains( statementCode, kBracketsStartWord )){
		//name sweep p1=v1... { bla bla bla }
		boost::regex ip_regex(kBracketedStatementRegEx);
		boost::match_results<std::string::const_iterator> results;
		if (boost::regex_search(statementCode, results, ip_regex))
		{
			std::string newStatement = results[0];
			#ifdef PARSING_VERBOSE
				log_io->ReportPlain2Log( kTab + "Parsing enclosed ({ child }):'" + newStatement  + "'" );
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
	boost::replace_all( statementCode, kTab, kEmptyWord );
	// parse parameters
	if(boost::contains( statementCode, kEqualsWord )){
		ParseParameters( statementCode, false );
	}

	//remove brakets from statementCode and add brackets in future
	boost::replace_first(statementCode, kParenthesisStartWord, kEmptyWord);
	boost::replace_last(statementCode, kParenthesisEndWord, kEmptyWord);
	set_has_brackets(true);
	//parse masterName
	boost::split(lineTockens, statementCode,
		boost::is_any_of(kDelimiter), boost::token_compress_on);
	set_master_name(lineTockens.back());
	set_advanced_analysis( TestIsAnalysisStatementAdvanced(master_name) );
	this->has_children = this->advanced_analysis;
	//parse nodes
	// [name node1 ... nodeN ]
	if(lineTockens.size() > 2){
		for(std::vector<std::string>::iterator it_node = (++lineTockens.begin());
			it_node != (--lineTockens.end()); it_node++){
			AddNode( *it_node );
		}
	}
	if(advanced_analysis && !inlineChildren){
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
					// test end of subcircuit
					if(boost::starts_with(statementCode, kBracketsEndWord)){
						correctly_parsed = true;
					}else{
						childrenCorrectlyParsed = childrenCorrectlyParsed
							&& ParseStatement(file, statementCode, *this, global_scope_parent,
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
				if(boost::starts_with(statementCode, kBracketsEndWord)){
					correctly_parsed = true;
				}else{
					childrenCorrectlyParsed = childrenCorrectlyParsed
						&& ParseStatement(file, statementCode, *this, global_scope_parent,
							currentReadLine, statementCount, endOfFile,
							parsingSpectreCode, permissiveParsingMode);
				}
			}
			endOfFile = true;
		}
	}

	correctly_parsed = childrenCorrectlyParsed;
	if(correctly_parsed){
		#ifdef PARSING_VERBOSE
			log_io->ReportPlain2Log( "END parsing analysis: (name, master_name) ('"
				+ name + "', '" + master_name + "' )" );
		#endif
	}
	return correctly_parsed;
}


/// Overriden in analysis statement
void AnalysisStatement::MuteAllNonEssentialAnalysis(){
	if( !essential_analysis ){
		this->mute_exportation = true;
	}
	if( children.size()>0 ){
		for (std::vector<Statement*>::iterator it_c = children.begin(); it_c != children.end(); it_c++){
			(*it_c)->MuteAllNonEssentialAnalysis();
		}
	}
}

/// Overridees statement
void AnalysisStatement::MuteNonMainTransientAnalysis(){
	if( !main_transient ){
		this->mute_exportation = true;
	}
	if( children.size()>0 ){
		for(std::vector<Statement*>::iterator it_c = children.begin();
			it_c !=  children.end(); it_c++){
			(*it_c)->MuteNonMainTransientAnalysis();
		}
	}
}
