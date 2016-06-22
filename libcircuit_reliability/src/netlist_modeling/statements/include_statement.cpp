/*
 * include_statement.cpp
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#include <boost/algorithm/string.hpp>

#include "include_statement.hpp"

#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"
#include "../../global_functions_and_constants/include_statement_constants.hpp"
#include "../../global_functions_and_constants/netlist_words_constants.hpp"


IncludeStatement::IncludeStatement() {
	this->id = kNotDefinedInt;
	this->refered_circuit_id = kNotDefinedString;
	this->statement_type = kIncludeStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->has_parallel_statements = false;
	this->correctly_parsed = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = false;
	this->has_nodes = false;
	this->has_children = false;
	this->has_parameters = false;
	this->has_raw_content = true;
	this->raw_content = kNotDefinedString;
	this->scanned = false;
	this->has_section= false;
	this->section = kNotDefinedString;
	this->include_statement_type = kNotDefinedInt;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->statement_type_description = kIncludeStatementDesc;
	this->export_canonical_path = false;
	// scope
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

}
IncludeStatement::IncludeStatement(Statement* belonging_circuit,
		LogIO* log_io, Scope* belonging_scope) {
	this->id = kNotDefinedInt;
	this->refered_circuit_id = kNotDefinedString;
	this->statement_type = kIncludeStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->has_parallel_statements = false;
	this->correctly_parsed = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = false;
	this->has_nodes = false;
	this->has_children = false;
	this->has_parameters = false;
	this->has_raw_content = true;
	this->raw_content = kNotDefinedString;
	this->scanned = false;
	this->has_section= false;
	this->section = kNotDefinedString;
	this->include_statement_type = kNotDefinedInt;
	// Radiation Properties
	this->unalterable = true;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	this->belonging_circuit = belonging_circuit;
	this->statement_type_description = kIncludeStatementDesc;
	this->export_canonical_path = false;
	// scope
	this->belonging_scope = belonging_scope;
	this->has_own_scope = false;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

	//logger
	this->log_io = log_io;
}

IncludeStatement::IncludeStatement(const IncludeStatement& orig) {
	this->id = orig.id;
	this->refered_circuit_id = orig.refered_circuit_id;
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
	this->scanned = orig.scanned;
	this->has_section= orig.has_section;
	this->include_statement_type = orig.include_statement_type;
	this->section = orig.section;
	this->can_be_substituted = orig.can_be_substituted;
	this->substitute_master_name = orig.substitute_master_name;
	this->unalterable = orig.unalterable;
	this->altered = orig.altered;
	this->can_be_injected = orig.can_be_injected;
	this->belonging_circuit = orig.belonging_circuit;
	this->statement_type_description = kIncludeStatementDesc;
	this->export_canonical_path = orig.export_canonical_path;
	this->path_to_belonging_circuit = orig.path_to_belonging_circuit;
	// scope
	// changed after deepCopy
	this->belonging_scope = orig.belonging_scope;
	this->has_own_scope = orig.has_own_scope;
	// Dependency
	this->consider_instances_dependency = false;
	this->scanned_for_instances_dependency = true;

	//logger
	this->log_io = orig.log_io;
}

IncludeStatement* IncludeStatement::GetCopy() {
	return new IncludeStatement(*this);
}

IncludeStatement::~IncludeStatement() {
}

void IncludeStatement::SetDefaultIncludeStatementType(){
	this->include_statement_type = kIncludeStatementType;
}

std::string IncludeStatement::ExportCircuitStatement( const std::string& indentation ){

	//include "file" section=Name
	std::string cs;
	cs += indentation + kCommentWord1 + " Including parsed model. Original: '"
		+ raw_content + "'" + kEmptyLine;
	cs += indentation;
	switch (include_statement_type){
		case kIncludeStatementType:{
			cs += kIncludeStatementWord;
		}
		break;
		case kIncludeAnalogIfStatementType:{
			cs += kIncludeAnalogIfStatementWord;
		}
		break;
		case kIncludeVCDStatementType:{
			cs += kIncludeVCDStatementWord;
		}
		break;
		case kIncludeEVCDStatementType:{
			cs += kIncludeVectorStatementWord;
		}
		break;
		case kIncludeVectorStatementType:{
			cs += kIncludeVectorStatementWord;
		}
		break;
		case kIncludeTechnologyModelStatementType:{
			cs += kIncludeTechnologyModelStatementWord;
		}break;
		default:{
			cs += kIncludeStatementWord;
		}
		break;
	}
	 if( !export_canonical_path
		&& include_statement_type== kIncludeStatementType ){
		cs += kDelimiter + kQuotes + refered_circuit_id
			+ "_netlist" + kDot + kSCS + kQuotes;
	} else {
		cs += kDelimiter + kQuotes + raw_content + kQuotes;
	}
	if( has_section ){
		cs += kDelimiter + kSectionWord + kEqualsWord + section;
	}
	cs += kEmptyLine;
	return cs;
}

bool IncludeStatement::ParseIncludeStatement( Statement& global_scope_parent,
	std::vector<std::string>& lineTockens, std::string & statementCode,
	int includeStatementType, int& statementCount){

	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing include statement: '" + statementCode + "'" );
	#endif
	//include "file" section=Name
	std::string nameNodesMasterNameLine = statementCode;
	// parse file
	set_id(statementCount++);
	this->global_scope_parent = &global_scope_parent;
	boost::replace_all( lineTockens.at(1), kQuotes, kEmptyWord );
	set_raw_content( lineTockens.at(1) );
	set_master_name("include_statement");
	set_scanned(false);
	set_include_statement_type(includeStatementType);
	if(lineTockens.size()>2){
		set_has_section(true);
		boost::split(lineTockens, statementCode, boost::is_any_of(kEqualsWord), boost::token_compress_on);
		set_has_section(true);
		set_section(lineTockens.back());
	}
	correctly_parsed = true;
	if(correctly_parsed){
		#ifdef PARSING_VERBOSE_MIN
			log_io->ReportPlain2Log( "include statement parsed: '" + statementCode + "'" );
		#endif
	}
	return correctly_parsed;
}
