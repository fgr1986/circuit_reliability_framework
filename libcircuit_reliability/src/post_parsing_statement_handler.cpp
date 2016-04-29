/**
 * @file post_parsing_statement_handler.cpp
 *
 * @date Created on:Jan 20, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * Main code of PostParsingStatementHandler Class (see post_parsing_statement_handler.hpp)
 *
 */


// radiation simulator includes
#include "post_parsing_statement_handler.hpp"
// constants includes
#include "global_functions_and_constants/global_template_functions.hpp"
#include "global_functions_and_constants/global_constants.hpp"
#include "global_functions_and_constants/statements_constants.hpp"

PostParsingStatementHandler::PostParsingStatementHandler() {
}

PostParsingStatementHandler::~PostParsingStatementHandler() {
}

bool PostParsingStatementHandler::SetUpAnalysisExportation(){
	bool success = true;
	bool exportRegularAnalysis;
	if( (exportRegularAnalysis=simulation_modes_handler->get_export_other_analysis()) ){
		log_io->ReportPlainStandard( kTab +  "Other analysis statements present in the parsed netlist will be preserved.");
	}else{
		log_io->ReportPlainStandard( kTab +  "Omitting other analysis statements present in the parsed netlist.");
	}
	for(std::vector<AnalysisStatement*>::iterator it_a = analysis_statements_found.begin();
				it_a != analysis_statements_found.end(); it_a++){
		(*it_a)->set_essential_analysis( false );
		(*it_a)->set_mute_exportation( !exportRegularAnalysis );
	}
	return success;
}

bool PostParsingStatementHandler::AnalyzeStatementDependency(){
	bool success = true;
	// Dependency Tree
	// Transistors: Already scanned
	log_io->ReportPlain2Log( "There have been found:");
	log_io->ReportPlainStandard( kTab +  "--> " + number2String(model_statements_found.size()) + " models.");
	log_io->ReportPlainStandard( kTab +  "--> " + number2String(analog_model_statements_found.size()) + " analog models.");
	log_io->ReportPlainStandard( kTab +  "--> " + number2String(instance_statements_found.size()) + " instances.");
	bool dependencyFound;
	// Models:
	for(std::vector<ModelStatement*>::iterator it_m = model_statements_found.begin();
				it_m != model_statements_found.end(); it_m++){
		if( !(*it_m)->get_is_a_primitive_model() ){
			dependencyFound = FindDependency( *it_m );
			#ifdef DEPENDENCY_VERBOSE
				if( dependencyFound ){
					log_io->ReportPlain2Log( "model: " + (*it_m)->get_name() + " associated with " + (*it_m)->get_dependency_parent()->get_name() );
				}else{
					log_io->ReportPlain2Log( "model: " + (*it_m)->get_name() + " Not associated with any other statement." );
				}
			#endif
			(*it_m)->set_scanned_for_instances_dependency( true );
		}else{
			#ifdef DEPENDENCY_VERBOSE
				log_io->ReportPlain2Log( "model: " + (*it_m)->get_name() + " is a primitive model" );
			#endif
		}
	}
	log_io->ReportPlainStandard( kTab +  "--> Dependency of models analyzed.");
	// AnalogModels:
	for(std::vector<AnalogModelStatement*>::iterator it_am = analog_model_statements_found.begin();
				it_am != analog_model_statements_found.end(); it_am++){
		if( !(*it_am)->get_is_a_primitive_model() ){
			dependencyFound = FindDependency( *it_am );
			#ifdef DEPENDENCY_VERBOSE
				if( dependencyFound ){
					log_io->ReportPlain2Log( "analog model: " + (*it_am)->get_name() + " associated with " + (*it_am)->get_dependency_parent()->get_name() );
				}else{
					log_io->ReportPlain2Log( "analog model: " + (*it_am)->get_name() + " Not associated with any other statement." );
				}
			#endif
			(*it_am)->set_scanned_for_instances_dependency( true );
		}else{
			#ifdef DEPENDENCY_VERBOSE
				log_io->ReportPlain2Log( "analog model: " + (*it_am)->get_name() + " is a primitive analog model" );
			#endif
		}
	}
	log_io->ReportPlainStandard( kTab +  "--> Dependency of analog models analyzed.");
	// Instances:
	// We already know that (analog) models
	// have its direct-dependency parent defined.
	// We need to know the subcircuit finally instantiated.
	int instancesCount = 0;
	for(std::vector<InstanceStatement*>::iterator it_i = instance_statements_found.begin(); it_i != instance_statements_found.end(); it_i++){
		#ifdef DEPENDENCY_VERBOSE
			log_io->ReportPlain2Log( "Scaning dependency for instance: " + (*it_i)->get_name() );
		#endif
		FindPath2BelongingCircuit( *it_i );
		if( !(*it_i)->get_is_a_primitive_instance() ){
			dependencyFound = FindDependency( *it_i );
			#ifdef DEPENDENCY_VERBOSE
				if( dependencyFound ){
					log_io->ReportPlain2Log( "model: " + (*it_i)->get_name() + " associated with " + (*it_i)->get_dependency_parent()->get_name() );
				}else{
					log_io->ReportPlain2Log( "model: " + (*it_i)->get_name() + " Not associated with any other statement." );
				}
			#endif
			if( dependencyFound ){
				if (!FindGlobalDependencyParent( *it_i  )){
					log_io->ReportRedStandard( "GLOBAL Dependency not found for  " + ( *it_i )->get_name()  );
				}
			}else{
				log_io->ReportRedStandard( "Dependency not found for  " + ( *it_i )->get_name()  );
			}
			(*it_i)->set_scanned_for_instances_dependency( true );
		}else{
			#ifdef DEPENDENCY_VERBOSE
				log_io->ReportPlain2Log( "instance: " + (*it_i)->get_name() + " is a primitive instance" );
			#endif
		}
		instancesCount++;
	}
	log_io->ReportPlainStandard( kTab +  "--> Dependency of instances analyzed.");
	return success;
}

