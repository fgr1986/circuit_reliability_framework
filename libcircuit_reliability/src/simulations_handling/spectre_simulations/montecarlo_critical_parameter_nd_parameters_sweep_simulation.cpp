/**
* @file montecarlor_ND_parameter_sweep_simulation.hpp
*
* @date Created on: March 27, 2014
*
* @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
*
* @section DESCRIPTION
*
* This Class is derived from SpectreSimulation
* and is responsible for the simple sweep simulation mode.
*
*/

// math
#include <cmath>
// Boost
// #include "boost/filesystem.hpp" // includes all needed Boost.Filesystem declarations
// Radiation simulator
#include "montecarlo_critical_parameter_nd_parameters_sweep_simulation.hpp"
#include "../../io_handling/results_processor.hpp"
// constants
#include "../../global_functions_and_constants/gnuplot_constants.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/netlist_words_constants.hpp"
#include "../../global_functions_and_constants/files_folders_io_constants.hpp"

MontecarloCriticalParameterNDParametersSweepSimulation::MontecarloCriticalParameterNDParametersSweepSimulation() {
	this->max_parallel_profile_instances = 10;
	this->plot_last_transients = false;
}

MontecarloCriticalParameterNDParametersSweepSimulation::~MontecarloCriticalParameterNDParametersSweepSimulation(){
	#ifdef DESTRUCTORS_VERBOSE
		std::cout << "MontecarloCriticalParameterNDParametersSweepSimulation destructor. direction:" << this << "\n";
	#endif
	// deleteContentsOfVectorOfPointers( montecarlo_critical_parameter_value_simulations_vector );
	// altered parameters have already been destroyed in each montecarlo_standard_simulation destructor
	// deleteContentsOfVectorOfPointers( altered_parameters_vector );
}

void MontecarloCriticalParameterNDParametersSweepSimulation::RunSimulation( ){
	log_io->ReportPurpleStandard( "Running montecarlo_critical_parameter_nd_parameters_sweep_simulation" );
	if (!TestSetUp()){
		log_io->ReportError2AllLogs( "RunSimulation had not been previously set up. ");
		return;
	}
	// montecarlo_iterations defined in radiation_handler.
	boost::thread_group mainTG;
	// params to be sweeped
	std::vector<SimulationParameter*> parameters2sweep;
	// Threads Creation
	for( auto const &p : *simulation_parameters ){
		if( p->get_allow_sweep() ){
			// Init parameter sweep (increments etc)
			p->InitSweep();
			parameters2sweep.push_back( p );
		}
	}
	 // go through every possible parameter combination
	unsigned int totalThreads = 1;
	for( auto const &p : parameters2sweep ){
		totalThreads = totalThreads*p->get_sweep_steps_number();
	}
	// Environment: maybe ConfigureEnvironmentVariables is required. see critical_parameter_ND_....
	log_io->ReportThread( "Total threads to be simulated: " + number2String(totalThreads) + ". Max number of sweep threads: " + number2String(max_parallel_profile_instances), 1 );
	// parallel threads control
	unsigned int runningThreads = 0;
	unsigned int threadsCount = 0;
	// current parameter sweep indexes
	std::vector<unsigned int> parameterCountIndexes(parameters2sweep.size(), 0);
	montecarlo_critical_parameter_value_simulations_vector.set_group_name("montecarlo_critical_parameter_value_simulations_vector");
	montecarlo_critical_parameter_value_simulations_vector.ReserveSimulationsInMemory( totalThreads );
	while( threadsCount<totalThreads ){
		// wait for resources
		WaitForResources( runningThreads, max_parallel_profile_instances, mainTG, threadsCount );
		// CreateProfile sets all parameter values, and after the simulation object
		// is created it can be updated.
		// Thus, it avoids race conditions when updating parameterCountIndexes and parameters2sweep
		MontecarloCriticalParameterValueSimulation* pMCPVS = CreateProfile(parameterCountIndexes, parameters2sweep, threadsCount);
		if( pMCPVS==nullptr ){
			log_io->ReportError2AllLogs( "Null CreateProfile " + number2String(threadsCount) );
			correctly_simulated = false;
			correctly_processed = false;
			return;
		}
		montecarlo_critical_parameter_value_simulations_vector.AddSpectreSimulation( pMCPVS );
		mainTG.add_thread( new boost::thread(boost::bind(&MontecarloCriticalParameterValueSimulation::RunSimulation, pMCPVS)) );
		// update variables
		UpdateParameterSweepIndexes( parameterCountIndexes, parameters2sweep);
		++threadsCount;
		++runningThreads;
	}
	mainTG.join_all();
	// check if every simulation ended correctly
	correctly_simulated = montecarlo_critical_parameter_value_simulations_vector.CheckCorrectlySimulated();
	correctly_processed = montecarlo_critical_parameter_value_simulations_vector.CheckCorrectlyProcessed();
	#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportPlainStandard( k2Tab + "Generating Map files.");
	#endif
	// generate map files
	GenerateAndPlotResults( parameters2sweep );
	// end
	log_io->ReportPlain2Log( "END OF MontecarloCriticalParameterNDParametersSweepSimulation::RunSimulation" );
}

bool MontecarloCriticalParameterNDParametersSweepSimulation::TestSetUp(){
	if( golden_critical_parameter==nullptr ){
		log_io->ReportError2AllLogs( "nullptr golden_critical_parameter in montecarlo_critical_parameter_simulation");
		return false;
	}
	if( simulation_parameters==nullptr ){
		log_io->ReportError2AllLogs( "nullptr simulation_parameters in montecarlo_critical_parameter_nd_parameters_sweep_simulation");
		return false;
	}else if(folder.compare("")==0){
		log_io->ReportError2AllLogs( "nullptr folder  in montecarlo_critical_parameter_nd_parameters_sweep_simulation");
		return false;
	}else if( golden_magnitudes_structure== nullptr){
		log_io->ReportError2AllLogs( "nullptr golden_magnitudes_structure in montecarlo_critical_parameter_nd_parameters_sweep_simulation");
		return false;
	}
	return true;
}

