/**
 * @file xml_io_manager.cpp
 *
 * @date Created on: February 2, 2013
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * Main code of XMLIOManager Class (xml_io_manager.hpp)
 *
 */

// Boost includes
#include <boost/foreach.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/lexical_cast.hpp>
// for xml ptree
#include <boost/version.hpp>
// radiation simulator io includes
#include "xml_io_manager.hpp"

// Spectre Handler Library
// Constants includes
#include "global_functions_and_constants/global_constants.hpp"
#include "global_functions_and_constants/files_folders_io_constants.hpp"
#include "global_functions_and_constants/global_template_functions.hpp"
#include "global_functions_and_constants/statements_constants.hpp"
// Metric modeling includes
#include "metric_modeling/ocean_eval_metric.hpp"
#include "metric_modeling/magnitude.hpp"
// Netlist modeling includes
#include "netlist_modeling/parameter.hpp"
#include "netlist_modeling/simulation_parameter.hpp"
#include "netlist_modeling/statements/analysis_statement.hpp"
#include "netlist_modeling/statements/control_statement.hpp"
#include "netlist_modeling/statements/transistor_statement.hpp"
#include "netlist_modeling/statements/radiation_source_subcircuit_statement.hpp"
#include "netlist_modeling/statements/simple_statement.hpp"
#include "netlist_modeling/statements/instance_statement.hpp"
#include "netlist_modeling/statements/model_statement.hpp"
#include "netlist_modeling/statements/analog_model_statement.hpp"

XMLIOManager::XMLIOManager() {
}

XMLIOManager::~XMLIOManager() {
	#ifdef DESTRUCTORS_VERBOSE
		log_io->ReportPlain2Log( "XMLIOManager Destructor.Direction: " + number2String(this) );
	#endif
}

bool XMLIOManager::ReadExperimentEnvironmentFromXML(
	const std::string &xmlCadence, const std::string &xmlTechnology,
	const std::string &xmlExperiment, ExperimentEnvironment& experimentEnvironment,
	VariabilitySpectreHandler& variabilitySpectreHandler, SimulationModesHandler& simulationModesHandler,
	CircuitIOHandler& circuitIOHandler, std::string& experimentTitle ) {
	// populate tree structure pt
	//using boost::property_tree::ptree;
	int statementCounter = 0;
	if(!ReadCadenceXML( xmlCadence, statementCounter, experimentEnvironment, variabilitySpectreHandler )) {
 		log_io->ReportError2AllLogs( kTab + "An exception occurred parsing cadence xml conf file.");
		return false;
	}
	if(!ReadTechnologyXML( xmlTechnology, statementCounter, experimentEnvironment, variabilitySpectreHandler )){
 		log_io->ReportError2AllLogs( kTab + "An exception occurred parsing xmlTechnology conf file.");
		return false;
	}
	if(!ReadExperimentXML( xmlExperiment, statementCounter, experimentEnvironment,
		variabilitySpectreHandler, simulationModesHandler, circuitIOHandler, experimentTitle ) ){
 		log_io->ReportError2AllLogs( kTab + "An exception occurred parsing experiment conf file.");
		return false;
	}
	return true;
}