void PostParsingStatementHandler::FindPath2BelongingCircuit( InstanceStatement* instance ){
	Statement* parent = instance->get_parent();
	std::string path = instance->get_name();
	std::string formalPath = instance->get_name();
	while( parent->get_statement_type() != kCircuitStatement && parent!=nullptr ){
		if( parent->get_statement_type() == kSubcircuitStatement ){
			path = "[ " + parent->get_name() + "]." + path;
		}else{
			formalPath = parent->get_name() + "." + formalPath;
			path = parent->get_name() + "." + path;
		}
		parent = parent->get_parent();
	}
	instance->set_path_to_belonging_circuit( path );
	instance->set_formal_path_to_belonging_circuit( formalPath );
}

bool PostParsingStatementHandler::FindGlobalDependencyParent( InstanceStatement* instanceStatement ){
	Statement* dependencyParent = instanceStatement->get_dependency_parent();
	bool globalDependencyParentFound = false;
	bool dependencyParentNull = false;
	while( !globalDependencyParentFound && !dependencyParentNull){
		if (dependencyParent==nullptr){
			dependencyParentNull = true;
		}else{
			switch (dependencyParent->get_statement_type()) {
				case kSubcircuitStatement:
					globalDependencyParentFound = true;
					break;
				case kModelStatement:
					if(! (globalDependencyParentFound =
						((ModelStatement *)dependencyParent)->get_is_a_primitive_model()) ){
						dependencyParent = dependencyParent->get_dependency_parent();
					}
					break;
				case kAnalogModelStatement:
					if(! (globalDependencyParentFound =
						((AnalogModelStatement *)dependencyParent)->get_is_a_primitive_model()) ){
						dependencyParent = dependencyParent->get_dependency_parent();
					}
					break;
				case kInstanceStatement:
					if(! (globalDependencyParentFound =
						((InstanceStatement *)dependencyParent)->get_is_a_primitive_instance()) ){
						dependencyParent = dependencyParent->get_dependency_parent();
					}
					break;
				default:
					dependencyParent = dependencyParent->get_dependency_parent();
				break;
			}
		}
	}
	if( !dependencyParentNull && dependencyParent->get_statement_type()==kSubcircuitStatement  ){
		( (SubcircuitStatement*)dependencyParent )->AddProgeny( instanceStatement );
		instanceStatement->set_global_dependency_parent( dependencyParent );
		// Test injection/replacement
		if(!instanceStatement->get_can_be_injected()){
			instanceStatement->set_can_be_injected(
				instanceStatement->TestCanBeInjected( dependencyParent->get_master_name()) );
		}
		if(!instanceStatement->get_can_be_substituted()){
			std::string auxSubstituteMasterName;
			instanceStatement->set_can_be_substituted(
				instanceStatement->TestCanBeSubstituted( dependencyParent->get_master_name(), auxSubstituteMasterName) );
			instanceStatement->set_substitute_master_name( auxSubstituteMasterName );
		}
	}else if( dependencyParentNull ){
		log_io->ReportRedStandard( instanceStatement->get_name() + "GlobalDependencyParent is null" );
	}else{
		log_io->ReportRedStandard("GlobalDependencyParent type of" + instanceStatement->get_name()
			+ "not an subcircuit, instead is type: " +  number2String( dependencyParent->get_statement_type()) );
	}
	return globalDependencyParentFound;
}