MontecarloCriticalParameterValueSimulation* MontecarloCriticalParameterNDParametersSweepSimulation::CreateProfile(
	const std::vector<unsigned int>& parameterCountIndexes,
	std::vector<SimulationParameter*>& parameters2sweep, const unsigned int ndProfileIndex ){
	// Create folder
	std::string s_ndProfileIndex = number2String(ndProfileIndex);
	std::string currentFolder = folder + kFolderSeparator
		 + "param_profile_" + s_ndProfileIndex;
	if( !CreateFolder(currentFolder, true ) ){
		 log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '" + currentFolder + "'." );
		 log_io->ReportError2AllLogs( "Error running profile" );
		 return nullptr;
	}
	// copy only files to folder
	// find . -maxdepth 1 -type f -exec cp {} destination_path \;
	std::string copyNetlists0 = "find ";
	std::string copyNetlists1 = " -maxdepth 1 -type f -exec cp {} ";
	std::string copyNetlists2 = " \\;";
	std::string copyNetlists;
	copyNetlists = copyNetlists0 + folder + copyNetlists1 + currentFolder + copyNetlists2;
	if( std::system( copyNetlists.c_str() ) > 0){
		 log_io->ReportError2AllLogs( k2Tab + "-> Error while copying netlist to '" + currentFolder + "'." );
		 log_io->ReportError2AllLogs( "Error running sweep" );
		 return nullptr;
	}
	// create thread
	MontecarloCriticalParameterValueSimulation* pMCPVS = CreateMontecarloCriticalParameterValueSimulation(
		currentFolder, parameterCountIndexes, parameters2sweep, ndProfileIndex );
	if( pMCPVS==nullptr ){
		log_io->ReportError2AllLogs( "pMCPVS is nullptr" );
		return nullptr;
	}
	return pMCPVS;
}

MontecarloCriticalParameterValueSimulation* MontecarloCriticalParameterNDParametersSweepSimulation::CreateMontecarloCriticalParameterValueSimulation(
		const std::string& currentFolder, const std::vector<unsigned int> & parameterCountIndexes,
		std::vector<SimulationParameter*>& parameters2sweep, const int ndProfileIndex  ){
	std::string s_ndProfileIndex = number2String(ndProfileIndex);
	// Simulation
	MontecarloCriticalParameterValueSimulation* pMCPVS = new MontecarloCriticalParameterValueSimulation();
	pMCPVS->set_n_dimensional(true);
	pMCPVS->set_n_d_profile_index(ndProfileIndex);
	pMCPVS->set_is_nested_simulation( true );
	pMCPVS->set_montecarlo_iterations( montecarlo_iterations );
	pMCPVS->set_max_parallel_montecarlo_instances( max_parallel_montecarlo_instances );
	pMCPVS->set_simulation_id(  simulation_id + "_child_prof_" + s_ndProfileIndex );
	// pMCPVS->set_parameter_index( paramIndex );
	// pMCPVS->set_sweep_index( sweepIndex );
	pMCPVS->set_log_io( log_io );
	pMCPVS->set_altered_scenario_index( altered_scenario_index );
	pMCPVS->set_golden_magnitudes_structure( golden_magnitudes_structure );
	// Spectre command and args
	pMCPVS->set_spectre_command( spectre_command );
	pMCPVS->set_pre_spectre_command( pre_spectre_command );
	pMCPVS->set_post_spectre_command( post_spectre_command );
	pMCPVS->set_spectre_command_log_arg( spectre_command_log_arg );
	pMCPVS->set_spectre_command_folder_arg( spectre_command_folder_arg );
	// Spectre environment variables
	pMCPVS->set_ahdl_simdb_env( ahdl_simdb_env );
	pMCPVS->set_ahdl_shipdb_env( ahdl_shipdb_env );
	pMCPVS->set_top_folder( top_folder );
	pMCPVS->set_folder( currentFolder );
	pMCPVS->set_altered_statement_path( altered_statement_path );
	pMCPVS->set_delete_spectre_folders( delete_spectre_folders );
	pMCPVS->set_delete_spectre_transients( delete_spectre_transients );
	pMCPVS->set_delete_processed_transients( delete_processed_transients );
	pMCPVS->set_plot_scatters( plot_scatters );
	pMCPVS->set_plot_transients( plot_transients );
	// pMCPVS->set_plot_last_transients( plot_last_transients );
	pMCPVS->set_interpolate_plots_ratio( interpolate_plots_ratio );
	pMCPVS->set_main_analysis( main_analysis );
	pMCPVS->set_main_transient_analysis( main_transient_analysis );
	// fgarcia: false?, so we analyze later s_xxx_001.tran, s_xxx_002..tran... instead s_xxx.tran
	pMCPVS->set_process_magnitudes( true );
	pMCPVS->set_export_processed_magnitudes( export_processed_magnitudes );
	pMCPVS->set_export_magnitude_errors( export_magnitude_errors );
	// copy of simulation_parameters
	pMCPVS->CopySimulationParameters( *simulation_parameters );
	/// Update golden parameter
	if( !pMCPVS->UpdateGoldenCriticalParameter( *golden_critical_parameter ) ){
		 log_io->ReportError2AllLogs( golden_critical_parameter->get_name()
			  + " not found in CriticalParameterValueSimulation and could not be updated." );
	}
	// update parameter values
	unsigned int sweepedParamIndex = 0;
	for( auto const &p : parameters2sweep ){
		if( !pMCPVS->UpdateParameterValue( *p, number2String( p->GetSweepValue( parameterCountIndexes.at(sweepedParamIndex)))) ){
			log_io->ReportError2AllLogs( p->get_name()
				+ " not found in MontecarloCriticalParameterValueSimulation and could not be updated." );
		}
		++sweepedParamIndex;
	}
	return pMCPVS;
}

