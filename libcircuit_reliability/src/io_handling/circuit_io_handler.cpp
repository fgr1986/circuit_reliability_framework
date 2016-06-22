/**
 * @file circuit_io_handler.cpp
 *
 * @date Created on: Jan 20, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * Main code of CircuitIOHandler Class (see circuit_io_handler.hpp)
 *
 */

// Boost includes
//#define BOOST_NO_CXX11_SCOPED_ENUMS // needed for copy file
#include "boost/filesystem.hpp" // includes all needed Boost.Filesystem declarations
#include <boost/algorithm/string.hpp>
// radiation simulator includes
#include "circuit_io_handler.hpp"
#include "../metric_modeling/ocean_eval_metric.hpp"
#include "../netlist_modeling/statements/simple_statement.hpp"
// constants includes
#include "../global_functions_and_constants/global_template_functions.hpp"
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/files_folders_io_constants.hpp"


CircuitIOHandler::CircuitIOHandler() {
}

CircuitIOHandler::~CircuitIOHandler() {
	#ifdef DESTRUCTORS_VERBOSE
		log_io->ReportPlainStandard( "CircuitIOHandler destructor. direction:" + number2String(this));
	#endif
}

void CircuitIOHandler::AddSimulationSpecialStatements( CircuitStatement& circuit ){
	log_io->ReportPlain2Log( "Adding our analysis statement to the main circuit.");
	if( simulation_mode==nullptr ){
		log_io->ReportError2AllLogs( "Error in circuit_io_handler: simulation_mode is nullptr");
		return;
	}
	if( simulation_mode->get_analysis_statement()== nullptr ){
		log_io->ReportError2AllLogs( "Error in circuit_io_handler: simulation_mode->get_analysis_statement is nullptr");
		return;
	}
	// create analysis statement
	auto mainAnalysis = simulation_mode->get_analysis_statement();
	bool auxTranAnalysisIsMain = mainAnalysis==simulation_mode->get_main_transient_analysis();
	auto mainAnalysisCopy = new AnalysisStatement( *mainAnalysis );
	// add non_transient metrics
	// (oceanEval metrics)
	for( auto const& m : metrics ){
		if( !m->is_transient_magnitude() ){
			auto pOceanEvalMag = dynamic_cast<OceanEvalMetric*>( m );
			// no mute
			auto oceanEvalSimpleStatement =  new SimpleStatement( false,
				"export " + pOceanEvalMag->get_name() + "=oceanEval(\"" + pOceanEvalMag->get_ocean_eval_expression() + "\")");
			// for transient analyses ( it reads logs for oceanEvals )
			if( auxTranAnalysisIsMain ){
				circuit.AddSimulationSpecialStatement( oceanEvalSimpleStatement );
			}else{ // for montecarlo analyses
				mainAnalysisCopy->AddStatement( oceanEvalSimpleStatement );
			}
		}
	}
	// add analysis statement
	circuit.AddSimulationSpecialStatement( mainAnalysisCopy );
	// add control statems
	for( auto const& cs : *simulation_mode->get_control_statements() ){
			circuit.AddSimulationSpecialStatement( new ControlStatement( *cs ));
	}
	log_io->ReportPlain2Log( "Analysis statement added.");
	// save spectre metrics
	for( auto const& m : metrics ){
		if( m->is_transient_magnitude() && m->get_name().compare("time")!=0 ){
			ControlStatement* saveStatement = new ControlStatement();
			saveStatement->set_log_io( log_io );
			saveStatement->set_id( kInjectedSaveMonitorId );
			saveStatement->set_name( m->get_name() );
			saveStatement->set_master_name( kSaveInjectionMasterName );
			saveStatement->set_advanced_control_statement( false );
			saveStatement->set_special_syntax_control_statement( true );
			// We add the node to the injected source but not to the scope
			saveStatement->set_raw_content( m->get_name() );
			circuit.AddSimulationSpecialStatement( saveStatement );
		}
	}
	log_io->ReportPlain2Log( "Metrics save statements added.");
}