bool XMLIOManager::ReadCadenceXML( const std::string &xmlCadence, int& statementCounter,
	ExperimentEnvironment& experimentEnvironment, VariabilitySpectreHandler& variabilitySpectreHandler ){
	// Cadence conf file
	boost::property_tree::ptree ptCadence;
	log_io->ReportGreenStandard( kTab +  kLongDelimiter );
	log_io->ReportGreenStandard( kTab + "-> Reading Cadence XML conf file: " + xmlCadence );
	log_io->ReportGreenStandard( kTab +  kLongDelimiter );
	try
	{
		read_xml(xmlCadence, ptCadence);
		log_io->ReportPlain2Log( kTab + "Cadence XML conf file read. Structures creation." );
		// Version
		log_io->ReportPlainStandard( kTab + "Cadence file writen for radiation_simulator version: " + ptCadence.get<std::string>("root.radiation_simulator_target_version") );
		// Simulator version
		log_io->ReportPlain2Log( kTab + "Works with Cadence: " + ptCadence.get<std::string>("root.cadence_version") );
		// Spectre run commands
		boost::property_tree::ptree ptSpectreCommand = ptCadence.get_child("root.spectre_run_command");
		log_io->ReportPlainStandard( kTab + "Spectre example command-line run: '" + ptSpectreCommand.get<std::string>("spectre_example_command") + "'");
		log_io->ReportPlain2Log( kTab + "set_spectre_command_log_arg: '" + ptSpectreCommand.get<std::string>("spectre_command_log_arg") + "'" );
		log_io->ReportPlain2Log( kTab + "set_pre_spectre_command: '" + ptSpectreCommand.get<std::string>("pre_spectre_command") + "'" );
		log_io->ReportPlain2Log( kTab + "set_spectre_command_folder_arg: '" + ptSpectreCommand.get<std::string>("spectre_command_folder_arg") + "'" );
		log_io->ReportPlain2Log( kTab + "set_spectre_command: '" + ptSpectreCommand.get<std::string>("spectre_command") + "'" );
		log_io->ReportPlain2Log( kTab + "Setting above spectre command args");
		variabilitySpectreHandler.set_pre_spectre_command( ptSpectreCommand.get<std::string>("pre_spectre_command") );
		variabilitySpectreHandler.set_spectre_command( ptSpectreCommand.get<std::string>("spectre_command") );
		variabilitySpectreHandler.set_spectre_command_log_arg( ptSpectreCommand.get<std::string>("spectre_command_log_arg") );
		variabilitySpectreHandler.set_spectre_command_folder_arg( ptSpectreCommand.get<std::string>("spectre_command_folder_arg") );
		variabilitySpectreHandler.set_post_spectre_command( ptSpectreCommand.get<std::string>("post_spectre_command") );
		// Concurrent parallel instances
		// no max_parallel_scenario_instances for variability
		int maxMontecarloParallelInstances = ptCadence.get<int>("root.max_parallel_montecarlo_instances");
		int maxParallelProfileInstances = ptCadence.get<int>("root.max_parallel_profile_instances");
		if(  maxParallelProfileInstances<1 || maxMontecarloParallelInstances<1  ){
			log_io->ReportError2AllLogs( kTab + "max_parallel_scenario_instances, max_parallel_montecarlo_instances or max_parallel_profile_instances are lower than 1.");
			return false;
		}else{
			log_io->ReportCyanStandard( kTab + "max_parallel_montecarlo_instances: " + number2String(maxMontecarloParallelInstances));
			log_io->ReportCyanStandard( kTab + "max_parallel_profile_instances: " + number2String(maxParallelProfileInstances));
		}
		variabilitySpectreHandler.set_max_parallel_montecarlo_instances( maxMontecarloParallelInstances );
		variabilitySpectreHandler.set_max_parallel_profile_instances( maxParallelProfileInstances );
		log_io->ReportPlainStandard( kTab + "Parsing: " );
		// Primitive names
		log_io->ReportPlainStandard( k2Tab + "Primitive names. " );
		boost::property_tree::ptree pPrimitives = ptCadence.get_child("root.primitives");
		int primitivesCount = pPrimitives.count("primitive");
		if( primitivesCount>0 ){
			for( auto const & v : pPrimitives ){
				if ( boost::iequals( v.first, "primitive" ) ){
					experimentEnvironment.AddPrimitiveName(v.second.get<std::string>("primitive_name"));
				}
			}
		}else{
			log_io->ReportError2AllLogs("No primitives found in cadence_conf xml file.");
			return false;
		}
		//analysis
		log_io->ReportPlainStandard( k2Tab + "Analysis Statements. " );
		boost::property_tree::ptree pAnalysis = ptCadence.get_child("root.analyses");
		int analysisCount = pAnalysis.count("statement");
		if( analysisCount>0 ){
			for( auto const & v : pAnalysis ){
				if ( boost::iequals( v.first, "statement" ) ){
					ProcessEnvironmentAnalysisStatement( v, experimentEnvironment, statementCounter );
				}
			}
		}else{
			log_io->ReportError2AllLogs("No analysis found in cadence_conf xml file.");
			return false;
		}
		//control statements
		log_io->ReportPlainStandard( k2Tab + "Control Statements. " );
		boost::property_tree::ptree pControl = ptCadence.get_child("root.control_statements");
		int controlStatementsCount = pControl.count("statement");
		if( controlStatementsCount > 0 ){
			for( auto const & v : pControl ){
				if ( boost::iequals( v.first, "statement" ) ){
					ProcessEnvironmentControlStatement( v, experimentEnvironment, statementCounter );
				}
			}
		}else{
			log_io->ReportError2AllLogs("No control statements found in cadence_conf xml file.");
			return false;
		}
		//keywords
		log_io->ReportPlainStandard( k2Tab + "Keywords. " );
		boost::property_tree::ptree pKeyWords = ptCadence.get_child("root.keywords");
		int keyWordsCount = pKeyWords.count("keyword");
		if( keyWordsCount>0){
			for( auto const & v : pKeyWords ){
				if ( boost::iequals( v.first, "keyword" ) ){
					experimentEnvironment.AddReservedWord(v.second.get<std::string>("keyword_name"));
				}
			}
		}else{
			log_io->ReportError2AllLogs("No keywords found in cadence_conf xml file.");
			return false;
		}

		log_io->ReportPlainStandard( kTab + "Resume: " );
		log_io->ReportPlainStandard( k2Tab + number2String( primitivesCount ) + " primitives read" );
		log_io->ReportPlainStandard( k2Tab + number2String( analysisCount ) + " analysis types read" );
		log_io->ReportPlainStandard( k2Tab + number2String( controlStatementsCount ) + " control statements types read" );
		log_io->ReportPlainStandard( k2Tab + number2String( keyWordsCount ) + " keywords types read" );
	} catch (std::exception const& ex) {
 		log_io->ReportError2AllLogs( kTab + "An exception occurred parsing cadence xml conf file: " + ex.what() );
		return false;
	}
	return true;
}

bool XMLIOManager::ReadTechnologyXML( const std::string &xmlTechnology, int& statementCounter,
	ExperimentEnvironment& experimentEnvironment, VariabilitySpectreHandler& variabilitySpectreHandler ){
	// Technology
	boost::property_tree::ptree ptTechnology;
	log_io->ReportGreenStandard( kTab +  kLongDelimiter );
	log_io->ReportGreenStandard( kTab + "-> Reading technology XML conf file: " + xmlTechnology );
	log_io->ReportGreenStandard( kTab +  kLongDelimiter );
	try
	{
		read_xml(xmlTechnology, ptTechnology);
		log_io->ReportPlain2Log( kTab + "XML readed. Structures creation." );
		// technology
		log_io->ReportPlainStandard( kTab + "Technology: " + ptTechnology.get<std::string>("root.technology") );
		experimentEnvironment.set_technology_name( ptTechnology.get<std::string>("root.technology") );
		log_io->ReportPlainStandard( kTab + "Technology file writen for radiation_simulator version: "
			+ ptTechnology.get<std::string>("root.radiation_simulator_target_version") );
		// path to top models and section handling
		experimentEnvironment.set_technology_models_file_path( ptTechnology.get<std::string>("root.technology_models_file_path") );
		experimentEnvironment.set_technology_considering_sections( ptTechnology.get<bool>("root.technology_considering_sections") );
		if( experimentEnvironment.get_technology_considering_sections() ){
			experimentEnvironment.set_technology_golden_section( ptTechnology.get<std::string>("root.technology_golden_section") );
			experimentEnvironment.set_technology_experiment_section( ptTechnology.get<std::string>("root.technology_experiment_section") );
		}
		// Excluded folders
		// [Optional]
		log_io->ReportPlainStandard( k2Tab + "Excluded folders [Optional].");
		boost::property_tree::ptree exFolders = ptTechnology.get_child("root.excluded_folders");
		int excludedFoldersCount = exFolders.count("excluded_folder");
		if( excludedFoldersCount > 0 ) {
			for( auto const & v : exFolders ){
				if ( boost::iequals( v.first, "excluded_folder" ) ){
					experimentEnvironment.AddExcludedCanonicalFolder(v.second.get<std::string>("excluded_folder_path"));
					log_io->ReportPlainStandard( k2Tab + "Excluded folder: " + v.second.get<std::string>("excluded_folder_path"));
				}
			}
		}
		//Transistor statements
		// [Optional]
		log_io->ReportPlain2Log( k2Tab + "Technology transistors.[Optional]" );
		boost::property_tree::ptree pTransistors = ptTechnology.get_child("root.transistors");
		int transistorCount = pTransistors.count("statement");
		if( transistorCount ){
			for( auto const & v : pTransistors ){
				if ( boost::iequals( v.first, "statement" ) ){
					TransistorStatement* p = new TransistorStatement();
					p->set_id(statementCounter);
					p->set_name(kNotDefinedString);
					p->set_master_name(v.second.get<std::string>("master_name"));
					p->set_description(v.second.get<std::string>("description"));
					log_io->ReportPlain2Log( k3Tab + "-> Found transistor statement '"
						+ p->get_master_name() + "' " );
					experimentEnvironment.AddTransistorStatementType(p);
					statementCounter++;
				}
			}
		}

		// Technology Unalterable nodes
		// [Optional]
		log_io->ReportPlainStandard( k2Tab + "Technology Unalterable nodes. " );
		boost::property_tree::ptree techUNodes = ptTechnology.get_child("root.tech_unalterable_nodes");
		int techUNodesCount = techUNodes.count("node");
		if( techUNodesCount > 0 ) {
			for( auto const & v : techUNodes ){
				if ( boost::iequals( v.first, "node" ) ){
					experimentEnvironment.AddUnalterableNode(v.second.get<std::string>("name"));
					log_io->ReportPlain2Log( k3Tab + "-> Found Technology Unalterable node: '"
						+ v.second.get<std::string>("name") + "'" );
				}
			}
		}
		log_io->ReportPlainStandard( kTab + "Resume: " );
		log_io->ReportPlainStandard( k2Tab + number2String(excludedFoldersCount ) + " excluded_folders read" );
		log_io->ReportPlainStandard( k2Tab + number2String( transistorCount ) + " Technology transistors read" );
		log_io->ReportPlainStandard( k2Tab + number2String( techUNodesCount ) + " Unalterable nodes read" );
	} catch (std::exception const& ex) {
		log_io->ReportError2AllLogs( kTab + "An exception occurred parsing technology xml conf file: " + ex.what() );
		return false;
	}
	return true;
}