bool MontecarloCriticalParameterNDParametersSweepSimulation::GenerateAndPlotResults(
	const std::vector< SimulationParameter* > & parameters2sweep  ){
	// a) General Results
	// 1 file: parameter critical value at each profile
	// N files, one per magnitude/metric, including meanMaxError, maxError, etc
	std::string mapsFolder =  top_folder + kFolderSeparator
		 + kResultsFolder + kFolderSeparator + kResultsDataFolder + kFolderSeparator + kMontecarloCriticalParameterNDParametersSweepResultsFolder + kFolderSeparator
		 + "scenario_" + simulation_id;
	std::string gnuplotScriptFolder =  top_folder + kFolderSeparator
		 + kResultsFolder + kFolderSeparator + kResultsGnuplotScriptsFolder + kFolderSeparator + kMontecarloCriticalParameterNDParametersSweepResultsFolder + kFolderSeparator
		 + "scenario_" + simulation_id;
	std::string imagesFolder =  top_folder + kFolderSeparator
		 + kResultsFolder + kFolderSeparator + kResultsImagesFolder + kFolderSeparator + kMontecarloCriticalParameterNDParametersSweepResultsFolder + kFolderSeparator
		 + "scenario_" + simulation_id;
	if( !CreateFolder(mapsFolder, true ) || !CreateFolder(imagesFolder, true ) || !CreateFolder(gnuplotScriptFolder, true ) ){
		 log_io->ReportError2AllLogs( k2Tab + "-> Error creating folders: '"
		 + mapsFolder + " and " + imagesFolder + "'." );
		 log_io->ReportError2AllLogs( "Error GenerateAndPlotResults" );
		 return false;
	}
	// aux mags
	auto auxMagnitudes = golden_magnitudes_structure->GetMagnitudesVector( 0 );
	unsigned int totalAnalizableMagnitudes = 0;
	for( auto const &m : *auxMagnitudes ){
		if( m->get_analyzable() ){
			++totalAnalizableMagnitudes;
		}
	}
	bool partialResults = true;
	partialResults = partialResults && GenerateAndPlotGeneralResults( *auxMagnitudes,
		parameters2sweep, mapsFolder, gnuplotScriptFolder, imagesFolder );
	if( !partialResults ){
		log_io->ReportError2AllLogs( "Unexpected error in GenerateAndPlotGeneralResults" );
		return false;
	}
	// b) Profile Scatters
	// scatter of each profile already plotted in MontecarloCriticalParameterValueSimulation
	// c) 3d plots
	// p1 vs p2 3d plots
	// std::vector<std::tuple<int,int>>
	std::set<std::pair<unsigned int,unsigned int>> exportedParamTuples;
	// init magnitude_column_indexes
	if( !InitMagnitudeColumnIndexes(*auxMagnitudes) ){
		log_io->ReportError2AllLogs( "Unexpected error in InitMagnitudeColumnIndexes" );
		return false;
	}
	// reserve memory
	main_nd_simulation_results.ReservePlanesInMemory( CountInvolvedPlanes(parameters2sweep) );
	unsigned int p1Index = 0;
	for( auto const &p1 : parameters2sweep ){
		unsigned int p2Index = 0;
		for( auto const &p2 : parameters2sweep ){
			auto auxPair1 = std::make_pair( p1Index, p2Index);
			auto auxPair2 = std::make_pair( p2Index, p1Index);
			if( p1Index!=p2Index &&
				exportedParamTuples.end()==exportedParamTuples.find( auxPair1 ) &&
				exportedParamTuples.end()==exportedParamTuples.find( auxPair2 ) ){
				// generate partial map and results
				#ifdef RESULTS_POST_PROCESSING_VERBOSE
				log_io->ReportPlainStandard("Exporting map: " + p1->get_name() + "-" + p2->get_name() );
				#endif
				std::string planesMapsFolder =  mapsFolder + kFolderSeparator
					 + p1->get_file_name() + "_" + p2->get_file_name();
				std::string planesGnuplotScriptFolder =  gnuplotScriptFolder + kFolderSeparator
					 + p1->get_file_name() + "_" + p2->get_file_name();
				std::string planesImagesFolder = imagesFolder + kFolderSeparator
					 + p1->get_file_name() + "_" + p2->get_file_name();
				if( !CreateFolder(planesMapsFolder, true ) || !CreateFolder(planesImagesFolder, true ) || !CreateFolder(planesGnuplotScriptFolder, true ) ){
					log_io->ReportError2AllLogs( k2Tab + "-> Error creating folders: '"
						+ planesMapsFolder + " and " + planesImagesFolder + "'." );
					log_io->ReportError2AllLogs( "Error GenerateAndPlotResults" );
					return false;
				}
				partialResults = partialResults && GenerateAndPlotParameterPairResults(
					*auxMagnitudes, totalAnalizableMagnitudes,
					p1Index, p2Index, parameters2sweep, planesMapsFolder, planesGnuplotScriptFolder, planesImagesFolder );
				// add parameters to set
				exportedParamTuples.insert(auxPair1);
			}
			++p2Index;
		}
		++p1Index;
	} // end of p1 vs p2 3d plot
	return partialResults;
}