bool CircuitIOHandler::ReadMainNetlist( int stateNumber, std::string netlistFile,
	CircuitStatement& mainCircuit, const bool permissiveParsingMode ){
	//reading main netlist
	bool success;
	log_io->ReportStage( number2String(stateNumber) + ") Reading main netlist...", false );
	int statementCount = 0;
	int circuitCount = 0;
	bool parsingSpectreCode = false;

	boost::filesystem::path pn( netlistFile );
	if( !boost::filesystem::exists(pn) ){
		log_io->ReportError2AllLogs( k2Tab + "Error reading '"
			+ pn.string() + "', file does not exist." );
		return false;
	}
	std::string mainCircuitFolder = pn.parent_path().c_str();

	success = ReadNetlist(netlistFile, mainCircuitFolder,
		mainCircuit, mainCircuit, true, circuitCount, statementCount,
		parsingSpectreCode, permissiveParsingMode );
	if( success ){
		log_io->ReportPlainStandard( k2Tab + "-> Main circuit read." );
		AddSimulationSpecialStatements( mainCircuit );
	}else{
		log_io->ReportError2AllLogs( k2Tab + "Error reading Main circuit !!!." );
	}
	return success;
}

bool CircuitIOHandler::ReadNetlist( std::string netlistAbsolutePath,
	std::string folderFromWhichIsInstantiated,
	CircuitStatement& circuit, CircuitStatement& parentCircuit,
	bool isMain, int& circuitCount, int& statementCount,
	bool& parsingSpectreCode, const bool permissiveParsingMode ){

	bool success = true;
	std::string s_circuitCount = number2String(circuitCount);
	if( isMain ) {
		log_io->ReportPlainStandard( kTab + "* Reading Main netlist..." );
	}else{
		log_io->ReportPlainStandard( kTab + + "* Parsing included circuit #" + s_circuitCount + ", reading netlist..." );
	}
	// Path relative/absolute alterations
	boost::filesystem::path pn( netlistAbsolutePath );
	if( isMain || pn.is_absolute() ){
		log_io->ReportPlain2Log( kTab + "Absolute path: " + netlistAbsolutePath );
		folderFromWhichIsInstantiated = pn.parent_path().c_str();
		parsed_circuits.push_back( boost::filesystem::canonical(pn).c_str() );
		log_io->ReportPlain2Log( kTab + "Marked canonical path: " + parsed_circuits.back() );
	}else{
		log_io->ReportError2AllLogs( "Found relative path where there should be an absolute one: '"
			+ netlistAbsolutePath + "'");
	}
	//parse netlist
	success = circuit.ParseNetlist( netlistAbsolutePath, statementCount,
		parsingSpectreCode, permissiveParsingMode );
	if( !success ){
		log_io->ReportError2AllLogs( k2Tab + "Error parsing '"
			+ s_circuitCount + "' circuit !!!." );
		return false;
	}

	log_io->ReportPlainStandard( k2Tab + "-> '" + s_circuitCount +
		"' circuit parsed. Scanning for instances child of (inline) subcircuits." );
	// parse included netlist
	for( auto const & is : *circuit.get_referenced_include_statements() ){
		std::string canonical_path = "";
		if(  boost::starts_with( is->get_raw_content(), "/")
			|| boost::starts_with( is->get_raw_content(), "\\")) {
			if(  boost::filesystem::exists( is->get_raw_content() ) ){
				canonical_path = boost::filesystem::canonical(is->get_raw_content()).c_str();
			}else{
				log_io->ReportError2AllLogs( k2Tab + "Error reading '"
					+ is->get_raw_content() + "' circuit !!!. File does not exist." );
				return false;
			}
		}else{
			if( boost::filesystem::exists(
				folderFromWhichIsInstantiated + kFolderSeparator + is->get_raw_content() ) ){
				canonical_path = boost::filesystem::canonical(
					folderFromWhichIsInstantiated + kFolderSeparator + is->get_raw_content()).c_str();
			}else{
				log_io->ReportError2AllLogs( k2Tab + "Error reading '"
					+ is->get_raw_content() + "' circuit !!!. File does not exist." );
				return false;
			}
		}
		if( boost::filesystem::exists(canonical_path) ){
			if( !NetlistAlreadyParsed( canonical_path )
				&& !experiment_environment->TestIsCanonicalFolderExcluded( canonical_path ) ){
				CircuitStatement* includedCircuit = new CircuitStatement( true );
				// Preincrement!
				includedCircuit->set_circuit_id( ++circuitCount );
				includedCircuit->set_id( statementCount++ );
				includedCircuit->set_post_parsing_statement_handler( post_parsing_statement_handler );
				// logger
				includedCircuit->set_log_io( log_io );
				// includedCircuit->.set_post_parsing_statement_handler( post_parsing_statement_handler );
				includedCircuit->set_experiment_environment( experiment_environment );
				includedCircuit->set_master_name("circuit");
				includedCircuit->set_name( kIncludedPrefixId + number2String(circuitCount) );
				includedCircuit->set_description( is->get_raw_content() );
				success = success && ReadNetlist( canonical_path, folderFromWhichIsInstantiated,
					*includedCircuit, circuit, false, circuitCount,
					statementCount, parsingSpectreCode, permissiveParsingMode);
				parentCircuit.AddReferencedCircuitStatement( includedCircuit );
				is->set_scanned( true );
				is->set_refered_circuit_id( kIncludedPrefixId + number2String(includedCircuit->get_circuit_id()) );
				is->set_export_canonical_path( false );
			}else{
				log_io->ReportPlain2Log(  kTab + "Netlist '" + canonical_path + "' parsing has been ommited." );
				is->set_export_canonical_path( true );
				is->set_scanned( true );
				is->set_raw_content( canonical_path );
			}
		}else{
			log_io->ReportError2AllLogs( k2Tab + "Error reading '"
				+ is->get_raw_content() + "' circuit !!!. File does not exist." );
			return false;
		}
	}
	if( success ){
		log_io->ReportPlainStandard( k2Tab + "Circuit '"
			+ s_circuitCount + "' and children correctly parsed." );
	}else{
		log_io->ReportError2AllLogs( k2Tab + "Error reading '"
			+ s_circuitCount + "' circuit !!!." );
	}
	return success;
}