bool XMLIOManager::ReadExperimentXML( const std::string &xmlExperiment, int& statementCounter,
	ExperimentEnvironment& experimentEnvironment, VariabilitySpectreHandler& variabilitySpectreHandler,
	SimulationModesHandler& simulationModesHandler, CircuitIOHandler& circuitIOHandler,
	std::string& experimentTitle ){
	// Experiment
	boost::property_tree::ptree ptExperiment;
	log_io->ReportGreenStandard( kTab +  kLongDelimiter );
	log_io->ReportGreenStandard( kTab + "-> Reading Experiment XML conf file: " + xmlExperiment );
	log_io->ReportGreenStandard( kTab +  kLongDelimiter );
	log_io->ReportPlainStandard( kTab + "[Optional*] fields requiere at least one of them.");
	try {
		read_xml(xmlExperiment, ptExperiment);
		log_io->ReportPlain2Log( kTab + "XML readed. Structures creation." );
		// experiment metadata
		experimentTitle = ptExperiment.get<std::string>("root.experiment_title");
		log_io->ReportPlainStandard( kTab + "Experiment title: " + experimentTitle );
		log_io->ReportPlainStandard( kTab + "Experiment file writen for radiation_simulator version: "
			+ ptExperiment.get<std::string>("root.radiation_simulator_target_version") );
		// Additional labels: **delete_spectre_folders delete_spectre_transients delete_processed_transients**
		// delete_spectre_folders
		variabilitySpectreHandler.set_delete_spectre_folders( ptExperiment.get<bool>("root.delete_spectre_folders") );
		log_io->ReportPlainStandard( variabilitySpectreHandler.get_delete_spectre_folders() ?
			kTab + "Experiment will delete cadence spectre files." :
			kTab + "Experiment will not delete cadence spectre files." );
		// delete_spectre_transients
		variabilitySpectreHandler.set_delete_spectre_transients( ptExperiment.get<bool>("root.delete_spectre_transients") );
		log_io->ReportPlainStandard( variabilitySpectreHandler.get_delete_spectre_transients() ?
			kTab + "Experiment will save individually cadence transients." :
			kTab + "Experiment will not individually save cadence transients." );
		// delete_processed_transients
		variabilitySpectreHandler.set_delete_processed_transients( ptExperiment.get<bool>("root.delete_processed_transients") );
		log_io->ReportPlainStandard( variabilitySpectreHandler.get_delete_processed_transients() ?
			kTab + "Experiment will save individually processed transients." :
			kTab + "Experiment will not individually save processed transients." );
		variabilitySpectreHandler.set_export_processed_metrics( ptExperiment.get<bool>("root.export_processed_metrics") );
		if(variabilitySpectreHandler.get_export_processed_metrics()){
			log_io->ReportPlainStandard( kTab + "Experiment will export_processed_metrics.");
		}else{
			log_io->ReportPlainStandard( kTab + "Experiment will not export_processed_metrics.");
		}
		variabilitySpectreHandler.set_plot_scatters( ptExperiment.get<bool>("root.plot_scatters") );
		if(variabilitySpectreHandler.get_plot_scatters()){
			log_io->ReportPlainStandard( kTab + "Experiment will plot all the scatters.");
		}else{
			log_io->ReportPlainStandard( kTab + "Experiment will not plot all the scatters.");
		}
		variabilitySpectreHandler.set_plot_transients( ptExperiment.get<bool>("root.plot_transients") );
		if(variabilitySpectreHandler.get_plot_transients()){
			log_io->ReportPlainStandard( kTab + "Experiment will plot all the transients.");
		}else{
			log_io->ReportPlainStandard( kTab + "Experiment will not plot all the transients.");
		}
		// last plot
		if( ptExperiment.get<bool>("root.plot_last_transients") ){
			log_io->ReportPlainStandard( kTab + "Experiment will plot last transient." );
			variabilitySpectreHandler.set_plot_last_transients( ptExperiment.get<bool>("root.plot_last_transients") );
		}else{
			variabilitySpectreHandler.set_plot_last_transients( false );
		}
		variabilitySpectreHandler.set_export_metric_errors( ptExperiment.get<bool>("root.export_metric_errors") );
		// Interpolation ratio
		variabilitySpectreHandler.set_interpolate_plots_ratio( ptExperiment.get<int>("root.interpolate_plots_ratio") );
		if(variabilitySpectreHandler.get_interpolate_plots_ratio()){
			log_io->ReportPlainStandard( kTab + "Experiment will generate single transients reports");
		}else{
			log_io->ReportPlainStandard( kTab + "Experiment will not generate single transients reports.");
		}
		// Any analysis found

		// Analysis mode
		bool simulationAnalysisFound = false;
		boost::property_tree::ptree pAvailableAnalysis = ptExperiment.get_child( "root.simulation_modes_analysis");
		log_io->ReportPlainStandard( kTab + "Montecarlo & Sweep Parameter Simulation Mode. XML entry:" + kMontecarloNDParametersSweepModeWord );
		if( !ProcessSimulationMode( pAvailableAnalysis, kMontecarloNDParametersSweepModeWord,
			*simulationModesHandler.get_montecarlo_nd_parameters_sweep_mode(), statementCounter, simulationAnalysisFound ) ){
			log_io->ReportError2AllLogs( "Error processing Montecarlo 2d sweep analysis in experiment conf file.");
			return false;
		}
		// other analysis....
		if( !simulationAnalysisFound ){
			log_io->ReportError2AllLogs( "No simulation analysis has been found in experiment conf file.");
			return false;
		}
		// Processed metrics
		log_io->ReportPlainStandard( kTab + "Processed Metrics");
		boost::property_tree::ptree pMetrics = ptExperiment.get_child("root.processed_metrics");
		int metricCount = pMetrics.count("metric");
		if( metricCount > 0 ) {
			log_io->ReportPlainStandard( kTab + "Metrics to be processed: " + number2String(metricCount));
			for( auto const & v : pMetrics ){
				if ( boost::iequals( v.first, "metric" ) ){
					if( !ProcessMetric( v, statementCounter, variabilitySpectreHandler, circuitIOHandler )) {
						log_io->ReportError2AllLogs( "Error parsing metric in experiment conf file.");
						return false;
					}
				}
			}
		}else{
			log_io->ReportError2AllLogs( kTab + "No metrics to be processed, please, re-write experiment xml file." );
			return false;
		}
		// [Optional]
		// Simulation Parameters
		boost::property_tree::ptree ptSimulationParameters = ptExperiment.get_child("root.simulation_parameters");
		int simulationParameterNumber =  ptSimulationParameters.count("simulation_parameter");
		log_io->ReportGreenStandard( kTab + "Total simParameters: " + number2String(simulationParameterNumber));
		if( simulationParameterNumber > 0 ){
			for( auto const & v : ptSimulationParameters ){
				if ( boost::iequals( v.first, "simulation_parameter" ) ){
					if( !ProcessSimulationParameter( v, variabilitySpectreHandler ) ){
						log_io->ReportError2AllLogs( "Error Parsing simulation parameter.");
					}
				}
			}
			log_io->ReportGreenStandard( kTab + "Total parameters added: "
				+ number2String(variabilitySpectreHandler.get_simulation_parameters()->size()));

		}else{
			log_io->ReportRedStandard( "No simulation parameters found.");
		}
	} catch (std::exception const& ex) {
		log_io->ReportError2AllLogs( kTab + "An exception occurred parsing experiment xml conf file: " + ex.what() );
		return false;
	}
	return true;
}