bool MontecarloCriticalParameterNDParametersSweepSimulation::InitMagnitudeColumnIndexes(
		const std::vector<Magnitude*>& auxMagnitudes ){
	// 0   1      2          3                4                    5
	// p1  p2     p_critical value MAG_i_name MAG_i_maxErrorMetric MAG_i_minErrorMetric
	// 6                         7                         8   9   10
	// MAG_i_meanMaxErrorMetric MAG_i_medianMaxErrorMetric q12 q34 MAG_i_maxErrorGlobal
	magnitude_column_indexes = {2}; // parameter critical value
	unsigned int auxMagCount = 3; // MAG_i_name
	for( auto const &m: auxMagnitudes ){
		if(m->get_analyzable()){
			magnitude_column_indexes.push_back( auxMagCount+1 ); // MAG_i_maxErrorMetric
			magnitude_column_indexes.push_back( auxMagCount+2 ); // MAG_i_minErrorMetric
			magnitude_column_indexes.push_back( auxMagCount+3 ); // MAG_i_meanMaxErrorMetric
			magnitude_column_indexes.push_back( auxMagCount+4 ); // MAG_i_medianMaxErrorMetric
			magnitude_column_indexes.push_back( auxMagCount+5 ); // q12
			magnitude_column_indexes.push_back( auxMagCount+6 ); // q34
			magnitude_column_indexes.push_back( auxMagCount+7 ); // MAG_i_maxErrorGlobal
			auxMagCount += 8; // next MAG_i_name
		}
	}
	return true;
}

bool MontecarloCriticalParameterNDParametersSweepSimulation::GenerateAndPlotParameterPairResults(
		const std::vector<Magnitude*>& auxMagnitudes, const unsigned int& totalAnalizableMagnitudes,
		const unsigned int& p1Index, const unsigned int& p2Index,
		const std::vector<SimulationParameter*>& parameters2sweep,
		const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	#ifdef RESULTS_ANALYSIS_VERBOSE
	log_io->ReportPlainStandard("Creating maps for params: " + parameters2sweep.at(p1Index)->get_name()
		+ "-" + parameters2sweep.at(p2Index)->get_name() );
	#endif
	bool partialResults = true;
	auto planes = GetPlanesForParams( p1Index, p2Index, parameters2sweep);
	unsigned int itemizedCount = 0;
	PlaneResultsStructure* planeStructure = new PlaneResultsStructure();
	// resize and reserve memory
	planeStructure->ResizeItemizedPlanesMemory( planes->size() );
	planeStructure->set_plane_id( number2String(p1Index) + "_" + number2String(p2Index) );
	for( auto& planeIndexes : *planes ){
		// debug
		// PrintIndexes( * plane );
		partialResults = partialResults && GenerateAndPlotItemizedPlane(
			auxMagnitudes, p1Index, p2Index, itemizedCount++,
			parameters2sweep, mapsFolder, gnuplotScriptFolder, imagesFolder, *planeIndexes, *planeStructure);
	}
	// add plane
	main_nd_simulation_results.AddPlaneResultsStructure( planeStructure );
	log_io->ReportPlainStandard("Processing itemized results with ResultsProcessor" );
	// Process itemized planes to extract the p1-p2 general plane results
	ResultsProcessor rp;
	std::string generalParameterResultsFile = mapsFolder + kFolderSeparator
		+ planeStructure->get_plane_id() + "_general" + kDataSufix;
	// process only mean
	partialResults = partialResults && rp.MeanProcessResultsFiles(
		planeStructure->get_itemized_data_paths(), generalParameterResultsFile, std::move(magnitude_column_indexes));
	if( !partialResults ){
		log_io->ReportError2AllLogs( "[ERROR ResultsProcessor] Error processing " + generalParameterResultsFile );
		return partialResults;
	}
	planeStructure->set_general_data_path(generalParameterResultsFile);
	// plot parameter critical value and magnitude errors
	SimulationParameter* p1 = parameters2sweep.at(p1Index);
	SimulationParameter* p2 = parameters2sweep.at(p2Index);
	log_io->ReportPlainStandard("Processed GnuplotPlane" );
	int gnuplotResult = GnuplotPlane( *planeStructure, false, *p1, *p2,
		planeStructure->get_plane_id() + "_general_", generalParameterResultsFile,
		gnuplotScriptFolder, imagesFolder );
	gnuplotResult += GnuplotPlaneMagnitudeResults( auxMagnitudes, *planeStructure,
		false, *p1, *p2, 0, "_general_", generalParameterResultsFile,
		gnuplotScriptFolder, imagesFolder );
	if( gnuplotResult > 0 ){
		log_io->ReportError2AllLogs( "Sim " + simulation_id + ".Unexpected gnuplot result: " + number2String(gnuplotResult) );
	}
	// clean
	deletePlaneProfileIndexesStructure( *planes );
	return partialResults;
}