bool CircuitIOHandler::NetlistAlreadyParsed( std::string path ){
	for( auto& pc : parsed_circuits ){
		if( path.compare( pc )==0 ){
			return true;
		}
	}
	return false;
}

bool CircuitIOHandler::ExportSingularNetlist( std::string singularFolder, CircuitStatement& singularCircuit ){
	bool success;
	boost::filesystem::path singularFolderPath( singularFolder );
	if( !boost::filesystem::exists( singularFolderPath )
		|| !boost::filesystem::is_directory( singularFolderPath )){
		#ifdef NETLIST_EXPORT_VERBOSE
			log_io->ReportPlainStandard( k2Tab + "-> Creating golden folder." );
		#endif
		if( !CreateFolder( singularFolder, false ) ){
			log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '" + singularFolder + "'." );
			return false;
		}
	}
	success = ExportNonAlteredNetlist( singularCircuit, true, singularFolder);
	#ifdef NETLIST_EXPORT_VERBOSE
		if( success ){
			log_io->ReportPlainStandard( k2Tab + "-> Singular circuit netlist exported: '" + singularCircuit.get_name() + "'" );
		}else{
			log_io->ReportError2AllLogs( k2Tab + "Error exporting Singular circuit netlist !!! '" + singularCircuit.get_name() + "'" );
		}
	#endif
	return success;
}

bool CircuitIOHandler::ExportNonAlteredNetlist( CircuitStatement& circuit, bool isMain, std::string folder ){
	#ifdef NETLIST_EXPORT_VERBOSE
		if(isMain){
			log_io->ReportPlainStandard( k2Tab + "Exporting netlist '" + circuit.get_name() + "'" );
		}else{
			log_io->ReportPlain2Log( k2Tab + "Exporting netlist '" + circuit.get_name() + "'" );
		}
	#endif
	bool success = true;
	std::string oScsFile;
	if( isMain ) {
		 oScsFile = folder + kFolderSeparator + kMainNetlistFile;
	}else{
		 oScsFile = folder + kFolderSeparator + circuit.get_name() + kCircutFileSufix;
	}
	success = success && circuit.ExportCircuit2SCS( oScsFile);
	#ifdef NETLIST_EXPORT_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "Netlist '" + circuit.get_name() + "' exported." );
	#endif
	//included circuits
	#ifdef NETLIST_EXPORT_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "Circuit '" + circuit.get_name() + "' has "
			+ number2String( circuit.get_referenced_circuit_statements()->size()) + " referenced circuits." );
	#endif
	for( auto const &rcs : *circuit.get_referenced_circuit_statements() ){
		success = success && ExportNonAlteredNetlist( *rcs, false, folder );
	}
	#ifdef NETLIST_EXPORT_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "-> Children of " + circuit.get_name() + " exported." );
	#endif
	return success;
}