bool XMLIOManager::ProcessSimulationMode( boost::property_tree::ptree pAvailableAnalysis,
	std::string simulationModeName, SimulationMode& simulationMode, int& statementCounter, bool& simulationAnalysisFound ){
	// pAvailableAnalysis = "root.simulation_modes_analysis"
	if( pAvailableAnalysis.count( simulationModeName ) ){
		boost::property_tree::ptree pAnalysisContainer = pAvailableAnalysis.get_child( simulationModeName );
		log_io->ReportPlainStandard( kTab + "Processing " + simulationModeName);
		for( auto const & analysis : pAnalysisContainer.get_child("analysis_statement") ){
			if ( boost::iequals( analysis.first, "statement" )) {
				if( !ProcessSimulationModeAnalysisStatement( analysis, simulationMode, statementCounter) ){
					log_io->ReportError2AllLogs( "Error parsing " + simulationModeName + " analysis in experiment conf file.");
					return false;
				}
				simulationAnalysisFound = true;
			}
		}
		// [Optional] Control statements
		if( pAnalysisContainer.count("control_statements") ){
			for( auto const & stdCS : pAnalysisContainer.get_child("control_statements") ){
				if ( boost::iequals( stdCS.first, "statement" )) {
					if( !ProcessSimulationModeControlStatement( stdCS, simulationMode, statementCounter)) {
						log_io->ReportError2AllLogs( "Error parsing "+ simulationModeName +" control statement in experiment conf file.");
						return false;
					}
				}
			}
		}
	}else{
		log_io->ReportRedStandard( kTab + "Analysis not found: " + simulationModeName);
	}
	return true;
}