bool MontecarloCriticalParameterNDParametersSweepSimulation::GenerateAndPlotItemizedPlane(
	const std::vector<Magnitude*>& auxMagnitudes,
	const unsigned int& p1Index, const unsigned int& p2Index, const unsigned int& itemizedCount,
	const std::vector<SimulationParameter*>& parameters2sweep,
	const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder,
	const std::vector<unsigned int>& profileIndexesInPlane, PlaneResultsStructure& plane ){
	bool partialResults = true;
	// auxiliar indexes
	std::vector<unsigned int> auxiliarIndexes(parameters2sweep.size(), 0);
	SimulationParameter* p1 = parameters2sweep.at(p1Index);
	SimulationParameter* p2 = parameters2sweep.at(p2Index);
	std::string partialPlaneId = p1->get_file_name() + "_" + p2->get_file_name() + "_" + number2String(itemizedCount);
	// General profiles one
	std::string gnuplotMapFilePath = mapsFolder + kFolderSeparator
		 + simulation_id + "_" + partialPlaneId + kDataSufix;
	// export file
	std::ofstream gnuplotMapFile;
	try {
		gnuplotMapFile.open( gnuplotMapFilePath.c_str() );
		gnuplotMapFile << "#" << p1->get_name() << " " << p2->get_name() << " mean_"
			<< golden_critical_parameter->get_name()
			<<" MAG_i_name MAG_i_maxErrorMetric MAG_i_minErrorMetric "
			<< "MAG_i_meanMaxErrorMetric MAG_i_medianMaxErrorMetric q12 q34 MAG_i_maxErrorGlobal\n";
		unsigned int profileCount = 0;
		unsigned int p1SweepCount = 0;
		unsigned int p2SweepCount = 0;
		for( auto& simulation : *(montecarlo_critical_parameter_value_simulations_vector.get_spectre_simulations()) ){
			if( vectorContains(profileIndexesInPlane, profileCount) ){
				MontecarloCriticalParameterValueSimulation* mcSSim = dynamic_cast<MontecarloCriticalParameterValueSimulation*>(simulation);
				if( p2SweepCount==p2->get_sweep_steps_number() ){
					p2SweepCount = 0;
					++p1SweepCount;
					gnuplotMapFile << "\n";
				}
				gnuplotMapFile << p1->GetSweepValue(p1SweepCount) << " " << p2->GetSweepValue(p2SweepCount++)
					<< " " << mcSSim->get_montecarlo_simulation_results()->get_mean_critical_parameter_value();
				// magnitudes
				for( auto& m : *(mcSSim->get_montecarlo_simulation_results()->get_metric_montecarlo_results()) ){
					gnuplotMapFile << " " << m->metric_magnitude_name << " " << m->max_error_metric << " " << m->min_error_metric
						<< " " << m->mean_max_error_metric << " " << m->median_max_error_metric << " " << m->q12_max_error_metric
						<< " " << m->q34_max_error_metric << " " << m->max_error_global;
				}
				gnuplotMapFile << "\n";
			}// update counters
			++profileCount;
			UpdateParameterSweepIndexes( auxiliarIndexes, parameters2sweep );
		}
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
		partialResults = false;
	}
	// close file
	gnuplotMapFile.close();
	// add data path
	plane.AddItemizedDataPath( gnuplotMapFilePath, partialPlaneId );
	// create gnuplot scatter map graphs
	if( partialResults ){
		// create gnuplot scatter map graphs
		int gnuplotResult = GnuplotPlane( plane, true, *p1, *p2,
			"partialPlaneId_" + partialPlaneId, gnuplotMapFilePath,
			gnuplotScriptFolder, imagesFolder );
		gnuplotResult += GnuplotPlaneMagnitudeResults( auxMagnitudes, plane, true,
			*p1, *p2, itemizedCount, "partialPlaneId_" + partialPlaneId,
			gnuplotMapFilePath, gnuplotScriptFolder, imagesFolder );
		if( gnuplotResult > 0 ){
			log_io->ReportError2AllLogs( "Sim " + simulation_id + ". Unexpected gnuplot result: " + number2String(gnuplotResult) );
		}
	}
	return partialResults;
}

bool MontecarloCriticalParameterNDParametersSweepSimulation::GenerateAndPlotGeneralResults(
		const std::vector<Magnitude*>& auxMagnitudes,
		const std::vector< SimulationParameter*>& parameters2sweep,
		const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	bool partialResults = true;
	// auxiliar indexes
	std::vector<unsigned int> auxiliarIndexes(parameters2sweep.size(), 0);
	// General profiles one
	std::string gnuplotMapFilePath = mapsFolder + kFolderSeparator + "general_s_" + simulation_id + kDataSufix;
	std::string gnuplotSpectreErrorMapFilePath = mapsFolder + kFolderSeparator + "general_spectre_error_s_" + simulation_id + kDataSufix;
	std::ofstream gnuplotMapFile;
	std::ofstream gnuplotSpectreErrorMapFile;
	double maxCritParamValue = 0;
	try {
		gnuplotMapFile.open( gnuplotMapFilePath.c_str() );
		gnuplotSpectreErrorMapFile.open( gnuplotSpectreErrorMapFilePath.c_str() );
		gnuplotMapFile << "#profileCount #Profile #mean_critical_parameter_value MAG_i_name MAG_i_maxErrorMetric MAG_i_minErrorMetric "
			<< "MAG_i_meanMaxErrorMetric MAG_i_medianMaxErrorMetric q12 q34 MAG_i_maxErrorGlobal\n";
		gnuplotSpectreErrorMapFile << "#profileCount #Profile SpectreError \n";
		unsigned int profileCount = 0;
		for( auto const &simulation : *(montecarlo_critical_parameter_value_simulations_vector.get_spectre_simulations()) ){
			MontecarloCriticalParameterValueSimulation* mcSSim = dynamic_cast<MontecarloCriticalParameterValueSimulation*>(simulation);
			std::string auxIndexes = "P";
			std::string auxSpectreError = mcSSim->get_correctly_simulated() ? "0" : "1";
			for ( auto const &i : auxiliarIndexes ){ auxIndexes += number2String(i); }
			double critParamValue = mcSSim->get_montecarlo_simulation_results()->get_mean_critical_parameter_value();
			gnuplotMapFile << profileCount << " " << auxIndexes << " " << critParamValue;
			// update maxCritParamValue
			maxCritParamValue = critParamValue>maxCritParamValue ? critParamValue : maxCritParamValue;
			// magnitudes
			for( auto const &m : *(mcSSim->get_montecarlo_simulation_results()->get_metric_montecarlo_results()) ){
				gnuplotMapFile << " " << m->metric_magnitude_name << " " << m->max_error_metric << " " << m->min_error_metric
					<< " " << m->mean_max_error_metric << " " << m->median_max_error_metric << " " << m->q12_max_error_metric
					<< " " << m->q34_max_error_metric << " " << m->max_error_global;
			}
			gnuplotMapFile << "\n";
			gnuplotSpectreErrorMapFile << profileCount++ << " " << auxIndexes << " " << auxSpectreError << "\n";
			// update counters
			UpdateParameterSweepIndexes( auxiliarIndexes, parameters2sweep );
		}
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
		partialResults = false;
	}
	// close files
	gnuplotMapFile.close();
	gnuplotSpectreErrorMapFile.close();
	main_nd_simulation_results.set_general_data_path(gnuplotMapFilePath);
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
	log_io->ReportPlainStandard( k2Tab + "gnuplotMapFile Created" );
	#endif
	if( partialResults ){
		// create gnuplot scatter map graphs
		int gnuplotResult = GnuplotGeneralResults( gnuplotSpectreErrorMapFilePath, gnuplotScriptFolder, imagesFolder );
		gnuplotResult += GnuplotGeneralMetricMagnitudeResults( auxMagnitudes,
			maxCritParamValue, mapsFolder, gnuplotScriptFolder, imagesFolder );
		if( gnuplotResult > 0 ){
			log_io->ReportError2AllLogs( "Sim " + simulation_id + ". Unexpected gnuplot result: " + number2String(gnuplotResult) );
		}
	}
	return partialResults;
}

