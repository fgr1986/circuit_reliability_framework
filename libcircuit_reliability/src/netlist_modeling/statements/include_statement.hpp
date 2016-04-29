/*
 * include_statement.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#ifndef INCLUDE_STATEMENT_H
#define INCLUDE_STATEMENT_H

#include <string>
#include <vector>

#include "statement.hpp"
 
class IncludeStatement : public Statement{
public:
	IncludeStatement();
	IncludeStatement(Statement* belonging_circuit,
		LogIO* log_io, Scope* belonging_scope);
	IncludeStatement(const IncludeStatement& orig);
	virtual ~IncludeStatement();
	bool get_scanned() const {return scanned; }
	void set_scanned(bool scanned) { this->scanned = scanned; } 
	bool get_has_section() const {return has_section; }
	void set_has_section(bool has_section) { this->has_section = has_section; } 
	bool get_export_canonical_path() const {return export_canonical_path; }
	void set_export_canonical_path(bool export_canonical_path) { this->export_canonical_path = export_canonical_path; } 
	int get_include_statement_type() const {return include_statement_type; }
	void set_include_statement_type(int include_statement_type) { this->include_statement_type = include_statement_type; }
	std::string get_section() const {return section; }
	void set_section(std::string section) { this->section = section; } 
	std::string get_refered_circuit_id() const {return refered_circuit_id; }
	void set_refered_circuit_id( std::string refered_circuit_id) { this->refered_circuit_id = refered_circuit_id; } 
	
	virtual IncludeStatement* GetCopy(); 
	virtual std::string ExportCircuitStatement(std::string indentation);

	void SetDefaultIncludeStatementType();
	bool ParseIncludeStatement( Statement& global_scope_parent, 
		std::vector<std::string>& lineTockens, std::string & readLine,
		int includeStatementType, int& statementCount);
	

private:
	bool scanned;
	bool has_section;
	int include_statement_type;
	std::string section;
	std::string refered_circuit_id;
	bool export_canonical_path;

};

#endif /* INCLUDE_STATEMENT_H */