bool XMLIOManager::ProcessSimulationModeAnalysisStatement(
	boost::property_tree::ptree::value_type const &v, SimulationMode& simulationMode, int& statementCounter ){
	// v is an "statement"
	AnalysisStatement* analysisStatement = new AnalysisStatement();
	analysisStatement->set_essential_analysis( true );
	analysisStatement->set_id( statementCounter );
	analysisStatement->set_master_name( v.second.get<std::string>("master_name") );
	analysisStatement->set_name( v.second.get<std::string>("name") );
	analysisStatement->set_description( v.second.get<std::string>("description") );
	analysisStatement->set_advanced_analysis( v.second.get<bool>("advanced_analysis") );
	analysisStatement->set_main_transient( v.second.get<bool>("main_transient") );
	if( analysisStatement->get_main_transient() ){
		simulationMode.set_main_transient_analysis( analysisStatement );
	}

	boost::property_tree::ptree pParameters = v.second.get_child("parameters");
	// the parameters are needed(such as stop time)
	int parametersCount =pParameters.count("parameter");
	if( parametersCount > 0 ) {
		for( auto const & vParam : pParameters ){
			if ( boost::iequals( vParam.first, "parameter" ) ){
				Parameter* parameter = new Parameter(
					vParam.second.get<std::string>("name"), vParam.second.get<std::string>("value") );
				analysisStatement->AddParameter( parameter );
				log_io->ReportPlain2Log( k2Tab + "-> Analysis parameter found " + parameter->get_name()
					+ " " + parameter->get_value() );
			}
		}
	}
	if( analysisStatement->get_advanced_analysis()
		&& v.second.count("children")>0 ){
		boost::property_tree::ptree pChildren = v.second.get_child("children");
		for( auto const & vChild : pChildren ){
			if ( boost::iequals( vChild.first, "statement" ) ){
				if( !ProcessSimulationModeChildAnalysisStatement(
					vChild, *analysisStatement, simulationMode, statementCounter ) ){
					log_io->ReportError2AllLogs( "Error parsing analysis children" );
					return false;
				}
			}
		}
	}

	simulationMode.set_analysis_statement( analysisStatement );

	log_io->ReportPlain2Log( k2Tab + "-> Found analysis " + analysisStatement->get_master_name()
		+ " " + (analysisStatement->get_advanced_analysis() ?  "(Complex)" : "(Simple)" ) );
	statementCounter++;
	return true;
}

bool XMLIOManager::ProcessSimulationModeChildAnalysisStatement(
	boost::property_tree::ptree::value_type const &v, AnalysisStatement& parent,
	SimulationMode& simulationMode, int& statementCounter ){
	// v is an "statement"
	AnalysisStatement* analysisStatement = new AnalysisStatement();
	analysisStatement->set_id( statementCounter );
	analysisStatement->set_master_name( v.second.get<std::string>("master_name") );
	analysisStatement->set_name( v.second.get<std::string>("name") );
	analysisStatement->set_description( v.second.get<std::string>("description") );
	analysisStatement->set_advanced_analysis( v.second.get<bool>("advanced_analysis") );
	analysisStatement->set_main_transient( v.second.get<bool>("main_transient") );
	if( analysisStatement->get_main_transient() ){
		simulationMode.set_main_transient_analysis( analysisStatement );
	}
	boost::property_tree::ptree pParameters = v.second.get_child("parameters");
	// the parameters are needed(such as stop time)
	int parametersCount =pParameters.count("parameter");
	if( parametersCount > 0 ) {
		for( auto const & vParam : pParameters ){
			if ( boost::iequals( vParam.first, "parameter" ) ){
				Parameter* parameter = new Parameter(
					vParam.second.get<std::string>("name"), vParam.second.get<std::string>("value") );
				analysisStatement->AddParameter( parameter );
				log_io->ReportPlain2Log( k2Tab + "-> Analysis parameter found " + parameter->get_name()
					+ " " + parameter->get_value() );
			}
		}
	}
	if( analysisStatement->get_advanced_analysis()
		&& v.second.count("children")>0 ){
		boost::property_tree::ptree pChildren = v.second.get_child("children");
		for( auto const & vChild : pChildren ){
			if ( boost::iequals( vChild.first, "statement" ) ){
				if( !ProcessSimulationModeChildAnalysisStatement(
					vChild, *analysisStatement, simulationMode, statementCounter ) ){
					log_io->ReportError2AllLogs( "Error parsing analysis children" );
					return false;
				}
			}
		}
	}

	parent.AddStatement( analysisStatement );

	log_io->ReportPlain2Log( k2Tab + "-> Found analysis " + analysisStatement->get_master_name()
		+ " " + (analysisStatement->get_advanced_analysis() ?  "(Complex)" : "(Simple)" ) );
	statementCounter++;
	return true;
}

bool XMLIOManager::ProcessSimulationModeControlStatement(
	boost::property_tree::ptree::value_type const &v,  SimulationMode& simulationMode, int& statementCounter ){

	ControlStatement* controlStatement = new ControlStatement();
	controlStatement->set_id(statementCounter);
	controlStatement->set_master_name(v.second.get<std::string>("master_name"));
	controlStatement->set_description(v.second.get<std::string>("description"));
	controlStatement->set_special_syntax_control_statement(
		v.second.get<bool>("special_syntax_control_statement") );
	controlStatement->set_advanced_control_statement(
		v.second.get<bool>("advanced_control_statement") );
	controlStatement->set_name(v.second.get<std::string>("name"));
	// the parameters are needed(such as stop time)
	if( v.second.count("parameters") > 0 ) {
		boost::property_tree::ptree pParameters = v.second.get_child("parameters");
		for( auto const & vParam : pParameters ){
			if ( boost::iequals( vParam.first, "parameter" ) ){
				Parameter* parameter = new Parameter(
					vParam.second.get<std::string>("name"), vParam.second.get<std::string>("value") );
				controlStatement->AddParameter( parameter );
				log_io->ReportPlain2Log( k2Tab + "-> Control statement parameter found " + parameter->get_name()
					+ " " + parameter->get_value() );
			}
		}
	}
	// children
	if( controlStatement->get_advanced_control_statement()
		&& v.second.count("children")>0 ){
		boost::property_tree::ptree pChildren = v.second.get_child("children");
		for( auto const & vChild : pChildren ){
			if ( boost::iequals( vChild.first, "statement" ) ){
				if( !ProcessSimulationModeChildControlStatement(
					vChild, *controlStatement, statementCounter ) ){
					log_io->ReportError2AllLogs( "Error parsing control statement children" );
					return false;
				}
			}
		}
	}
	simulationMode.AddControlStatement( controlStatement );
	log_io->ReportPlain2Log( k2Tab + "-> Found control statement " + controlStatement->get_master_name() );
	statementCounter++;
	return true;
}


