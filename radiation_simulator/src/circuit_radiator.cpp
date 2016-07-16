/**
 * @file circuit_radiator.cpp
 *
 * @date Created on: Jan 20, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * Main code of CircuitRadiator Class (see circuit_radiator.hpp)
 *
 */

// Radiation simulator includes
#include "circuit_radiator.hpp"

// Spectre Handler Library
#include "global_functions_and_constants/include_statement_constants.hpp"
#include "global_functions_and_constants/global_constants.hpp"
#include "global_functions_and_constants/statements_constants.hpp"
#include "global_functions_and_constants/global_template_functions.hpp"
#include "global_functions_and_constants/files_folders_io_constants.hpp"


CircuitRadiator::CircuitRadiator() {
	radiationSubcircuitsCircuit.set_name( kAlterationSourcesCircuitStringId );
	radiationSubcircuitsCircuitAHDL.set_name( "radiationSubcircuitsCircuitAHDL" );
}

CircuitRadiator::~CircuitRadiator() {
	#ifdef DESTRUCTORS_VERBOSE
		log_io->ReportPlain2Log( "CircuitRadiator Destructor. Direction: " + number2String(this) );
	#endif
}

bool CircuitRadiator::SetUpSingularScenarios( std::string goldenFolder,
		std::string goldenAhdlFolder, std::string radiationSubcircuitAHDLFolder ){

	// Add params to main circuit
	main_circuit->AddIncludeStatementAndRegister( GetNewIncludeStatementOfParametersCircuit() );

	// Golden and Golden AHDL
	log_io->ReportPlainStandard( k3Tab + "Exporting golden netlist '" + main_circuit->get_name() + "'" );
	CircuitStatement* exportedGoldenCircuit = new CircuitStatement( *main_circuit );
	// models and section for golden
	exportedGoldenCircuit->AddIncludeStatementAndRegister( GetGoldenModelsAndSectionIncludeStatement() );
	// mute analysis
	exportedGoldenCircuit->MuteNonMainTransientAnalysis();
	// Export golden ahdl
	if( !circuit_io_handler->ExportSingularNetlist( goldenAhdlFolder , *exportedGoldenCircuit ) ){
		return false;
	}
	// Export golden
	if( !circuit_io_handler->ExportSingularNetlist( goldenFolder , *exportedGoldenCircuit ) ){
		return false;
	}
	// delete golden
	delete exportedGoldenCircuit;

	// Golden Circuit refers its owns models
	// add experiment models/section and simulation parameters for altered circuits and AHDL circuit
	log_io->ReportPlainStandard( k3Tab + "Adding models include reference to circuits" );
	main_circuit->AddIncludeStatementAndRegister( GetExperimentModelsAndSectionIncludeStatement() );

	// AHDL
	log_io->ReportPlainStandard( k3Tab + "Exporting AHDL (golden) netlist '" + main_circuit->get_name() + "'" );
	// Create file
	CreateRadiationAndAHDLCircuits();
	// Export Dummy Golden netlist(AHDL)
	if( !circuit_io_handler->ExportSingularNetlist( radiationSubcircuitAHDLFolder, radiationSubcircuitsCircuitAHDL )){
		return false;
	}

	return true;
}

