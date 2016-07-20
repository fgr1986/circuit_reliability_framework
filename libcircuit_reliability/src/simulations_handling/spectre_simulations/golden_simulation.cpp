 /**
 * @file golden_simulation.cpp
 *
 * @date Created on: March 27, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * GoldenSimulation Class Body
 *
 */

// Radiation simulator
#include "golden_simulation.hpp"

#include "../../io_handling/raw_format_processor.hpp"

#include "../../metric_modeling/ocean_eval_metric.hpp"
// constants
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/files_folders_io_constants.hpp"
#include "../../global_functions_and_constants/gnuplot_constants.hpp"

GoldenSimulation::GoldenSimulation() {
	this->export_processed_metrics = true;
	this->singular_results_path = kNotDefinedString;
	// injection mode related
	this->has_additional_injection = false;
}

GoldenSimulation::~GoldenSimulation(){
	// only if the vector has been created
	#ifdef DESTRUCTORS_VERBOSE
	   std::cout<< "GoldenSimulation destructor. direction:" << this << "\n";
	#endif
	if( process_metrics && processed_metrics!= nullptr ){
		#ifdef DESTRUCTORS_VERBOSE
			std::cout<< "Deleting processed_metrics\n";
		#endif
		deleteContentsOfVectorOfPointers( *processed_metrics );
		delete processed_metrics;
		#ifdef DESTRUCTORS_VERBOSE
			std::cout<< "processed_metrics deleted\n";
		#endif
	}
}

void GoldenSimulation::RunSimulation( ){
	if (!TestSetUp()){
		log_io->ReportError2AllLogs( "RunSimulation had not been previously set up. ");
		return;
	}
	ConfigureEnvironmentVariables();
	ShowEnvironmentVariables();
	if (simulation_parameters==nullptr){
		log_io->ReportError2AllLogs( "simulation_parameters is nullptr. ");
		return;
	}
	if( !ExportParametersCircuit( folder, n_d_profile_index )){
		log_io->ReportError2AllLogs( "Error creating parameters Circuit ");
		return;
	}
	// Register Parameters
	transient_simulation_results.RegisterSimulationParameters(simulation_parameters);
	transient_simulation_results.set_spectre_result( RunSpectre() );
	if( correctly_simulated && process_metrics ){
		// process
		// log_io->ReportPlain2Log( k2Tab + "#" + simulation_id + " scenario: processing metrics.");
		// Set up metrics
		// log_io->ReportPlain2Log( k3Tab + "#" + simulation_id + " scenario: creating metrics from golden.");
		processed_metrics = CreateGoldenMetricsVector();
		// log_io->ReportPlain2Log( k3Tab + "#" + simulation_id + " scenario: processing results.");
		// process spectre results
		if( !ProcessSpectreResults( folder, simulation_id, transient_simulation_results, true, *processed_metrics, true, false ) ){
			log_io->ReportError2AllLogs( "Error while processing GOLDEN spectre_results. Scenario #" + simulation_id );
			return;
		}
		// if simple golden simulation (not sweep)
		if(!is_nested_simulation){
			golden_metrics_structure = new NDMetricsStructure();
			golden_metrics_structure->SimpleInitialization( 1, *processed_metrics, singular_results_path );
		}
		// plot transiets if required
		if( correctly_processed && plot_transients ){
			// log_io->ReportPlain2Log( k3Tab + "Plotting #" + simulation_id + " scenario.");
			CreateGoldenGnuplotTransientImages();
		}
		// delete previous transients, if needed
		if( !ManageIndividualResultFiles( transient_simulation_results, true ) ){
			log_io->ReportError2AllLogs( "Error deleting raw data. Golden Scenario #" + simulation_id );
			// return false; // program can continue
		}
	}
}

bool GoldenSimulation::TestSetUp(){
	if( folder.compare("")==0 ){
		log_io->ReportError2AllLogs( "nullptr folder ");
		return false;
	}
	return true;
}

int GoldenSimulation::RunSpectre(){
	std::string execCommand = spectre_command + " "
		+ spectre_command_log_arg + " " + folder + kFolderSeparator + kSpectreLogFile + " "
		+ spectre_command_folder_arg + " " + folder + kFolderSeparator + kSpectreResultsFolder + " "
		+ folder + kFolderSeparator + kMainNetlistFile
		+ " " + post_spectre_command + " " + folder + kFolderSeparator + kSpectreStandardLogsFile;
	int spectre_result = std::system( execCommand.c_str() ) ;
	if( spectre_result>0 ){
		correctly_simulated = false;
		log_io->ReportError2AllLogs( "Unexpected Spectre spectre_result for singular scenario #" + simulation_id + ": spectre output = " + number2String(spectre_result) );
		log_io->ReportError2AllLogs( "Spectre Log Folder " + folder );
		return spectre_result;
	}
	correctly_simulated = true;
	singular_results_path = top_folder + kFolderSeparator
		+ kResultsFolder + kFolderSeparator + kResultsDataFolder + kFolderSeparator + kTransientResultsFolder + kFolderSeparator
		+ simulation_id + "_" + kProcessedTransientFile;
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
	log_io->ReportGreenStandard( k2Tab + "#" + simulation_id + " scenario: ENDED." + " spectre_result=" + number2String(spectre_result) );
	#endif
	return spectre_result;
}