bool XMLIOManager::ProcessSimulationModeChildControlStatement(
	boost::property_tree::ptree::value_type const &v,  Statement& parent, int& statementCounter ){

	ControlStatement* controlStatement = new ControlStatement();
	controlStatement->set_id(statementCounter);
	controlStatement->set_master_name(v.second.get<std::string>("master_name"));
	controlStatement->set_description(v.second.get<std::string>("description"));
	controlStatement->set_special_syntax_control_statement(
		v.second.get<bool>("special_syntax_control_statement") );
	controlStatement->set_advanced_control_statement(
		v.second.get<bool>("advanced_control_statement") );
	controlStatement->set_name(v.second.get<std::string>("name"));
	// the parameters are needed(such as stop time)
	if( v.second.count("parameters") > 0 ) {
		boost::property_tree::ptree pParameters = v.second.get_child("parameters");
		for( auto const & vParam : pParameters ){
			if ( boost::iequals( vParam.first, "parameter" ) ){
				Parameter* parameter = new Parameter(
					vParam.second.get<std::string>("name"), vParam.second.get<std::string>("value") );
				controlStatement->AddParameter( parameter );
				log_io->ReportPlain2Log( k2Tab + "-> Control statement parameter found " + parameter->get_name()
					+ " " + parameter->get_value() );
			}
		}
	}
	// children
	if( controlStatement->get_advanced_control_statement()
		&& v.second.count("children")>0 ){
		boost::property_tree::ptree pChildren = v.second.get_child("children");
		for( auto const & vChild : pChildren ){
			if ( boost::iequals( vChild.first, "statement" ) ){
				if( !ProcessSimulationModeChildControlStatement(
					vChild, *controlStatement, statementCounter ) ){
					log_io->ReportError2AllLogs( "Error parsing control statement children" );
					return false;
				}
			}
		}
	}
	parent.AddStatement( controlStatement );
	log_io->ReportPlain2Log( k2Tab + "-> Found control statement " + controlStatement->get_master_name() );
	statementCounter++;
	return true;
}

bool XMLIOManager::ProcessMetric(boost::property_tree::ptree::value_type const &v,
	int& statementCounter, VariabilitySpectreHandler& variabilitySpectreHandler, CircuitIOHandler& circuitIOHandler){
	Metric* newMetric;
	std::string metricName = v.second.get<std::string>("metric_name");
	log_io->ReportPlainStandard( k2Tab + "Processing metric: " + metricName );
	// new in v3.0.1
	if( v.second.get<bool>("metric_transient_magnitude") ){
	 newMetric = new Magnitude( metricName );
	}else{
		newMetric = new OceanEvalMetric( metricName );
	}
	// plottable
	if( newMetric->is_transient_magnitude() ){
		auto pMag = static_cast<Magnitude*>(newMetric);
		pMag->set_plottable( v.second.get<bool>("magnitude_plottable") );
		if( pMag->get_plottable() ){
			if( v.second.count("magnitude_plottable_in_golden") > 0){
				pMag->set_plottable_in_golden( v.second.get<bool>("magnitude_plottable_in_golden") );
			}else{
				pMag->set_plottable_in_golden( false );
			}
		}
	}
	// analyzable
	newMetric->set_analyzable( v.second.get<bool>("metric_analyzable") );
	if(newMetric->get_analyzable()){
		// standard voltage/current verilog-a signal
		if( newMetric->is_transient_magnitude() ){
			auto pMag = static_cast<Magnitude*>(newMetric);
			pMag->set_analyze_error_in_time( v.second.get<bool>("magnitude_analyze_error_in_time") );
			pMag->set_ommit_upper_threshold( v.second.get<bool>("magnitude_ommit_upper_threshold"));
			pMag->set_ommit_lower_threshold( v.second.get<bool>("magnitude_ommit_lower_threshold"));
			pMag->set_abs_error_margin_ones( v.second.get<double>("magnitude_abs_error_margin_ones") );
			pMag->set_abs_error_margin_zeros( v.second.get<double>("magnitude_abs_error_margin_zeros") );
			pMag->set_abs_error_margin_default( v.second.get<double>("magnitude_abs_error_margin_default") );
			if( pMag->get_abs_error_margin_ones() <= 0 || pMag->get_abs_error_margin_zeros() <= 0
				|| pMag->get_abs_error_margin_default() <= 0 ){
				log_io->ReportError2AllLogs("pMag abs error margins of magnitude '" + newMetric->get_name() + "' less or equal than '0'");
				return false;
			}
			pMag->set_error_threshold_ones( v.second.get<double>("magnitude_error_threshold_ones") );
			pMag->set_error_threshold_zeros( v.second.get<double>("magnitude_error_threshold_zeros") );
			if( pMag->get_error_threshold_ones() <= 0 || pMag->get_error_threshold_zeros() <= 0 ){
				log_io->ReportError2AllLogs("magnitude abs error margins of magnitude '" + pMag->get_name() + "' less or equal than '0'");
				return false;
			}
			if( pMag->get_analyze_error_in_time() ){
				pMag->set_error_time_span_zeros( v.second.get<double>("magnitude_error_time_span_zeros") );
				pMag->set_error_time_span_ones( v.second.get<double>("magnitude_error_time_span_ones") );
				pMag->set_error_time_span_default( v.second.get<double>("magnitude_error_time_span_default") );
				if( pMag->get_error_time_span_ones() <= 0 || pMag->get_error_time_span_zeros() <= 0
					|| pMag->get_error_time_span_default() <= 0 ){
					log_io->ReportError2AllLogs("magnitude_error_time_span of magnitude '" +pMag->get_name() + "' less or equal than '0'");
					return false;
				}
				// new functionallity analyze in time
				pMag->set_analyze_error_in_time_window( v.second.get<bool>("magnitude_analyze_error_in_time_window") );
				if( pMag->get_analyze_error_in_time_window() ){
					pMag->set_analyzable_time_window_t0( v.second.get<double>("magnitude_analyzable_time_window_t0") );
					pMag->set_analyzable_time_window_tf( v.second.get<double>("magnitude_analyzable_time_window_tf") );
				}
			}
		}else{ //oceanEval expresion
			auto pOceanEvalMag = static_cast<OceanEvalMetric*>(newMetric);
			pOceanEvalMag->set_abs_error_margin( v.second.get<double>("metric_abs_error_margin") );
			pOceanEvalMag->set_ocean_eval_expression( v.second.get<std::string>("metric_ocean_eval_expression") );
		}
	}
	variabilitySpectreHandler.AddMetric( newMetric );
	circuitIOHandler.AddMetric( newMetric );
	return true;
}

