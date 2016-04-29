/**
 * @file circuit_radiator.cpp
 *
 * @date Created on: Jan 20, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * Main code of CircuitExporter Class (see circuit_exporter.hpp)
 *
 */

// Radiation simulator includes
#include "circuit_exporter.hpp"

// Spectre Handler Library
#include "global_functions_and_constants/statements_constants.hpp"
#include "global_functions_and_constants/include_statement_constants.hpp"
#include "global_functions_and_constants/global_constants.hpp"
#include "global_functions_and_constants/global_template_functions.hpp"
#include "global_functions_and_constants/global_template_functions.hpp"
#include "global_functions_and_constants/files_folders_io_constants.hpp"


CircuitExporter::CircuitExporter() {
}

CircuitExporter::~CircuitExporter() {
	#ifdef DESTRUCTORS_VERBOSE
		log_io->ReportPlain2Log( "CircuitExporter Destructor. Direction: " + number2String(this) );
	#endif
}

bool CircuitExporter::SetUpScenarios( std::string goldenFolder,
		std::string goldenAhdlFolder, std::string variationsAHDLFolder, std::string variationsFolder ){
	// Export Golden netlist
	log_io->ReportPlainStandard( k3Tab + "Exporting golden netlist '" + main_circuit->get_name() + "'" );

	CircuitStatement* exportedGoldenCircuit = new CircuitStatement( *main_circuit );
	// models and section for golden
	exportedGoldenCircuit->AddIncludeStatementAndRegister( GetGoldenModelsAndSectionIncludeStatement() );
	// parameters
	exportedGoldenCircuit->AddIncludeStatementAndRegister( GetNewIncludeStatementOfParametersCircuit() );
	// mute analysis
	exportedGoldenCircuit->MuteNonMainTransientAnalysis();
	if( !circuit_io_handler->ExportSingularNetlist( goldenAhdlFolder , *exportedGoldenCircuit ) ){
		return false;
	}
	if( !circuit_io_handler->ExportSingularNetlist( goldenFolder , *exportedGoldenCircuit ) ){
		return false;
	}
	// Export Golden netlist
	log_io->ReportPlainStandard( k3Tab + "Exporting golden netlist '" + main_circuit->get_name() + "'" );
	// delete golden
	delete exportedGoldenCircuit;


	log_io->ReportPlainStandard( k3Tab + "Adding models include reference to circuits" );
	// add models
	main_circuit->AddIncludeStatementAndRegister( GetExperimentModelsAndSectionIncludeStatement() );

	// AHDL CIRCUIT AND PARAMETERS
	CircuitStatement* AHDLCircuit = new CircuitStatement( *main_circuit );
	AHDLCircuit->set_name( "ahdl_circuit" );
	// mute non transient analysis
	AHDLCircuit->MuteNonMainTransientAnalysis();
	// Parameters needed for AHDL AND radiation circuit
	for(std::vector<SimulationParameter*>::iterator
		it_p = variability_spectre_handler->get_simulation_parameters()->begin();
		it_p != variability_spectre_handler->get_simulation_parameters()->end(); it_p++){
			AHDLCircuit->AddParameter( new Parameter(**it_p) );
	}
	// Export ahdl netlist
	if( !circuit_io_handler->ExportSingularNetlist( variationsAHDLFolder , *AHDLCircuit ) ){
		return false;
	}
	delete AHDLCircuit;

	// Variations circuit, main circuit already has the models/sections
	CircuitStatement* variationsCircuit = new CircuitStatement( *main_circuit );
	// parameters reference
	variationsCircuit->AddIncludeStatementAndRegister( GetNewIncludeStatementOfParametersCircuit() );
	// Export variations netlist
	if( !circuit_io_handler->ExportSingularNetlist( variationsFolder , *variationsCircuit ) ){
		return false;
	}
	return true;
}

IncludeStatement* CircuitExporter::GetGoldenModelsAndSectionIncludeStatement(){
	IncludeStatement* include_subck = new IncludeStatement();
	include_subck->set_log_io( log_io );
	include_subck->SetDefaultIncludeStatementType();
	include_subck->set_raw_content( experiment_environment->get_technology_models_file_path() );
	include_subck->set_refered_circuit_id( experiment_environment->get_technology_models_file_path() );
	include_subck->set_include_statement_type( kIncludeTechnologyModelStatementType );
	if( experiment_environment->get_technology_considering_sections() ){
		include_subck->set_section( experiment_environment->get_technology_golden_section() );
	}
	include_subck->set_has_section( experiment_environment->get_technology_considering_sections()  );
	return include_subck;
}

IncludeStatement* CircuitExporter::GetExperimentModelsAndSectionIncludeStatement(){
	IncludeStatement* include_subck = new IncludeStatement();
	include_subck->set_log_io( log_io );
	include_subck->SetDefaultIncludeStatementType();
	include_subck->set_raw_content( experiment_environment->get_technology_models_file_path() );
	include_subck->set_refered_circuit_id( experiment_environment->get_technology_models_file_path() );
	include_subck->set_include_statement_type( kIncludeTechnologyModelStatementType );
	if( experiment_environment->get_technology_considering_sections() ){
		include_subck->set_section( experiment_environment->get_technology_experiment_section() );
	}
	include_subck->set_has_section( experiment_environment->get_technology_considering_sections()  );
	return include_subck;
}

IncludeStatement* CircuitExporter::GetNewIncludeStatementOfParametersCircuit( ){
	IncludeStatement* include_subck = new IncludeStatement();
	include_subck->set_log_io( log_io );
	include_subck->SetDefaultIncludeStatementType();
	include_subck->set_raw_content( kParametersCircuitStringId );
	include_subck->set_has_section( false );
	include_subck->set_refered_circuit_id( kParametersCircuitStringId );
	return include_subck;
}