int MontecarloCriticalParameterNDParametersSweepSimulation::GnuplotGeneralResults(
	const std::string& gnuplotSpectreErrorMapFilePath,
	const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
	log_io->ReportPlainStandard( k2Tab + "Plot with GnuplotGeneralResults" );
	#endif
	// title
	std::string title = "[General]"  + golden_critical_parameter->get_title_name() + ", " +  simulation_id +  " general_profiles";
	// Files
	std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator
		 + simulation_id + "_general" + "_" + kGnuPlotScriptSufix;
	std::string outputImagePath = imagesFolder + kFolderSeparator
		 + simulation_id + "_general" + kSvgSufix;
	std::string gnuplotDataFile = main_nd_simulation_results.get_general_data_path();
	// Generate scripts
	std::ofstream gnuplotScriptFile;
	gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
	// Svg
	gnuplotScriptFile << "set term svg  size " << kSvgImageWidth << ","<< kSvgImageHeight
		 << " fname " << kSvgFont << "\n";
	gnuplotScriptFile << "set output \"" << outputImagePath << "\"\n";
	gnuplotScriptFile << "set title \" " << title << " \"\n";
	gnuplotScriptFile << "set grid\n";
	// Axis
	gnuplotScriptFile << "set format x \"%g\"\n";
	gnuplotScriptFile << "set format y \"%g\"\n";
	gnuplotScriptFile << "set xlabel \""  << "Profile" << "\"\n";
	gnuplotScriptFile << "set ylabel \" "<< golden_critical_parameter->get_title_name() << " \"\n";
	gnuplotScriptFile << "set y2label \"Spectre error\"\n";

	// # remove border on top and right and set color to gray
	gnuplotScriptFile << "set style line 11 lc rgb '#808080' lt 1\n";
	gnuplotScriptFile << "set border 3 back ls 11\n";
	gnuplotScriptFile << "set tics nomirror\n";
	// line style
	gnuplotScriptFile <<  "set style line 1 lc rgb '#8b1a0e' pt 1 ps 1 lt 1 lw 2 # --- red\n";
	gnuplotScriptFile <<  "set style line 2 lc rgb '#5e9c36' pt 6 ps 1 lt 1 lw 2 # --- green\n";
	gnuplotScriptFile <<  "set style fill solid\n";
	// // Background
	gnuplotScriptFile << kWholeBackground << "\n";
	// legend
	gnuplotScriptFile <<  "set key bottom right\n";
	gnuplotScriptFile <<  "ntics = 10\n";
	// Plot
	// add xtics with xtic(2), u 1:3:xtic(2)
	if( correctly_simulated ){
		gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' u 1:3 axes x1y1 w lp ls 2 title '"
			<< golden_critical_parameter->get_title_name() << "'\n";

	}else{
		gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' u 1:3 axes x1y1 w lp ls 2 title '"
			<< golden_critical_parameter->get_title_name() << "', \\\n";
		gnuplotScriptFile <<  " '" << gnuplotSpectreErrorMapFilePath << "' u 1:3 axes x1y2 with boxes ls 1 notitle\n";
	}
	gnuplotScriptFile <<  "set xtics ntics\n";

	gnuplotScriptFile << "unset output\n";

	// close file
	gnuplotScriptFile << "quit\n";
	gnuplotScriptFile.close();
	// Exec comand
	std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
	// Image paths
	main_nd_simulation_results.set_general_image_path( outputImagePath );
	return std::system( execCommand.c_str() );
}