bool CircuitIOHandler::ExportSimpleAlteredScenario( CircuitStatement& radiationCircuit,
	CircuitStatement& alteredCircuit, CircuitStatement& mainCircuit, std::string folder ){
	bool success = true;
	std::string oScsFile;

	#ifdef NETLIST_EXPORT_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "Exporting altered scenario." );
	#endif
	// Export radiation netlist.
	oScsFile = folder + kFolderSeparator + kAlterationSourceNetlistFile;
	success = success &&radiationCircuit.ExportCircuit2SCS( oScsFile);
	#ifdef NETLIST_EXPORT_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "Radiation Circuit Exported." );
	#endif
	// Export altered netlist.
	// oScsFile = folder + kFolderSeparator + kRadiatedNetlistFile;
	oScsFile = folder + kFolderSeparator + alteredCircuit.get_name() + kCircutFileSufix;
	success = success &&alteredCircuit.ExportCircuit2SCS( oScsFile);
	#ifdef NETLIST_EXPORT_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "Radiated Circuit Exported." );
	#endif
	// Export non-altered netlists
	success = success && ExportAlteredNetlist( mainCircuit,
		alteredCircuit.get_id(), true, folder);

	#ifdef NETLIST_EXPORT_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "Altered Scenario Exported." );
	#endif
	return success;
}

bool CircuitIOHandler::ExportAlteredScenario(
	CircuitStatement& radiationCircuit, CircuitStatement& alteredCircuit,
	CircuitStatement& alteredStatementsCircuit, CircuitStatement& mainCircuit, std::string folder ){
	bool success = true;
	std::string oScsFile;
	#ifdef NETLIST_EXPORT_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "Exporting altered scenario." );
	#endif
	// Export radiation netlist.
	oScsFile = folder + kFolderSeparator + kAlterationSourceNetlistFile;
	success = success && radiationCircuit.ExportCircuit2SCS( oScsFile);
	#ifdef NETLIST_EXPORT_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "Radiation Circuit Exported." );
	#endif
	// Export altered netlist.
	// oScsFile = folder + kFolderSeparator + kRadiatedNetlistFile;
	oScsFile = folder + kFolderSeparator + alteredCircuit.get_name() + kCircutFileSufix;
	success = success && alteredCircuit.ExportCircuit2SCS( oScsFile);
	#ifdef NETLIST_EXPORT_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "Radiated Circuit Exported." );
	#endif
	// Export altered statements netlist.
	oScsFile = folder + kFolderSeparator + kAlteredStatementsNetlistFile;
	success = success && alteredStatementsCircuit.ExportCircuit2SCS( oScsFile);
	#ifdef NETLIST_EXPORT_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "Altered Circuit Exported." );
	#endif
	// Export non-altered netlists
	success = success && ExportAlteredNetlist( mainCircuit,
		alteredCircuit.get_id(), true, folder);
	#ifdef NETLIST_EXPORT_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "Altered Scenario Exported." );
	#endif
	return success;
}

bool CircuitIOHandler::ExportAlteredNetlist( CircuitStatement& circuit, int avoidCircuitId,
	bool isMain, std::string folder ){

	bool success = true;
	std::string oScsFile;
	// Test if the non-altered circuit is the main one.
	if( isMain ) {
		oScsFile = folder + kFolderSeparator + kMainNetlistFile;
	}else{
		oScsFile = folder + kFolderSeparator + circuit.get_name() + kCircutFileSufix;
	}
	// Test if the altered circuit is the one we where going to export.
	if( circuit.get_circuit_id() != avoidCircuitId ){
		success = success && circuit.ExportCircuit2SCS( oScsFile);
		#ifdef NETLIST_EXPORT_VERBOSE
			log_io->ReportPlain2Log( k2Tab + "Non-Altered Netlist '" + circuit.get_name() + "' exported." );
		#endif
	}
	//included circuits
	for( auto const & rcs : *circuit.get_referenced_circuit_statements() ){
		success = success && ExportAlteredNetlist( *rcs, avoidCircuitId, false, folder );
	}
	#ifdef NETLIST_EXPORT_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "-> Children of " +  circuit.get_name()  + " exported." );
	#endif
	return success;
}

void CircuitIOHandler::AddMetric( Metric* metric ){
	this->metrics.push_back( metric );
}
