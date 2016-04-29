 /**
 * @file simulation_mode.cpp
 *
 * @date Created on: March 25, 2014
 * 
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 * 
 * @section DESCRIPTION
 * 
 * Main code of HTMLIO Experiment (see simulation_mode.hpp)
 * 
 */

// Radiation simulator includes
#include "alteration_mode.hpp"
// Constants includes
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/global_template_functions.hpp"

AlterationMode::AlterationMode() {
	this->perform_alteration = false;
	this->injection_mode = true;
	this->inject_all_mode = true;
	this->name = kNotDefinedString;
}

AlterationMode::~AlterationMode() {	
	deleteContentsOfVectorOfPointers( available_radiation_sources );
}

void AlterationMode::AddRadiationSource( RadiationSourceSubcircuitStatement* radiationSource ){
	available_radiation_sources.push_back( radiationSource );
}

void AlterationMode::AddAlterableStatement( std::string alterable_statement,
	std::string new_statement ){	
	alterable_statements.insert(std::make_pair(alterable_statement,new_statement));
}

void AlterationMode::TestAlterationMode(){
	if( available_radiation_sources.size() < 0){
		log_io->ReportError2AllLogs( "[Warning] No available_radiation_sources in environment.");
	}
	if( alterable_statements.size() < 0){
		log_io->ReportError2AllLogs( "[Warning] No alterable_statements in environment.");
	}
}