int MontecarloCriticalParameterNDParametersSweepSimulation::GnuplotGeneralMetricMagnitudeResults(
	const std::vector<Magnitude*>& analyzedMagnitudes, double& maxCritParamValue,
	const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){

	int partialResult = 0;
	unsigned int magCount = 0;
	std::string gnuplotDataFile = main_nd_simulation_results.get_general_data_path();
	for( auto const &m : analyzedMagnitudes ){
		if( m->get_analyzable() ){
			// Files
			std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator
				+ simulation_id + "_general_mag_" + number2String(magCount) + kGnuPlotScriptSufix;
			std::string outputImagePath = imagesFolder + kFolderSeparator
				+ simulation_id + "_general_mag_" + number2String(magCount) + kSvgSufix;
			std::string title = "[General] " + m->get_title_name() + " errors & " + golden_critical_parameter->get_title_name();
			// Generate scripts
			std::ofstream gnuplotScriptFile;
			gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
			// Svg
			gnuplotScriptFile << "set term svg  size " << kSvgImageWidth << ","<< kSvgImageHeight
				<< " fname " << kSvgFont << "\n";
			gnuplotScriptFile << "set output \"" << outputImagePath << "\"\n";
			gnuplotScriptFile << "set title \" " << title << " \"\n";
			gnuplotScriptFile << "set grid\n";
			// Axis
			gnuplotScriptFile << "set format x \"%g\"\n";
			gnuplotScriptFile << "set format y \"%g\"\n";
			gnuplotScriptFile << "set format y2 \"%g\"\n";
			gnuplotScriptFile << "set xlabel \"Profile\"\n";
			gnuplotScriptFile << "set y2label \"" << golden_critical_parameter->get_title_name() << "\"\n";
			gnuplotScriptFile << "set ylabel \"Error in magnitude "  << m->get_title_name() << "\"\n";
			// # remove border on top and right and set color to gray
			gnuplotScriptFile << "set style line 11 lc rgb '#808080' lt 1\n";
			gnuplotScriptFile << "set border 3 back ls 11\n";
			gnuplotScriptFile << "set tics nomirror\n";
			gnuplotScriptFile << "set y2tics\n";
			// palete range
			if( maxCritParamValue < 1 ){
				maxCritParamValue = 1;
				// Palete
				gnuplotScriptFile << kMinimalPalette << "\n";
				gnuplotScriptFile << "set y2range [0:1]\n";
				gnuplotScriptFile << "set cbrange [0:1]\n";
			}else{
				// Color Paletes
				gnuplotScriptFile << kUpsetsPalette << "\n";
				gnuplotScriptFile << "set y2range [0:"<< maxCritParamValue << "]\n";
				gnuplotScriptFile << "set cbrange [0:"<< maxCritParamValue << "]\n";
			}
			// line style
			gnuplotScriptFile <<  "set style line 1 lc rgb '#0060ad' lt 1 lw 1 pt 7 ps 1  # --- blue\n";
			gnuplotScriptFile <<  "set style line 2 lc rgb '#0060ad' lt 1 lw 3 pt 7 ps 1.5  # --- blue\n";
			gnuplotScriptFile <<  "set style line 3 lc rgb '#ffd35a' lt 3 lw 1 pt 9 ps 1  # --- yellow\n";
			gnuplotScriptFile <<  "set style line 4 lc rgb '#ff3a00' lt 3 lw 0 pt 9 ps 1  # --- red\n";
			gnuplotScriptFile <<  "set style line 5 lc rgb '#666666' lt 3 lw 2 pt 9 ps 1  # --- grey \n";
			gnuplotScriptFile <<  "set boxwidth 0.5 relative\n";
			gnuplotScriptFile <<  "set style fill transparent solid 0.5\n";
			// // Background
			gnuplotScriptFile << kWholeBackground << "\n";
			// Plot
			// # 1              2            3      4             5                6              7             8   9   10  11
			// # profCount profile crit_val MAG_i_name MAG_i_maxError MAG_i_minError MAG_i_mean MAG_i_median q12 q34 MAG_i_maxErrorGlobal
			int magDataIndex = 4 + data_per_magnitude_per_line*magCount; // title
			gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' using 1:3 axis x1y2 with filledcurve x1 ls 3 title '"
				<< golden_critical_parameter->get_title_name() << "', \\\n";
			// max min
			gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataIndex+5)  <<  ":" << (magDataIndex+2)
				<< ":" << (magDataIndex+1) << ":" << (magDataIndex+6) << " axis x1y1  w candlesticks ls 1 notitle whiskerbars, \\\n";
			gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataIndex+3) << " axis x1y1  w lp ls 2 title '" << m->get_title_name()
				<< "  (mean_max_error_metric)'\n";
			gnuplotScriptFile << " # Uncomment the following for ploting the median\n";
			gnuplotScriptFile <<  "#     '" << gnuplotDataFile << "' u 1:" << (magDataIndex+4) << " axis x1y1  w lp ls 5 title 'median max_err_"
				<< m->get_title_name() << "'\n";
			gnuplotScriptFile << " # Uncomment the following for ploting upsets region\n";
			gnuplotScriptFile <<  "#     '" << gnuplotDataFile << "' using 1:3 axis x1y2 with filledcurve x2 ls 4 notitle, \\\n";
			// legend
			gnuplotScriptFile <<  "set key top left\n";

			gnuplotScriptFile << "unset output\n";
			// close file
			gnuplotScriptFile << "quit\n";
			gnuplotScriptFile.close();
			// Exec comand
			std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
			partialResult += std::system( execCommand.c_str() );
			// Image paths
			main_nd_simulation_results.AddGeneralMagnitudeImagePath( outputImagePath , title );

			// Image paths
			// criticalParameterValueSimulationsVector.set_group_processed_image_path(  outputImagePath + kSvgSufix  );
			// criticalParameterValueSimulationsVector.set_group_processed_latex_image_path( outputImagePath+ kLatexOutputImagesSufix  );
			magCount++;
		}
	}
	return partialResult;
}