bool XMLIOManager::ProcessRadiationSource(boost::property_tree::ptree::value_type const &v,
	int& statementCounter, AlterationMode& radiationMode){
	// v.first="radiation_source"
	for( auto const & st : v.second ){
		if ( boost::iequals( st.first, "statement" ) ){
			RadiationSourceSubcircuitStatement* rsss = new RadiationSourceSubcircuitStatement();
			rsss->set_id(statementCounter);
			rsss->set_radiation_source_substitute_statement( !radiationMode.get_injection_mode() );
			rsss->set_name(st.second.get<std::string>("name"));
			rsss->set_master_name(st.second.get<std::string>("master_name"));
			if( st.second.count("parameters") > 0){
				for( auto const & param : st.second.get_child("parameters") ){
					if ( boost::iequals( param.first, "parameter" )) {
						rsss->AddParameter( new Parameter(
							param.second.get<std::string>("name"), param.second.get<std::string>("value")));
						log_io->ReportPlainStandard( k3Tab + "-> " + rsss->get_name() + " param: "
							+ param.second.get<std::string>("name") + " " + param.second.get<std::string>("value") );
					}
				}
			}
			if( st.second.count("nodes") > 0){
				for( auto const & node : st.second.get_child("nodes") ){
					if ( boost::iequals( node.first, "node" )) {
						rsss->AddLocalNode( new Node( node.second.get<std::string>("name"), false ));
					}
				}
			}
			if( st.second.count("children") > 0){
				for( auto const & child : st.second.get_child("children") ){
					if ( boost::iequals( child.first, "statement" )) {
						if( !ProcessSubcircuitChild( child, statementCounter, *rsss ) ){
							log_io->ReportError2AllLogs("Error processing radiation source child.");
							return false;
						}
					}
				}
			}
			log_io->ReportPlainStandard( k2Tab + "-> " + radiationMode.get_name() + " radiation mode has "
				+ number2String(radiationMode.get_available_radiation_sources()->size()) + " sources." );
			log_io->ReportPlainStandard( k2Tab + "-> Found radiation_source reference '" + rsss->get_name()
				+ "'. Source add to " + radiationMode.get_name() + " radiation mode." );
			radiationMode.AddRadiationSource( rsss );
			log_io->ReportPlainStandard( k2Tab + "-> " + radiationMode.get_name() + " radiation mode has "
				+ number2String(radiationMode.get_available_radiation_sources()->size()) + " sources." );
			statementCounter++;
		}
	}
	return true;
}

bool XMLIOManager::ProcessSubcircuitChild(boost::property_tree::ptree::value_type const &v,
	int& statementCounter, SubcircuitStatement& subcircuit){

	// v.first="statement"
	Statement* st;
	int statementType = v.second.get<int>("statement_type");
	bool error = false;
	switch( statementType ){
		case kSimpleStatement:{
			st = new SimpleStatement();
		}
		break;
		case kInstanceStatement:{
			st = new InstanceStatement();
		}
		break;
		case kTransistorStatement:{
			st = new TransistorStatement();
		}
		break;
		case kModelStatement:{
			st = new ModelStatement();
		}
		break;
		case kAnalogModelStatement:{
			st = new AnalogModelStatement();
		}
		break;
		case kSubcircuitStatement:{
			st = new SubcircuitStatement();
		}
		break;
		default:{
			error= true;
		}
		break;
	}
	if( error ){
		log_io->ReportError2AllLogs("Statement type not allowed processing radiation source child.");
		return false;
	}
	st->set_id(statementCounter);
	st->set_name(v.second.get<std::string>("name"));
	st->set_master_name(v.second.get<std::string>("master_name"));
	if( v.second.count("parameters") > 0){
		for( auto const & param : v.second.get_child("parameters") ){
			if ( boost::iequals( param.first, "parameter" )) {
				st->AddParameter( new Parameter(
					param.second.get<std::string>("name"), param.second.get<std::string>("value")));
				log_io->ReportPlainStandard( k3Tab + "-> " + st->get_name() + " param: "
					+ param.second.get<std::string>("name") + " " + param.second.get<std::string>("value") );
			}
		}
	}
	if( v.second.count("nodes") > 0){
		for( auto const & node : v.second.get_child("nodes") ){
			if ( boost::iequals( node.first, "node" )) {
				st->AddLocalNode( new Node( node.second.get<std::string>("name"), false ));
			}
		}
	}
	if( statementType == kSubcircuitStatement && v.second.count("children") > 0){
		for( auto const & child : v.second.get_child("children") ){
			if ( boost::iequals( child.first, "statement" )) {
				if( !ProcessSubcircuitChild( child, statementCounter, *dynamic_cast<SubcircuitStatement*>(st) ) ){
					log_io->ReportError2AllLogs("Error processing subcircuit child.");
					return false;
				}
			}
		}
	}
	log_io->ReportPlainStandard( k2Tab + "-> Found child '" + st->get_name() + "'" );
	subcircuit.AddStatement( st );
	statementCounter++;
	return true;
}

bool XMLIOManager::ProcessEnvironmentAnalysisStatement(
	boost::property_tree::ptree::value_type const &v,
	ExperimentEnvironment& experimentEnvironment, int& statementCounter ){

	AnalysisStatement* analysisStatement = new AnalysisStatement();
	analysisStatement->set_id( statementCounter );
	analysisStatement->set_master_name( v.second.get<std::string>("master_name") );
	analysisStatement->set_description( v.second.get<std::string>("description") );
	analysisStatement->set_advanced_analysis( v.second.get<bool>("advanced_analysis") );

	experimentEnvironment.AddAnalysisType( analysisStatement );
	log_io->ReportPlain2Log( k2Tab + "-> Found analysis " + analysisStatement->get_master_name()
		+ " " + (analysisStatement->get_advanced_analysis() ?  "(Complex)" : "(Simple)" ) );
	statementCounter++;
	return true;
}

bool XMLIOManager::ProcessEnvironmentControlStatement(
	boost::property_tree::ptree::value_type const &v,
	ExperimentEnvironment& experimentEnvironment, int& statementCounter ){
	ControlStatement* controlStatement = new ControlStatement();
	controlStatement->set_id(statementCounter);
	controlStatement->set_master_name(v.second.get<std::string>("master_name"));
	controlStatement->set_description(v.second.get<std::string>("description"));
	controlStatement->set_special_syntax_control_statement(
		v.second.get<bool>("special_syntax_control_statement") );
	controlStatement->set_advanced_control_statement(
		v.second.get<bool>("advanced_control_statement") );
	experimentEnvironment.AddControlStatementType( controlStatement );
	log_io->ReportPlain2Log( k2Tab + "-> Found control statement " + controlStatement->get_master_name() );
	statementCounter++;
	return true;
}

