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
#include "simulation_mode.hpp"
// Constants includes
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/global_template_functions.hpp"

SimulationMode::SimulationMode() {
	this->id = kNotDefinedInt;
	this->alteration_mode = nullptr;
	this->analysis_statement = nullptr;
	this->main_transient_analysis = nullptr;
}

SimulationMode::~SimulationMode() {
	#ifdef DESTRUCTORS_VERBOSE
		log_io->ReportPlain2Log( "SimulationMode destructor: " + short_description + ".Direction: " + number2String(this) );
	#endif
	deleteContentsOfVectorOfPointers( control_statements );
	if( analysis_statement ){
		delete analysis_statement;
	}
}

void SimulationMode::AddControlStatement( ControlStatement* control_statement){
	control_statements.push_back(control_statement);
}

void SimulationMode::SelectRadiationSource( int index){
	if( index < (int)alteration_mode->get_available_radiation_sources()->size()){
		alteration_mode->set_selected_radiation_source_index( index );
	}
}
