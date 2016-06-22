/*
 * subcircuit_statement.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#ifndef RADIATION_SOURCE_SUBCIRCUIT_STATEMENT_H
#define RADIATION_SOURCE_SUBCIRCUIT_STATEMENT_H

#include <istream>
#include <fstream>

#include "subcircuit_statement.hpp"
#include "../simulation_parameter.hpp"

class RadiationSourceSubcircuitStatement : public SubcircuitStatement{
public:
	RadiationSourceSubcircuitStatement();
	RadiationSourceSubcircuitStatement(Statement* belonging_circuit, LogIO* log_io, Scope* belonging_scope);
	RadiationSourceSubcircuitStatement( const RadiationSourceSubcircuitStatement& orig );
	virtual ~RadiationSourceSubcircuitStatement();

	virtual RadiationSourceSubcircuitStatement* GetCopy() override;
	virtual std::string ExportCircuitStatement( const std::string& indentation ) override;

	// void set_export_parameters( bool export_parameters ){ this->export_parameters = export_parameters; }
	// std::vector<Node*>* get_children_instance_nodes() {return &children_instance_nodes;}
	bool get_radiation_source_substitute_statement() { return radiation_source_substitute_statement; }
	void set_radiation_source_substitute_statement( bool radiation_source_substitute_statement ) {
		this->radiation_source_substitute_statement = radiation_source_substitute_statement; }

private:
	bool radiation_source_substitute_statement;

};

#endif /* RADIATION_SOURCE_SUBCIRCUIT_STATEMENT_H */