bool XMLIOManager::ProcessSimulationParameter(
	boost::property_tree::ptree::value_type const &vrp, VariabilitySpectreHandler& variabilitySpectreHandler ){

	SimulationParameter* rp = new SimulationParameter();
	rp->set_name( vrp.second.get<std::string>("simulation_parameter_name") );
	rp->set_value( vrp.second.get<std::string>("simulation_parameter_default_value") );
	rp->set_fixed( vrp.second.get<bool>("simulation_parameter_fixed") );
	rp->set_golden_fixed( vrp.second.get<bool>("simulation_parameter_golden_fixed") );

	log_io->ReportPlain2Log( k2Tab + "-> Found simulation_parameter '" + rp->get_name() + "' def_val:" + rp->get_value());
	if (!rp->get_fixed()){
		rp->set_allow_find_critical_value( vrp.second.get<bool>("simulation_parameter_allow_find_critical_value")  );
		log_io->ReportPlain2Log( k3Tab + "-> Parameter value is NOT fixed" );
		if(rp->get_allow_find_critical_value()){
			log_io->ReportRedStandard( k3Tab + "-> This program does not consider any parameter as critical. Use the other one!" );
			// variabilitySpectreHandler.set_critical_parameter( rp );
		}else{
			log_io->ReportPlain2Log( k3Tab + "-> Parameter does NOT allow_find_critical_value" );
		}
		rp->set_value_change_mode( vrp.second.get<int>("simulation_parameter_value_change_mode") );
		rp->set_value_max( vrp.second.get<double>("simulation_parameter_value_max") );
		rp->set_value_min( vrp.second.get<double>("simulation_parameter_value_min") );
		if( vrp.second.count("simulation_parameter_value_stop_percentage")>0 ){
			rp->set_value_stop_percentage( vrp.second.get<double>("simulation_parameter_value_stop_percentage") );
		}else{
			rp->set_value_stop_percentage(5.0);
		}
		if( vrp.second.count("simulation_parameter_sweep_steps")>0 ){
			rp->set_sweep_steps_number( vrp.second.get<double>("simulation_parameter_sweep_steps") );
		}else{
			rp->set_sweep_steps_number( 10 );
		}
	}else{
		log_io->ReportPlain2Log( k3Tab + "-> Parameter value is fixed" );
	}
	log_io->ReportPlain2Log( rp->GetInfo() );
	variabilitySpectreHandler.AddSimulationParameter(rp);
	return true;
}

bool XMLIOManager::ExportCircuit2XML( Statement& circuit,
	const std::string &filename, const std::string technology ) {
	using boost::property_tree::ptree;

	bool success;
	ptree ptCadence;

	ptCadence.add("circuit", "");
	ptCadence.add("circuit.technology", technology);

	ptree & node_statements = ptCadence.add("circuit.statements", "");
	success = ExportStatement2XML(node_statements, &circuit);
	// writing the unchanged ptree in output_files
	//////////////////
	// fgarcia hack
	//////////////////
	#if BOOST_VERSION>=105600
		boost::property_tree::write_xml(
			filename, ptCadence, std::locale(),
			// libboost >= 1.56
			boost::property_tree::xml_parser::xml_writer_make_settings<std::string>('\t', 1) );
	#else
		boost::property_tree::write_xml(
			filename, ptCadence, std::locale(),
			// libboost <= 1.55
			boost::property_tree::xml_writer_settings<char>('\t', 1) );
	#endif
	return success;
}

bool XMLIOManager::ExportStatement2XML( boost::property_tree::ptree& node_parent, Statement* ins ){
	using boost::property_tree::ptree;
	#ifdef XML_EXPORT_VERBOSE
		log_io->ReportPlain2Log( "export2xml: '" + ins->get_name() + "' '"
			+ ins->get_master_name() + "' '" + ins->get_raw_content() );
	#endif

	bool success = true;
	// create statement_node
	ptree & node_statement = node_parent.add( "statement" , "");
	// main characteristics
	node_statement.put("id", ins->get_id());
	node_statement.put("statement_type", ins->get_statement_type());
	node_statement.put("statement_type_description", ins->get_statement_type_description());
	node_statement.put("master_name", ins->get_master_name());
	node_statement.put("name", ins->get_name());
	node_statement.put("description", ins->get_description());
	node_statement.put("raw_content", ins->get_raw_content());
	node_statement.put("parent_id", ins->get_parent()->get_id());

	// export parameters if any
	if( ins->get_parameters()->size() > 0 ){
		ptree & node_parameters = node_statement.add("parameters", "");
		for(std::vector<Parameter*>::iterator it_parameter = ins->get_parameters()->begin();
				it_parameter != ins->get_parameters()->end(); it_parameter++){
			ptree & node_parameter = node_parameters.add("parameter", "");
			//node_parameter.put("parameter_type", pp->get_parameter_type() );
			node_parameter.put("name", (*it_parameter)->get_name() );
			node_parameter.put("value", (*it_parameter)->get_value() );
		}
	}
	// export nodes if any
	if( ins->get_nodes()->size() > 0 ){
		ptree & node_nodes = node_statement.add("nodes", "");
		for(std::vector<Node*>::iterator it_node = ins->get_nodes()->begin();
				it_node != ins->get_nodes()->end(); it_node++){
			ptree & node_node = node_nodes.add("node", "");
			// node_node.put("position", nn->get_position() );
			node_node.put("injected", (*it_node)->get_injected() );
			node_node.put("name", (*it_node)->get_name() );
		}
	}
	// export children if any
	if( ins->get_children()->size() > 0 ){
		ptree & node_children = node_statement.add("children", "");
		for (std::vector<Statement*>::iterator it_child = ins->get_children()->begin() ; it_child != ins->get_children()->end(); it_child++){
			success = success && ExportStatement2XML( node_children, *it_child );
		}
	}
	return true;
}