IncludeStatement* CircuitRadiator::GetGoldenModelsAndSectionIncludeStatement(){
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

IncludeStatement* CircuitRadiator::GetExperimentModelsAndSectionIncludeStatement(){
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

bool CircuitRadiator::RadiateMainCircuit( ){
	bool success = true;
	// Radiate main_circuit
	int modificationCounter = 0;
	success = success && RadiateCircuit( *main_circuit, modificationCounter );
	return success;
}

// Radiate a whole circuit
bool CircuitRadiator::RadiateCircuit( CircuitStatement& circuit, int& modificationCounter ){
	bool success = true;
	log_io->ReportPlainStandard( k2Tab + "Radiating netlist '" + circuit.get_name() + "'" );
	if( simulation_mode->get_alteration_mode()->get_injection_mode() ){
		success = AnalyzeStatementForInjection( circuit, modificationCounter );
	}else{
		success = AnalyzeStatementForReplacement( circuit, modificationCounter );
	}
	log_io->ReportInfo2AllLogs( k2Tab + "Netlist '" + circuit.get_name() + "' analyzed and altered." );
	for(std::vector<CircuitStatement*>::iterator it_c = circuit.get_referenced_circuit_statements()->begin();
			it_c != circuit.get_referenced_circuit_statements()->end(); it_c++){
		success = success && RadiateCircuit( **it_c, modificationCounter );
	}
	return success;
}

/////////////////////////////
/// Substitution Methods  ///
/////////////////////////////

bool CircuitRadiator::AnalyzeStatementForReplacement( Statement& statement, int& modificationCounter ){
	bool success =true;
	if( !statement.get_unalterable() ){
		if( statement.get_can_be_substituted() ){
			#ifdef RADIATION_VERBOSE
				log_io->ReportGreenStandard( k2Tab + statement.get_name() + " can be replaced.");
			#endif
				// InstanceStatement Cast (includes transistor statements)
			InstanceStatement* is = dynamic_cast<InstanceStatement*>(&statement);
			ModelStatement* ms = dynamic_cast<ModelStatement*>(&statement);
			AnalogModelStatement* ams = dynamic_cast<AnalogModelStatement*>(&statement);
			if (is) {
				/* ... points to a InstanceStatement ... */
				// Analyze if the injection is a simple one,
				// or or it requires from instances substitution though
				if( is->get_belonging_scope()->get_is_subcircuit_scope() ){
					#ifdef RADIATION_VERBOSE
						log_io->ReportPlain2Log("-> Instance: " + is->get_name() + " is a child of a subcircuit.");
					#endif
					SubstituteSubcircuitChild(*is, modificationCounter);
				}else{ // Simple substitution
					SubstituteSimpleStatement(*is, modificationCounter);
				}
			}else if(ms){
				for(std::vector<Statement*>::iterator it_m = statement.get_dependency_children()->begin();
					it_m != statement.get_dependency_children()->end(); it_m++){
					AnalyzeStatementForReplacement( **it_m, modificationCounter );
				}
			}else if(ams){
				for(std::vector<Statement*>::iterator it_am = statement.get_dependency_children()->begin();
					it_am != statement.get_dependency_children()->end(); it_am++){
					AnalyzeStatementForReplacement( **it_am, modificationCounter );
				}
			}
		}else{
			log_io->ReportPlain2Log( k2Tab + statement.get_name() + " can NOT be replaced.");
		}
		if(statement.get_altered()){
			log_io->ReportPlain2Log( k2Tab +  statement.get_name() + " has already been altered.");
		}
		// foreach child
		if( statement.get_children()->size() > 0 ){
			for(std::vector<Statement*>::iterator it_s = statement.get_children()->begin();
				it_s != statement.get_children()->end(); it_s++){
				// std::vector<Node*> scope_injected_nodes_children;
				success = success && AnalyzeStatementForReplacement( **it_s, modificationCounter );
			}
		}
	}else{
		log_io->ReportPlain2Log( k2Tab +  statement.get_name() + " (" + statement.get_master_name() + ") is an unalterable statement.");
	}
	return success;
}

bool CircuitRadiator::SubstituteSimpleStatement( Statement& statement,
	int& modificationCounter ){
	bool success = true;
	std::string alteredFolder = GetRadiatedFolderName( modificationCounter );
	if( !CreateFolder( alteredFolder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '" + alteredFolder + "'." );
		return false;
	}
	// Copy of the circuit to be altered
	CircuitStatement* localAlteredCircuit = new CircuitStatement(
		*(CircuitStatement*) statement.get_belonging_circuit() ) ;
	// Altered statement
	Statement* alteredStatement;
	if( !localAlteredCircuit->GetChildById( statement.get_id(), alteredStatement ) ){
		log_io->ReportError2AllLogs( k3Tab + "Error altering. Error finding statement with id '"
			+ number2String( statement.get_id() ) + "'");
			return false;
	}
	alteredStatement->set_master_name( alteredStatement->get_substitute_master_name() );
	// Report substitution
	#ifdef RADIATION_VERBOSE
		log_io->ReportSubstitution( false, alteredStatement->get_substitute_master_name(), statement.get_name(),
			statement.get_belonging_scope()->get_name(), statement.get_parent()->get_name(),
			number2String(statement.get_parent()->get_id()), number2String( modificationCounter ));
	#endif

	// Export netlists
	log_io->ExportReadmeStandardSubstitution( statement.get_name(),
		statement.get_master_name(), statement.get_name(), alteredStatement->get_master_name(), alteredFolder );
	circuit_io_handler->ExportSimpleAlteredScenario( radiationSubcircuitsCircuit,
		*localAlteredCircuit, *main_circuit, alteredFolder );
	radiation_spectre_handler->AddScenarioFolderPath( modificationCounter, alteredFolder, statement.get_name() );

	// deletes both alteredStatementsCircuit and alteredSubcircuitParent,
	delete localAlteredCircuit;
	statement.set_altered( true );
	modificationCounter++;
	return success;
}

bool CircuitRadiator::SubstituteSubcircuitChild( Statement& childOfSubcircuitStatement,
	int& modificationCounter ){
	bool success = true;
	CircuitStatement* alteredStatementsCircuit = GetAlteredStatementsCircuit();
	// Create altered subcircuit ('s2') of the parent ('s') of 'i'
	// ReplaceInjectedSubcircuitInstances( 's', s2' );
	SubcircuitStatement* alteredSubcircuitParent = new SubcircuitStatement(
		*( SubcircuitStatement*) childOfSubcircuitStatement.get_global_scope_parent() );
	alteredSubcircuitParent->set_name( alteredSubcircuitParent->get_name() + kRadiatedSufix );
	// Altered statement
	Statement* alteredStatement;
	if( !alteredSubcircuitParent->GetChildById( childOfSubcircuitStatement.get_id(), alteredStatement ) ){
		log_io->ReportError2AllLogs( k3Tab + "Error altering. Error finding statement with id '"
			+ number2String( childOfSubcircuitStatement.get_id() ) + "'");
			return false;
	}
	alteredStatement->set_master_name( alteredStatement->get_substitute_master_name() );

	#ifdef RADIATION_VERBOSE
		log_io->ReportPlain2Log( "Adding " + alteredSubcircuitParent->get_name()
				+ " while replacing " + childOfSubcircuitStatement.get_name() );
	#endif
	alteredStatementsCircuit->AddStatement( alteredSubcircuitParent );
	// Report substitution
	#ifdef RADIATION_VERBOSE
		log_io->ReportSubstitution( true, alteredStatement->get_substitute_master_name(), childOfSubcircuitStatement.get_name(),
			childOfSubcircuitStatement.get_belonging_scope()->get_name(), childOfSubcircuitStatement.get_parent()->get_name(),
			number2String(childOfSubcircuitStatement.get_parent()->get_id()), number2String( modificationCounter ));
	#endif
	std::string deepLevel = "Substituted " + childOfSubcircuitStatement.get_name();
	std::string path2BelongingCircuit = childOfSubcircuitStatement.get_name();
	success = success && ReplaceAlteredSubcircuitInstances( deepLevel,
		alteredStatement->get_substitute_master_name(), *alteredStatementsCircuit,
		*( SubcircuitStatement*) childOfSubcircuitStatement.get_global_scope_parent(),
		*alteredSubcircuitParent, modificationCounter,  childOfSubcircuitStatement.get_name(),
		childOfSubcircuitStatement.get_master_name(), path2BelongingCircuit,
		childOfSubcircuitStatement.get_belonging_scope()->get_name());
	// deletes both alteredStatementsCircuit and alteredSubcircuitParent,
	delete alteredStatementsCircuit;
	childOfSubcircuitStatement.set_altered( true );
	return success;
}

///////////////////////
// Injection Methods //
///////////////////////

bool CircuitRadiator::AnalyzeStatementForInjection( Statement& statement, int& modificationCounter ){
	bool success =true;
	if( !statement.get_unalterable() ){
		// if substitution method and the statement can be substituted substitute.
		// Else, if injection method and the statement can be injected
		// InstanceStatement Cast check, only in case
		InstanceStatement* is = dynamic_cast<InstanceStatement*>(&statement);
		if (is) {
		    /* ... points to a InstanceStatement ... */
			#ifdef RADIATION_VERBOSE
				if( statement.get_can_be_injected() ){
					log_io->ReportPlain2Log( k2Tab + statement.get_name() + " can be injected.");
				}else{
					log_io->ReportPlain2Log( k2Tab + statement.get_name() + " can NOT be injected.");
				}
				if(statement.get_altered()){
					log_io->ReportPlain2Log( k2Tab +  statement.get_name() + " has already been altered.");
				}
			#endif
			if( statement.get_can_be_injected() && !statement.get_altered() ){
				success = success && InjectStatement( (InstanceStatement&) statement,
					modificationCounter );
			}else{
				#ifdef RADIATION_VERBOSE
					log_io->ReportStatementNotRadiated( statement.get_name(), number2String( statement.get_parent()->get_id() ),
						statement.get_parent()->get_name(), statement.get_belonging_scope()->get_name() );
				#endif
			}
		}
		// foreach child
		if( statement.get_children()->size() > 0 ){
			for(std::vector<Statement*>::iterator it_s = statement.get_children()->begin();
				it_s != statement.get_children()->end(); it_s++){
				// std::vector<Node*> scope_injected_nodes_children;
				success = success && AnalyzeStatementForInjection( **it_s, modificationCounter );
			}
		}
	}else{
		log_io->ReportPlain2Log( k2Tab +  statement.get_name() + " (" + statement.get_master_name() + ") is an unalterable statement.");
	}
	return success;
}

bool CircuitRadiator::InjectStatement( InstanceStatement& statement, int& modificationCounter ){
	bool success =true;
	// foreach node	in statement
	// This way we analyze the whole nodes in scope.
	for(std::vector<Node*>::iterator it_n = statement.get_nodes()->begin();
			it_n != statement.get_nodes()->end(); it_n++){
		#ifdef RADIATION_VERBOSE
			log_io->ReportPlain2Log( "Analyzing statement '" + statement.get_name() + "', node '"
				+ (*it_n)->get_name() + "' of scope '" + statement.get_belonging_scope()->get_name() + "'" );
		#endif
		//if perceptible of being injected and not being already injected
		if( (*it_n)->get_injectable() && !(*it_n)->get_injected() ){
			// Analyze if the injection is a simple one,
			// or or it requires from instances substitution though
			if( statement.get_belonging_scope()->get_is_subcircuit_scope() ){
				if( !(*it_n)->get_pin_in_subcircuit() ){
					#ifdef RADIATION_VERBOSE
						log_io->ReportPlain2Log("-> node: " + (*it_n)->get_name() + " is NOT a pin of subcircuit. Injecting.");
					#endif
					InjectNodeOfSubcircuitChild( **it_n, statement, modificationCounter );
				}
				#ifdef RADIATION_VERBOSE
					else{
						log_io->ReportPlain2Log("-> node: " + (*it_n)->get_name()
							+ " is a pin of subcircuit. Not injected (Trying to inject statement "
							+ statement.get_name() + " )");
					}
				#endif
			}else{ // Simple injection
				InjectSimpleNode(**it_n, statement, modificationCounter);
			}
		}else{
			#ifdef RADIATION_VERBOSE
				log_io->ReportPlain2Log( "node '" + (*it_n)->get_name()
					+ "'' in statement '" + statement.get_name() + "' already injected'" );
			#endif
		}
		#ifdef RADIATION_VERBOSE
			log_io->ReportPlain2Log( "Statement '" + statement.get_name() + "', node '"
				+ (*it_n)->get_name() + "' of scope '" + statement.get_belonging_scope()->get_name() + "' analyzed" );
		#endif
	}
	//update statement status
	statement.set_altered( true );
	return success;
}

bool CircuitRadiator::InjectNodeOfSubcircuitChild( Node& node,
	InstanceStatement& childOfSubcircuitStatement, int& modificationCounter ){
	bool success = true;
	CircuitStatement* alteredStatementsCircuit = GetAlteredStatementsCircuit();
	// Create altered subcircuit ('s2') of the parent ('s') of 'i'
	// ReplaceInjectedSubcircuitInstances( 's', s2' );
	SubcircuitStatement* alteredSubcircuitParent = new SubcircuitStatement(
		*( SubcircuitStatement*) childOfSubcircuitStatement.get_global_scope_parent() );
	// Injector instance
	InstanceStatement* injectorSource = GetNewInjectorInstance( node, kAlterationInjectorName );
	alteredSubcircuitParent->set_name( alteredSubcircuitParent->get_name() + kRadiatedSufix );
	alteredSubcircuitParent->AddStatement( injectorSource );
	if( radiation_spectre_handler->get_save_injection_sources() && simulation_mode->get_alteration_mode()->get_injection_mode() ){
		alteredSubcircuitParent->AddStatement( GetNewSaveInjectionStatement( "" ) );
	}
	#ifdef RADIATION_VERBOSE
		log_io->ReportPlain2Log( "Adding " + alteredSubcircuitParent->get_name()
				+ " while injecting " + childOfSubcircuitStatement.get_name()  + " Node: " + node.get_name() );
	#endif
	alteredStatementsCircuit->AddStatement( alteredSubcircuitParent );
	// now update path 2 belonging circuit
	post_parsing_statement_handler->FindPath2BelongingCircuit( injectorSource );
	// Injector monitoring
	std::string path2BelongingCircuit = childOfSubcircuitStatement.get_name();
	// Report injection
	#ifdef RADIATION_VERBOSE
		log_io->ReportNodeInjection( true, node.get_name(), childOfSubcircuitStatement.get_name(),
			childOfSubcircuitStatement.get_belonging_scope()->get_name(), childOfSubcircuitStatement.get_parent()->get_name(),
			number2String(childOfSubcircuitStatement.get_parent()->get_id()), number2String( modificationCounter ));
	#endif
	std::string deepLevel = "injecting " + childOfSubcircuitStatement.get_name()  + " Node: " + node.get_name() ;
	success = success && ReplaceAlteredSubcircuitInstances( deepLevel, node.get_name(),
		*alteredStatementsCircuit, *( SubcircuitStatement*) childOfSubcircuitStatement.get_global_scope_parent(),
		*alteredSubcircuitParent, modificationCounter,  childOfSubcircuitStatement.get_name(),
		childOfSubcircuitStatement.get_master_name(), path2BelongingCircuit,
		childOfSubcircuitStatement.get_belonging_scope()->get_name());
	// deletes both alteredStatementsCircuit and alteredSubcircuitParent,
	// therefore also injectorSource
	delete alteredStatementsCircuit;
	node.set_injected( true );
	return success;
}

// Standard injection, for nodes outside subcircuits
bool CircuitRadiator::InjectSimpleNode( Node& node, InstanceStatement& statement, int& modificationCounter ){
	bool success = true;
	std::string alteredFolder = GetRadiatedFolderName( modificationCounter );
	if( !CreateFolder( alteredFolder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '" + alteredFolder + "'." );
		return false;
	}
	// Copy of the circuit to be altered
	CircuitStatement* localAlteredCircuit = new CircuitStatement( *(CircuitStatement*) statement.get_belonging_circuit() ) ;
	if( localAlteredCircuit==nullptr ){
		log_io->ReportError2AllLogs( "null localAlteredCircuit in InjectSimpleNode, for statement " + statement.get_name() );
	}
	// Include statement reference
	localAlteredCircuit->AddIncludeStatementAndRegister( GetNewIncludeStatementOfRadiationCircuit() );
	// Injector instance
	InstanceStatement* injectorSource = GetNewInjectorInstance( node, kAlterationInjectorName );
	// Add radiation source to parent (if not a subcircuit)
	success = success && AddRadiationSourceInstance2Parent( statement, *localAlteredCircuit, *injectorSource);
	// now update path 2 belonging circuit
	post_parsing_statement_handler->FindPath2BelongingCircuit( injectorSource );
	// Injector monitoring
	if( radiation_spectre_handler->get_save_injection_sources() && simulation_mode->get_alteration_mode()->get_injection_mode() ){
		localAlteredCircuit->AddStatement( GetNewSaveInjectionStatement( "" ) );
	}
	// Export netlists
	log_io->ExportReadmeStandardInjection( statement.get_name() + ":" + node.get_name(),
		statement.get_master_name(), statement.get_name(),
		statement.get_belonging_scope()->get_name(), node.get_name(), alteredFolder );
	circuit_io_handler->ExportSimpleAlteredScenario( radiationSubcircuitsCircuit,
		*localAlteredCircuit, *main_circuit, alteredFolder );
	radiation_spectre_handler->AddScenarioFolderPath( modificationCounter, alteredFolder, statement.get_name() + ":" + node.get_name() );
	// Delete modified and radiantion source subcircuit circuits
	#ifdef RADIATION_VERBOSE
		log_io->ReportNodeInjection( false, node.get_name(), statement.get_name(),
			statement.get_belonging_scope()->get_name(), statement.get_parent()->get_name(),
			number2String(statement.get_parent()->get_id()), number2String( modificationCounter ));
	#endif
	// Mark node and update modification counter
	node.set_injected( true );
	++modificationCounter;
	delete localAlteredCircuit;
	return true;
}

bool CircuitRadiator::AddRadiationSourceInstance2Parent(Statement& statement,
	CircuitStatement& modifiedCircuit, InstanceStatement& injectorSource){
	if( statement.get_statement_type() == kCircuitStatement ){
		modifiedCircuit.AddStatement( &injectorSource );
	}else{
		Statement* parent = NULL;
		if(!modifiedCircuit.GetChildById(statement.get_parent()->get_id(), parent) ){
			log_io->ReportError2AllLogs( k3Tab + "Error altering. Error finding parent of '"
				+ number2String( statement.get_id() ) + "in order to inject " + injectorSource.get_master_name() +"'");
			return false;
		}else{
			parent->AddStatement( &injectorSource );
		}
	}
	return true;
}

///////////////////////
// General Methods   //
///////////////////////

bool CircuitRadiator::ReplaceAlteredSubcircuitInstances( std::string& deepLevel, std::string alteredParameterName,
	CircuitStatement& alteredStatementsCircuit,
	SubcircuitStatement& originalSubcircuit, SubcircuitStatement& alteredSubcircuit, int& modificationCounter,
	std::string alteredStatementName, std::string alteredStatementMasterName,
	std::string& alteredStatementPathToBelongingCircuit, std::string alteredScopeName ){
	bool success = true;
	deepLevel += " || Changing '" + originalSubcircuit.get_name() + "'' -> '"
		+ alteredSubcircuit.get_name() + "'";
	for( auto const& instanceStatement : *(originalSubcircuit.get_progeny()) ){
		if( !(instanceStatement->get_unalterable()) ){
			success = success && ReplaceAlteredSubcircuitInstance(deepLevel,
				alteredParameterName, alteredStatementsCircuit,
				*instanceStatement, originalSubcircuit, alteredSubcircuit, modificationCounter, alteredStatementName,
				alteredStatementMasterName, alteredStatementPathToBelongingCircuit, alteredScopeName );
		}
	}
	return success;
}

bool CircuitRadiator::ReplaceAlteredSubcircuitInstance(std::string& deepLevel, std::string alteredParameterName,
	CircuitStatement& alteredStatementsCircuit, InstanceStatement& instanceOfSubcircuit,
	SubcircuitStatement& originalSubcircuit, SubcircuitStatement& alteredSubcircuit,
	int& modificationCounter, std::string alteredStatementName, std::string alteredStatementMasterName,
	std::string alteredStatementPathToBelongingCircuit, std::string alteredScopeName ){
	bool success = true;
	// Alterations made to a 'local' altered circuit,
	// in order to preserve the original alteredStatementsCircuit for recursive calls
	CircuitStatement* localAlteredStatementsCircuit = new CircuitStatement( alteredStatementsCircuit );
	std::string localDeepLevel = deepLevel;
	std::string localAlteredStatementPathToBelongingCircuit = alteredStatementPathToBelongingCircuit;
 	// Create altered models till find the instance
	Statement* dependencyParent = instanceOfSubcircuit.get_dependency_parent();
	// Models/Analog Models between instance and subcircuit
	while( dependencyParent != instanceOfSubcircuit.get_global_dependency_parent() ){
		// altered_model will be deleted in the localAlteredStatementsCircuit destructor
		Statement* alteredModel;
		if( dependencyParent->get_statement_type()==kModelStatement ){
			alteredModel = new ModelStatement( *(ModelStatement*) dependencyParent );
		}else{
			alteredModel = new AnalogModelStatement( *(AnalogModelStatement*) dependencyParent );
		}
		alteredModel->set_name( dependencyParent->get_name() + kRadiatedSufix );
		alteredModel->set_master_name( dependencyParent->get_master_name() + kRadiatedSufix );
		localAlteredStatementsCircuit->AddStatement( alteredModel );
		dependencyParent = dependencyParent->get_dependency_parent();
	}
	// We know that dependencyParent is a subcircuit
	SubcircuitStatement* cdp = dynamic_cast<SubcircuitStatement*>(dependencyParent);
	if( !cdp || dependencyParent->get_statement_type()!=kSubcircuitStatement ){
		log_io->ReportError2AllLogs( k3Tab
			+ "Error. Dependency_parent in not a subcircuit! type:"
			+ number2String(dependencyParent->get_statement_type()) );
		return false;
	}
	// Recursive call in case that the substituted instance_statement is instance of a subcircuit
	if( instanceOfSubcircuit.get_belonging_scope()->get_is_subcircuit_scope() ) {
		SubcircuitStatement* originalSubcircuitParent =
			( SubcircuitStatement*) instanceOfSubcircuit.get_global_scope_parent();
		SubcircuitStatement* alteredSubcircuitParent = new SubcircuitStatement( *originalSubcircuitParent );
		alteredSubcircuitParent->set_name(alteredSubcircuitParent->get_name() + kRadiatedSufix );
		// Find the altered statement in itself and alter
		Statement* alteredSubcircuitChild;
		if( !alteredSubcircuitParent->GetChildById( instanceOfSubcircuit.get_id(), alteredSubcircuitChild ) ){
			log_io->ReportError2AllLogs( k3Tab + "Error altering. Error finding statement with id '"
				+ number2String( instanceOfSubcircuit.get_id() ) + "'");
			return false;
		}
		alteredSubcircuitChild->set_master_name( alteredSubcircuitChild->get_master_name() + kRadiatedSufix );
		alteredSubcircuitChild->set_name( alteredSubcircuitChild->get_name() + kRadiatedSufix );
		localAlteredStatementPathToBelongingCircuit = alteredSubcircuitChild->get_name()
			+ "." + localAlteredStatementPathToBelongingCircuit;
		// We need to add the altered subcircuit to the altered circuit
		#ifdef RADIATION_VERBOSE
		log_io->ReportPlain2Log(  "Alteration #" + number2String(modificationCounter)
			+ " adding '" + alteredSubcircuitParent->get_name()
			+ "'' while substituting '" + instanceOfSubcircuit.get_name() +
			+ "' (" + originalSubcircuit.get_name() + " by " + alteredSubcircuit.get_name() + ")" );
		#endif
		localAlteredStatementsCircuit->AddStatement( alteredSubcircuitParent );
		// And call recursivelly
		success = success && ReplaceAlteredSubcircuitInstances(localDeepLevel, alteredParameterName,
			*localAlteredStatementsCircuit, *originalSubcircuitParent, *alteredSubcircuitParent,
			modificationCounter, alteredStatementName, alteredStatementMasterName,
			localAlteredStatementPathToBelongingCircuit, alteredScopeName );
	} else {
		// Alterations made to a 'local' altered instance,
		CircuitStatement* localInstanceOfSubcircuitAlteredCircuit = new CircuitStatement(
			*(CircuitStatement*) instanceOfSubcircuit.get_belonging_circuit() );

		Statement* alteredInstance;
		if( !localInstanceOfSubcircuitAlteredCircuit->GetChildById( instanceOfSubcircuit.get_id(), alteredInstance ) ){
			log_io->ReportError2AllLogs( k3Tab + "Error altering. Error finding statement with id '"
				+ number2String( instanceOfSubcircuit.get_id() ) + "'");
				return false;
		}
		alteredInstance->set_master_name( alteredInstance->get_master_name() + kRadiatedSufix );
		// Include the reference of the radiation circuit
		// In the altered one.
		localInstanceOfSubcircuitAlteredCircuit->AddIncludeStatementAndRegister( GetNewIncludeStatementOfRadiationCircuit( ) );
		localInstanceOfSubcircuitAlteredCircuit->AddIncludeStatementAndRegister( GetNewIncludeStatementOfAlteredStatementsCircuit( ) );
		// monitor
		if( radiation_spectre_handler->get_save_injection_sources() && simulation_mode->get_alteration_mode()->get_injection_mode() ){
			localInstanceOfSubcircuitAlteredCircuit->AddStatement( GetNewSaveInjectionStatement( instanceOfSubcircuit.get_name() + "." ) );
		}
		// Export netlists
		std::string alteredFolder = GetRadiatedFolderName( modificationCounter );
		if( !CreateFolder( alteredFolder, false ) ){
			log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '" + alteredFolder + "'." );
			return false;
		}
		std::string pathInject = alteredInstance->get_name() + "." + alteredStatementPathToBelongingCircuit;
		if ( simulation_mode->get_alteration_mode()->get_injection_mode() ){
			pathInject += ":" + alteredParameterName;
		}
    // Export readme
		log_io->ExportReadmeInstanceOfSubcircuitReplacement(
			pathInject, simulation_mode->get_alteration_mode()->get_injection_mode(), localDeepLevel,
			alteredParameterName, instanceOfSubcircuit.get_name(), number2String(modificationCounter),
			originalSubcircuit.get_name(), alteredSubcircuit.get_name(),
			localInstanceOfSubcircuitAlteredCircuit->get_name(), alteredFolder,
			alteredStatementName, alteredStatementMasterName, alteredScopeName );
    // log
		#ifdef RADIATION_VERBOSE
			log_io->ReportInstanceOfSubcircuitReplacement(
				simulation_mode->get_alteration_mode()->get_injection_mode(), localDeepLevel,
				instanceOfSubcircuit.get_name(), alteredParameterName, number2String(modificationCounter),
				originalSubcircuit.get_name(), alteredSubcircuit.get_name(),
				localInstanceOfSubcircuitAlteredCircuit->get_name(), alteredFolder,
				alteredStatementName, alteredStatementMasterName, alteredScopeName );
		#endif
		// fgarcia: everything is alright till here
// log_io->ReportGreenStandard("[debug] localInstanceOfSubcircuitAlteredCircuit " + localInstanceOfSubcircuitAlteredCircuit->get_name() );
// log_io->ReportGreenStandard("[debug] instanceOfSubcircuit " + instanceOfSubcircuit.get_name() );
// log_io->ReportGreenStandard("[debug] instanceOfSubcircuit.get_belonging_circuit() " + instanceOfSubcircuit.get_belonging_circuit()->get_name() );
// log_io->ReportGreenStandard("[debug] alteredInstance " + alteredInstance->get_name() + " mastername:" + alteredInstance->get_master_name() );
// log_io->ReportGreenStandard("[debug] export " + localInstanceOfSubcircuitAlteredCircuit->ExportCircuitStatement(""));
		circuit_io_handler->ExportAlteredScenario( radiationSubcircuitsCircuit, *localInstanceOfSubcircuitAlteredCircuit,
			*localAlteredStatementsCircuit, *main_circuit, alteredFolder );
		radiation_spectre_handler->AddScenarioFolderPath( modificationCounter, alteredFolder, pathInject );
		// Remove the alteredSubcircuit, allowing the localAlteredCircuit destruction
		localInstanceOfSubcircuitAlteredCircuit->RemoveStatement( &alteredSubcircuit );
		delete localInstanceOfSubcircuitAlteredCircuit;
		modificationCounter++;
	}
	// Delete localAlteredStatementsCircuit
	// alteredSubcircuitParent can be deleted normally through localAlteredStatementsCircuit destructor
	delete localAlteredStatementsCircuit;
	return success;
}

void CircuitRadiator::CreateRadiationAndAHDLCircuits(){
	// Circuit including the radiation source subcircuit
	// And the altered subcircuit
	radiationSubcircuitsCircuit.set_log_io( log_io );
	radiationSubcircuitsCircuit.set_name( kAlterationSourcesCircuitStringId );

	// Circuit including the radiation source subcircuit
	// And a single instance of it, in order to
	// Create the ahdl compiled sources
	radiationSubcircuitsCircuitAHDL.set_log_io( log_io );
	// radiationSubcircuitsCircuitAHDL should have kMainNetlistFile
	// as name in order to set up correctly the compiled ddbb
	radiationSubcircuitsCircuitAHDL.set_name( kMainNetlistFile );
	// Copy from main circuit
	radiationSubcircuitsCircuitAHDL.basicCopy( main_circuit );
	radiationSubcircuitsCircuitAHDL.MuteNonMainTransientAnalysis();
	// Sources
	if( simulation_mode->get_alteration_mode()->get_injection_mode() ){
		// radiation source subcircuit
		RadiationSourceSubcircuitStatement* radiationSourceSubck = new RadiationSourceSubcircuitStatement(
			*simulation_mode->get_alteration_mode()->get_selected_radiation_source() );
		// Add the source subcircuit to the circuit wrapper (deleted when removing modifiedCircuit)
		radiationSubcircuitsCircuit.AddStatement( radiationSourceSubck );
		// radiation source subcircuit
		RadiationSourceSubcircuitStatement* radiationSourceSubckAHDL = new RadiationSourceSubcircuitStatement(
			*simulation_mode->get_alteration_mode()->get_selected_radiation_source() );
		// Add the source subcircuit to the circuit wrapper (deleted when removing modifiedCircuit)
		radiationSubcircuitsCircuitAHDL.AddStatement( radiationSourceSubckAHDL );
		radiationSubcircuitsCircuitAHDL.AddStatement( GetNewInjectorInstance( *(new Node( kAHDLNodeId, false)), "AHDL_aux_instance") );
		radiationSubcircuitsCircuitAHDL.AddStatement( GetNewRauxInstance( kAHDLNodeId ));
	}else{
		for(std::vector<RadiationSourceSubcircuitStatement*>::iterator
			it_rs = simulation_mode->get_alteration_mode()->get_available_radiation_sources()->begin();
			it_rs != simulation_mode->get_alteration_mode()->get_available_radiation_sources()->end(); it_rs++){
				radiationSubcircuitsCircuit.AddStatement( new RadiationSourceSubcircuitStatement(**it_rs ));
				// AHDL radiation sources have the default export_parameters set to true
				radiationSubcircuitsCircuitAHDL.AddStatement( new RadiationSourceSubcircuitStatement(*(*it_rs)) );
				radiationSubcircuitsCircuitAHDL.AddStatement( GetNewReplacementSourceInstance( *(it_rs) ));
				// Here we need an RAux instance in AHDL circuit
				for(std::vector<Node*>::iterator
					it_n = (*it_rs)->get_nodes()->begin();
					it_n != (*it_rs)->get_nodes()->end(); it_n++){
					radiationSubcircuitsCircuitAHDL.AddStatement(
							GetNewRauxInstance( kAHDLNodePrefix + (*it_rs)->get_name() + "_" + (*it_n)->get_name()) );
				}
		}
	}
	radiationSubcircuitsCircuit.UpdateOwnScope();
	radiationSubcircuitsCircuitAHDL.UpdateOwnScope();
}

CircuitStatement* CircuitRadiator::GetAlteredStatementsCircuit(){
	#ifdef RADIATION_VERBOSE
	log_io->ReportPlain2Log( "New altered circuit");
	#endif
	CircuitStatement* alteredStatements = new CircuitStatement( false );
	alteredStatements->set_circuit_id( kAlteredStatementsCircuitId );
	alteredStatements->set_log_io( log_io );
	alteredStatements->set_name( kAlteredStatementsCircuitStringId );
	alteredStatements->UpdateOwnScope();
	return alteredStatements;
}

InstanceStatement* CircuitRadiator::GetNewInjectorInstance( Node& node,
	std::string radiationSourceName ){
	InstanceStatement* injectorSource = new InstanceStatement();
	injectorSource->set_log_io( log_io );
	injectorSource->set_id( kInjectedSourceId );
	injectorSource->set_name( radiationSourceName );
	injectorSource->set_master_name(
		simulation_mode->get_alteration_mode()->get_selected_radiation_source()->get_name() );
	// We add the node to the injected source but not to the scope
	injectorSource->AddLocalNode( &node );
	return injectorSource;
}

ControlStatement* CircuitRadiator::GetNewSaveInjectionStatement( std::string path2BelongingCircuit ){
	ControlStatement* saveStatement = new ControlStatement();
	saveStatement->set_log_io( log_io );
	saveStatement->set_id( kInjectedSaveMonitorId );
	// saveStatement->set_name( radiationSourceName );
	RadiationSourceSubcircuitStatement* selectedSource =
		simulation_mode->get_alteration_mode()->get_selected_radiation_source();
	saveStatement->set_name( selectedSource->get_name() + kSaveInjectionSufix );
	saveStatement->set_master_name( kSaveMasterName );
	saveStatement->set_advanced_control_statement(false);
	saveStatement->set_special_syntax_control_statement(true);
	// We add the node to the injected source but not to the scope
	saveStatement->set_raw_content( path2BelongingCircuit + kAlterationInjectorName + ":"
		+ selectedSource->get_nodes()->front()->get_name() );
	return saveStatement;
}

InstanceStatement* CircuitRadiator::GetNewReplacementSourceInstance(
	RadiationSourceSubcircuitStatement* rsss ){
	InstanceStatement* rssi = new InstanceStatement();
	rssi->set_log_io( log_io );
	rssi->set_id( kInjectedSourceId );
	rssi->set_name( kInstancePrefix + rsss->get_name() );
	rssi->set_master_name( rsss->get_name() );
	// We add the node to the injected source but not to the scope
	for(std::vector<Node*>::iterator
		it_n = rsss->get_nodes()->begin(); it_n != rsss->get_nodes()->end(); it_n++){
		rssi->AddLocalNode( new Node(
			kAHDLNodePrefix + rsss->get_name() + "_" + (*it_n)->get_name(), false)  );
	}
	return rssi;
}

InstanceStatement* CircuitRadiator::GetNewRauxInstance(
	RadiationSourceSubcircuitStatement* rsss ){
	InstanceStatement* rAuxI = new InstanceStatement();
	rAuxI->set_log_io( log_io );
	rAuxI->set_id( kInjectedSourceId );
	rAuxI->set_name( kRauxSourceId );
	rAuxI->set_master_name( kResistor );
	rAuxI->AddParameter( new Parameter(kRauxParam, kRauxValue) );
	// We add the node to the injected source but not to the scope
	for(std::vector<Node*>::iterator
		it_n = rsss->get_nodes()->begin(); it_n != rsss->get_nodes()->end(); it_n++){
		rAuxI->AddLocalNode(  new Node(
			kAHDLNodePrefix + rsss->get_name() + "_" + (*it_n)->get_name()  , false) );
		rAuxI->AddLocalNode(  new Node("0" , false) );
	}
	return rAuxI;
}

InstanceStatement* CircuitRadiator::GetNewRauxInstance(
	std::string nodeName ){
	InstanceStatement* rAuxI = new InstanceStatement();
	rAuxI->set_log_io( log_io );
	rAuxI->set_id( kInjectedSourceId );
	rAuxI->set_name( kRauxSourceId + "_" + nodeName );
	rAuxI->set_master_name( kResistor );
	rAuxI->AddParameter( new Parameter(kRauxParam, kRauxValue) );
	// We add the node to the injected source but not to the scope
	rAuxI->AddLocalNode(  new Node( nodeName , false) );
	rAuxI->AddLocalNode(  new Node( "0", false) );
	return rAuxI;
}

IncludeStatement* CircuitRadiator::GetNewIncludeStatementOfParametersCircuit( ){
	IncludeStatement* include_subck = new IncludeStatement();
	include_subck->set_log_io( log_io );
	include_subck->SetDefaultIncludeStatementType();
	include_subck->set_raw_content( kParametersCircuitStringId );
	include_subck->set_has_section( false );
	include_subck->set_refered_circuit_id( kParametersCircuitStringId );
	return include_subck;
}

IncludeStatement* CircuitRadiator::GetNewIncludeStatementOfRadiationCircuit( ){
	IncludeStatement* include_subck = new IncludeStatement();
	include_subck->set_log_io( log_io );
	include_subck->SetDefaultIncludeStatementType();
	include_subck->set_raw_content( kAlterationSourcesCircuitStringId );
	include_subck->set_has_section( false );
	include_subck->set_refered_circuit_id( kAlterationSourcesCircuitStringId );
	return include_subck;
}

IncludeStatement* CircuitRadiator::GetNewIncludeStatementOfAlteredStatementsCircuit( ){
	IncludeStatement* include_subck = new IncludeStatement();
	include_subck->set_log_io( log_io );
	include_subck->SetDefaultIncludeStatementType();
	include_subck->set_raw_content( kAlteredStatementsCircuitStringId );
	include_subck->set_has_section( false );
	include_subck->set_refered_circuit_id( kAlteredStatementsCircuitStringId );
	return include_subck;
}

std::string CircuitRadiator::GetRadiatedFolderName( int modificationCounter ){
	return top_folder + kFolderSeparator + kAlterationScenariosFolder
		+ kFolderSeparator + kAlterationScenarioFolderPrefix
		+ "_" + number2String( modificationCounter );
}