int MontecarloCriticalParameterNDParametersSweepSimulation::GnuplotPlane(
	PlaneResultsStructure& plane, const bool isPartialPlane,
	const SimulationParameter& p1, const SimulationParameter& p2,
	const std::string& partialPlaneId, const std::string& gnuplotDataFile,
	const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	// Files
	std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator
		 + simulation_id + "_" + partialPlaneId + "_" + kGnuPlotScriptSufix;
	std::string outputImagePath = imagesFolder + kFolderSeparator
		 + simulation_id + "_" + partialPlaneId + "_3D" + kSvgSufix;
	std::string title = golden_critical_parameter->get_title_name() + ", " + partialPlaneId;
	// Generate scripts
	std::ofstream gnuplotScriptFile;
	gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
	gnuplotScriptFile << "set term svg  size " << kSvgImageWidth << ","<< kSvgImageHeight << " fname " << kSvgFont << "\n";
	gnuplotScriptFile << "set output \"" << outputImagePath  << "\"\n";
	gnuplotScriptFile << "set grid\n";
	// Axis
	if( p1.get_value_change_mode()!=kSPLineal ){
		gnuplotScriptFile << "set logscale x\n";
	}
	if( p2.get_value_change_mode()!=kSPLineal ){
		gnuplotScriptFile << "set logscale y\n";
	}
	gnuplotScriptFile << "set format x \"%g\"\n";
	gnuplotScriptFile << "set format y \"%g\"\n";
	gnuplotScriptFile << "set mxtics\n";
	gnuplotScriptFile << "set xlabel \""  << p1.get_title_name() << "\"\n";
	gnuplotScriptFile << "set ylabel \""  << p2.get_title_name() << "\"\n";
	gnuplotScriptFile << "set zlabel \"" << golden_critical_parameter->get_title_name() << "\" offset -2.5,0\n";
	// Offset for xtics
	gnuplotScriptFile << "set ytics left offset 0,-0.5\n";

	// Background
	gnuplotScriptFile << kWholeBackground << "\n";
	gnuplotScriptFile << "set title \"" << title << "\"\n";
	gnuplotScriptFile << kTransparentObjects << "\n";
	// linestyle
	gnuplotScriptFile << kElegantLine << "\n";
	// mp3d interpolation and hidden3d
	// mp3d z-offset, interpolation and hidden3d
	gnuplotScriptFile <<  "set ticslevel 0\n";
	gnuplotScriptFile << "set pm3d hidden3d 100\n";
	if( interpolate_plots_ratio >= 0 ){
		gnuplotScriptFile << "set pm3d interpolate " << interpolate_plots_ratio << "," << interpolate_plots_ratio << "\n";
	}else{
		gnuplotScriptFile << "set pm3d corners2color max\n";
	}
	gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:3 notitle w pm3d\n";
	gnuplotScriptFile << "unset output\n";
	// close file
	gnuplotScriptFile << "quit\n";
	gnuplotScriptFile.close();

	// Exec comand
	std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
	// Image paths
	if( isPartialPlane ){
		plane.AddItemizedImagePath( outputImagePath, title );
	}else{
		plane.set_general_image_path( outputImagePath );
	}
	return std::system( execCommand.c_str() );
}

int MontecarloCriticalParameterNDParametersSweepSimulation::GnuplotPlaneMagnitudeResults(
	const std::vector<Magnitude*>& analyzedMagnitudes,
	PlaneResultsStructure& plane, const bool isPartialPlane,
	const SimulationParameter& p1, const SimulationParameter& p2,
	const unsigned int& partialPlaneCount,
	const std::string& partialPlaneId, const std::string& gnuplotDataFile,
	const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	unsigned int magCount = 0;
	int partialResults = 0;
	for( auto const &m : analyzedMagnitudes ){
		if( m->get_analyzable() ){
			// Files
			std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator
				 + simulation_id + "_" + partialPlaneId + "_mag_" + number2String(magCount) + "_" + kGnuPlotScriptSufix;
			std::string outputImagePath = imagesFolder + kFolderSeparator
				 + simulation_id + "_" + partialPlaneId + "_mag_" + number2String(magCount) + "_3D" + kSvgSufix;
			std::string title = m->get_title_name() + ", " + partialPlaneId;
			// Generate scripts
			std::ofstream gnuplotScriptFile;
			gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
			gnuplotScriptFile << "set term svg  size " << kSvgImageWidth << ","<< kSvgImageHeight << " fname " << kSvgFont << "\n";
			gnuplotScriptFile << "set output \"" << outputImagePath  << "\"\n";
			gnuplotScriptFile << "set grid\n";
			// Axix border
			gnuplotScriptFile << "#set border 4095\n";
			// Axis
			if( p1.get_value_change_mode()!=kSPLineal ){
				gnuplotScriptFile << "set logscale x\n";
			}
			if( p2.get_value_change_mode()!=kSPLineal ){
				gnuplotScriptFile << "set logscale y\n";
			}
			gnuplotScriptFile << "set format x \"%g\"\n";
			gnuplotScriptFile << "set format y \"%g\"\n";
			gnuplotScriptFile << "set mxtics\n";
			gnuplotScriptFile << "set xlabel \"" << p1.get_title_name() << "\"\n";
			gnuplotScriptFile << "set ylabel \"" << p2.get_title_name() << "\"\n";
			gnuplotScriptFile << "set zlabel \"" << m->get_title_name() << "\" offset -2.5,0\n";
			// Offset for xtics
			gnuplotScriptFile << "set ytics left offset 0,-0.5\n";
			// Format
			gnuplotScriptFile << "set format cb \"%g\"\n";
			// Color Paletes
			gnuplotScriptFile << kUpsetsPalette << "\n";
			// Background
			gnuplotScriptFile << kWholeBackground << "\n";
			gnuplotScriptFile << "set title \"" << title << " \"\n";
			gnuplotScriptFile << kTransparentObjects << "\n";
			// linestyle
			gnuplotScriptFile << kElegantLine << "\n";
			// mp3d interpolation and hidden3d
			// mp3d z-offset, interpolation and hidden3d
			gnuplotScriptFile <<  "set ticslevel 0\n";
			gnuplotScriptFile << "set pm3d hidden3d 100\n";
			if( interpolate_plots_ratio >= 0 ){
				gnuplotScriptFile << "set pm3d interpolate " << interpolate_plots_ratio << "," << interpolate_plots_ratio << "\n";
			}else{
				gnuplotScriptFile << "set pm3d corners2color max\n";
			}
			int magDataIndex = 4 + data_per_magnitude_per_line*magCount; // title
			gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:" << (magDataIndex+4)
				<< " title 'mean_max_err_" << m->get_title_name() << "' w pm3d\n";
			gnuplotScriptFile << "unset output\n";
			// close file
			gnuplotScriptFile << "quit\n";
			gnuplotScriptFile.close();

			// Exec comand
			std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
			partialResults += std::system( execCommand.c_str() );
			// Image paths
			if( isPartialPlane ){
				plane.AddItemizedMagnitudeImagePath( partialPlaneCount, outputImagePath, title );
			}else{
				plane.AddGeneralMagnitudeImagePath( outputImagePath, title );
			}
			magCount++;
		}
	}
	return partialResults;
}
