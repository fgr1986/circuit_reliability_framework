/*
 * library_statement.cpp
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#include <boost/algorithm/string.hpp>
 
#include "section_statement.hpp"
 
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"
#include "../../global_functions_and_constants/netlist_words_constants.hpp"


SectionStatement::SectionStatement() {
	this->id = kNotDefinedInt;
	this->statement_type = kSectionStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->has_parallel_statements = false;
	this->correctly_parsed = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = false;
	this->has_nodes = false;
	this->has_children = true;
	this->has_parameters = false;
	this->has_raw_content = false;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kSectionStatementDesc;
	// scope
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;
	
}
SectionStatement::SectionStatement(Statement* belonging_circuit, 
		LogIO* log_io, Scope* belonging_scope) {
	this->id = kNotDefinedInt;
	this->statement_type = kSectionStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->has_parallel_statements = false;
	this->correctly_parsed = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = false;
	this->has_nodes = false;
	this->has_children = true;
	this->has_parameters = false;
	this->has_raw_content = false;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kSectionStatementDesc;
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

SectionStatement::SectionStatement(const SectionStatement& orig) {
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
	this->can_be_injected = orig.can_be_injected;
	this->altered = orig.altered;
	this->statement_type_description = kSectionStatementDesc;
	this->path_to_belonging_circuit = orig.path_to_belonging_circuit;
	this->belonging_circuit = orig.belonging_circuit;
	// scope
	this->belonging_scope = orig.belonging_scope;
	this->has_own_scope = orig.has_own_scope;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;
	
	// logger
	this->log_io = log_io;
	deepCopyOfChildren( orig.children );
}

SectionStatement* SectionStatement::GetCopy() {
	return new SectionStatement(*this);
}

SectionStatement::~SectionStatement() {
}

std::string SectionStatement::ExportCircuitStatement(std::string indentation){
	
	// section SectionName
	//    statements
	//     ...
	// section [SectionName]

	std::string cs;
	cs += indentation + kCommentWord1 + "Start of " + kSectionWord + kDelimiter + master_name;
	cs += kEmptyLine + indentation + kSectionWord + kDelimiter + master_name;

	//export children
	if(children.size() > 0){
		for(std::vector<Statement*>::iterator it_children = children.begin();
		 it_children !=  children.end(); it_children++){
			cs += kEmptyLine + (*it_children)->ExportCircuitStatement(indentation + kTab);
		}
	}
	cs += kEmptyLine + indentation + kEndSectionWord + kDelimiter + master_name;
	cs += kEmptyLine + indentation + kCommentWord1 + "End of " + kSectionWord + kDelimiter + master_name;
	return cs;
}

bool SectionStatement::ParseSectionStatement( Statement& global_scope_parent, std::ifstream* file,
	std::vector<std::string>& lineTockens, std::string & statementCode,
	std::string& currentReadLine, int& statementCount, bool& endOfFile,
	bool& parsingSpectreCode, const bool permissiveParsingMode ){
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing section: '" + statementCode + "'" );
	#endif
	bool childrensCompleted = true;
	correctly_parsed = false;
	set_id(statementCount++);
	this->global_scope_parent = &global_scope_parent;
	set_name(kStatementMasterName);
	set_master_name(lineTockens.at(1));
	// Test Unalterable
	set_unalterable( TestUnalterableStatement(master_name) );
	//parse and childrens
	statementCode = currentReadLine;
	currentReadLine = "";
	while(!correctly_parsed && getline(*file, currentReadLine)) {
		// ProcessLine
		if( ProcessLine(statementCode, currentReadLine, *this, statementCount, parsingSpectreCode) ){					
			continue;
		}else{
			if(statementCode.compare(kEmptyWord) != 0){
				// test end of section	
				boost::split(lineTockens, statementCode, boost::is_any_of(kDelimiter), boost::token_compress_on);	
				if(lineTockens.front().compare( kEndSectionWord) == 0){			
					correctly_parsed = true;
				}else{
					childrensCompleted = childrensCompleted
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
			// test end of section	
			boost::split(lineTockens, statementCode, boost::is_any_of(kDelimiter), boost::token_compress_on);	
			if(lineTockens.front().compare(kEndSectionWord) == 0){
				#ifdef PARSING_VERBOSE
					log_io->ReportPlain2Log( "EO Section: OK" );			
				#endif
				correctly_parsed = true;
			} else {
				childrensCompleted = childrensCompleted
					&& ParseStatement(file, statementCode, *this, global_scope_parent,
						currentReadLine, statementCount, endOfFile, parsingSpectreCode, permissiveParsingMode);
				 if( permissiveParsingMode ){
				 	#ifdef PARSING_VERBOSE
						log_io->ReportRedStandard( "End of file, unexpected end of Section: Forced by a permisive parsing mode." );
					#endif
					correctly_parsed = true;
				}
			}
		}else if( permissiveParsingMode ){
			#ifdef PARSING_VERBOSE
				log_io->ReportRedStandard( "2-> End of file, unexpected end of Section: Forced by a permisive parsing mode." );
			#endif
			correctly_parsed = true;
		}
		endOfFile = true;
	}

	correctly_parsed = correctly_parsed && childrensCompleted; 
	if(correctly_parsed){
		#ifdef PARSING_VERBOSE_MIN
			log_io->ReportPlain2Log( "section_statement: '" + name + "', '" + master_name + "' parsed" );
		#endif
	}
	return correctly_parsed;
}