NDMetricsStructure* GoldenSimulation::GetGoldenMetrics(){
	NDMetricsStructure* gms = new NDMetricsStructure(*golden_metrics_structure);
	return gms;
}

std::vector<Metric*>* GoldenSimulation::CreateGoldenMetricsVector(){
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "#" + simulation_id + " scenario: Setting up Metrics" );
	#endif
	if( metrics_2be_found==nullptr ){
		log_io->ReportError2AllLogs( "null metrics_2be_found in CreateGoldenMetricsVector, simulation_id:" + simulation_id );
		return nullptr;
	}
	std::vector<Metric*>* parameterMetrics = new std::vector<Metric*>();
	for( auto const &m : *metrics_2be_found ){
		// Create an empty copy of each metric
		if( m->is_transient_magnitude() ){
			auto pMag = static_cast<Magnitude*>( m );
			parameterMetrics->push_back( new Magnitude(*pMag, false) );
		}else{
			auto pOceanEvalMag = static_cast<OceanEvalMetric*>( m );
			parameterMetrics->push_back( new OceanEvalMetric(* pOceanEvalMag) );
		}
	}
	return parameterMetrics;
}


int GoldenSimulation::CreateGoldenGnuplotTransientImages(){

	std::string gnuplotScriptFilePath = top_folder + kFolderSeparator
		+ kResultsFolder + kFolderSeparator + kResultsGnuplotScriptsFolder + kFolderSeparator + kTransientResultsFolder + kFolderSeparator
		+ simulation_id + kGnuPlotScriptSufix;

	std::string outputImagePath = top_folder + kFolderSeparator
		+ kResultsFolder + kFolderSeparator + kResultsImagesFolder + kFolderSeparator
		+ kTransientResultsFolder + kFolderSeparator
		+ simulation_id;

	// static script
	std::ofstream gnuplotScriptFile;
	gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
	gnuplotScriptFile << "set grid\n";
	// Axis
	gnuplotScriptFile << "set format x \"%g\"\n";
	gnuplotScriptFile << "set format y \"%g\"\n";

	// # remove border on top and right and set color to gray
	gnuplotScriptFile << "set style line 11 lc rgb '#808080' lt 1\n";
	gnuplotScriptFile << "set border 3 back ls 11\n";
	gnuplotScriptFile << "set tics nomirror\n";
	// line style
	gnuplotScriptFile <<  "set style line 1 lc rgb '#5e9c36' pt 6 ps 1 lt 1 lw 2 # --- green\n";
	// plots
	int indexCount = 2;
	for(std::vector<Metric*>::iterator it_mg = ++(processed_metrics->begin());
		it_mg != processed_metrics->end(); ++it_mg){
		if( (*it_mg)->is_transient_magnitude() ){
			auto pMag = static_cast<Magnitude*>(*it_mg);

			if( pMag->get_plottable()  && pMag->get_found_in_results() ){
				std::string mgName = (*it_mg)->get_title_name();
				// Svg
				gnuplotScriptFile <<  "set term svg noenhanced size "<< kSvgImageWidth << ","
					<< kSvgImageHeight << " fname " << kSvgFont << "\n";
				gnuplotScriptFile <<  "set output \"" << outputImagePath << "_m_"
					<< mgName << kGnuplotTransientSVGSufix << "\"\n";
				// metrics
				gnuplotScriptFile <<  "set xlabel \"time\"\n";
				gnuplotScriptFile <<  "set ylabel \"" << mgName <<  "\"\n";
				gnuplotScriptFile <<  "set title \"" << mgName << " \"\n";
				gnuplotScriptFile <<  "set key bottom right\n";
				// Plot
				gnuplotScriptFile <<  "plot '" << transient_simulation_results.get_processed_file_path() << "' u 1:"
					+ number2String(indexCount) << " w lp ls 1 title 'golden'\n";
				gnuplotScriptFile << "unset output\n";
				// Add images path
				transient_simulation_results.AddTransientImage(
					outputImagePath + "_m_" + mgName + kGnuplotTransientSVGSufix, (*it_mg)->get_name() );
			}else if( pMag->get_plottable() ){
				log_io->ReportError2AllLogs( (*it_mg)->get_name() + " not found in results." );
			}
		}
		indexCount++;
	}
	// close file
	gnuplotScriptFile << "\nquit\n";
	gnuplotScriptFile.close();

	std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
	return std::system( execCommand.c_str() );
}