bool PostParsingStatementHandler::FindDependency( Statement* statement ){
	bool found = false;
	if( statement->get_consider_instances_dependency() ){
		Statement* dependencyParent = statement;
		for(std::vector<SubcircuitStatement*>::iterator it_s = subcircuit_statements_found.begin();
					it_s != subcircuit_statements_found.end(); it_s++){
			if( (*it_s)->get_name().compare(statement->get_master_name())==0 ){
				dependencyParent = *it_s;
				found = true;
			}
		}
		for(std::vector<AnalogModelStatement*>::iterator it_am = analog_model_statements_found.begin();
					it_am != analog_model_statements_found.end(); it_am++){
			if( (*it_am)->get_name().compare(statement->get_master_name())==0 ){
				dependencyParent = *it_am;
				found = true;
			}
		}
		for(std::vector<ModelStatement*>::iterator it_m = model_statements_found.begin();
					it_m != model_statements_found.end(); it_m++){
			if( (*it_m)->get_name().compare(statement->get_master_name())==0 ){
				dependencyParent = *it_m;
				found = true;
			}
		}
		if( dependencyParent==statement ){
			log_io->ReportRedStandard( kTab +  "--> Statement dependencyParent not found for " + statement->get_name() + " "
				+ statement->get_master_name() );
			log_io->ReportRedStandard( kTab +  "--> This statement will be treated as non dependent on others" );
			statement->set_dependency_parent( statement );
			statement->set_global_dependency_parent( statement );
		}else{
			statement->set_dependency_parent( dependencyParent );
			dependencyParent->AddDescendant( statement );
		}
		statement->set_scanned_for_instances_dependency( true );
	}
	return found;
}

// Subcircuit listings
void PostParsingStatementHandler::AddSubcircuitStatementFound( SubcircuitStatement* subcircuit_statement_found ){
	subcircuit_statements_found.push_back( subcircuit_statement_found );
	primitive_statements_found.push_back( subcircuit_statement_found );
}

// Model listings

void PostParsingStatementHandler::AddAnalogModelStatementFound( AnalogModelStatement* analog_model_statement_found ){
	analog_model_statements_found.push_back( analog_model_statement_found );
	if( analog_model_statement_found->get_is_a_primitive_model() ){
		primitive_statements_found.push_back( analog_model_statement_found );
	}
}

void PostParsingStatementHandler::AddModelStatementFound( ModelStatement* model_statement_found ){
	model_statements_found.push_back( model_statement_found );
	if( model_statement_found->get_is_a_primitive_model() ){
		primitive_statements_found.push_back( model_statement_found );
	}
}

// Instances listings

void PostParsingStatementHandler::AddInstanceStatementFound( InstanceStatement* instance_statement_found ){
	instance_statements_found.push_back( instance_statement_found );
	if( instance_statement_found->get_is_a_primitive_instance() ){
		primitive_statements_found.push_back( instance_statement_found );
	}
}

void PostParsingStatementHandler::AddTransistorStatementFound( TransistorStatement* transistor_statement_found ){
	transistor_statements_found.push_back( transistor_statement_found );
	if( transistor_statement_found->get_is_a_primitive_instance() ){
		primitive_statements_found.push_back( transistor_statement_found );
	}
}

void PostParsingStatementHandler::AddAnalysisStatementFound( AnalysisStatement* analysis_statement_found ){
	analysis_statements_found.push_back( analysis_statement_found );
}
