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
	this->plot_critical_parameter_value_evolution = false;
	// injection mode related
	this->has_additional_injection = false;
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
	// Update numruns in MC analyses
	auto numRunParameter = new SimulationParameter( kMCNumRunsParamName, "1",
		true, kNotDefinedInt, kNotDefinedInt, kNotDefinedInt, kNotDefinedInt, kNotDefinedInt );
	AddAdditionalSimulationParameter( numRunParameter );
	log_io->ReportPurpleStandard( "MC numruns set to:" + numRunParameter->get_value() );
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
	if( totalThreads<2 ){
		log_io->ReportError2AllLogs( "No sweept parameter, threads:" + number2String(totalThreads) );
		correctly_simulated = false;
		correctly_processed = false;
		return;
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
	log_io->ReportThread( "Total threads to be simulated: " + number2String(totalThreads)
		+ ". Max number of concurrent profile threads: " + number2String(max_parallel_profile_instances)
		+ ". Max number of concurrent montecarlo threads: " + number2String(max_parallel_montecarlo_instances), 1 );
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
	}else if( golden_metrics_structure== nullptr){
		log_io->ReportError2AllLogs( "nullptr golden_metrics_structure in montecarlo_critical_parameter_nd_parameters_sweep_simulation");
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
		const std::string& currentFolder, const std::vector<unsigned int>& parameterCountIndexes,
		std::vector<SimulationParameter*>& parameters2sweep, const int ndProfileIndex  ){
	std::string s_ndProfileIndex = number2String(ndProfileIndex);
	// Simulation
	MontecarloCriticalParameterValueSimulation* pMCPVS = new MontecarloCriticalParameterValueSimulation();
	pMCPVS->set_has_additional_injection( has_additional_injection );
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
	pMCPVS->set_golden_metrics_structure( golden_metrics_structure );
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
	pMCPVS->set_plot_last_transients( plot_last_transients );
	pMCPVS->set_plot_critical_parameter_value_evolution( plot_critical_parameter_value_evolution );
	pMCPVS->set_interpolate_plots_ratio( interpolate_plots_ratio );
	pMCPVS->set_main_analysis( main_analysis );
	pMCPVS->set_main_transient_analysis( main_transient_analysis );
	// fgarcia: false?, so we analyze later s_xxx_001.tran, s_xxx_002..tran... instead s_xxx.tran
	pMCPVS->set_process_metrics( true );
	pMCPVS->set_export_processed_metrics( export_processed_metrics );
	pMCPVS->set_export_metric_errors( export_metric_errors );
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
	const std::vector<SimulationParameter*>& parameters2sweep  ){
	// a) General Results
	// 1 file: parameter critical value at each profile
	// N files, one per metric/metric, including meanMaxError, maxError, etc
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
	auto auxMetrics = golden_metrics_structure->GetMetricsVector( 0 );
	unsigned int totalAnalizableMetrics = 0;
	for( auto const &m : *auxMetrics ){
		if( m->get_analyzable() ){
			++totalAnalizableMetrics;
		}
	}
	bool partialResults = true;
	partialResults = partialResults && GenerateAndPlotGeneralResults( *auxMetrics,
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
	// init all metric column_indexes
	if( !InitMetricColumnIndexes(*auxMetrics) ){
		log_io->ReportError2AllLogs( "Unexpected error in InitMetricColumnIndexes" );
		return false;
	}
	// reserve memory
	main_nd_simulation_results.ReservePlanesInMemory( CountInvolvedPlanes(parameters2sweep) );
	// create planes
	std::string planesMapsFolder;
	std::string planesGnuplotScriptFolder;
	std::string planesImagesFolder;
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
				planesMapsFolder.clear();
				planesGnuplotScriptFolder.clear();
				planesImagesFolder.clear();
				planesMapsFolder =  mapsFolder + kFolderSeparator + p1->get_file_name() + "_" + p2->get_file_name();
				planesGnuplotScriptFolder =  gnuplotScriptFolder + kFolderSeparator + p1->get_file_name() + "_" + p2->get_file_name();
				planesImagesFolder = imagesFolder + kFolderSeparator + p1->get_file_name() + "_" + p2->get_file_name();
				if( !CreateFolder(planesMapsFolder, true ) || !CreateFolder(planesImagesFolder, true ) || !CreateFolder(planesGnuplotScriptFolder, true ) ){
					log_io->ReportError2AllLogs( k2Tab + "-> Error creating folders: '" + planesMapsFolder + " and " + planesImagesFolder + "'." );
					log_io->ReportError2AllLogs( "Error GenerateAndPlotResults" );
					return false;
				}
				partialResults = partialResults && GenerateAndPlotParameterPairResults(
					*auxMetrics, totalAnalizableMetrics,
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

bool MontecarloCriticalParameterNDParametersSweepSimulation::InitMetricColumnIndexes(
		const std::vector<Metric*>& auxMetrics ){
	// profile outputs
	// #profileCount #Profile #max_critical_parameter_value #min_critical_parameter_value #mean_critical_parameter_value "
	// #MAG_i_name #MAG_i_maxErrorMetric #MAG_i_minErrorMetric #MAG_i_meanMaxErrorMetric"
	// #MAG_i_medianMaxErrorMetric q12 q34 MAG_i_maxMaxErrorGlobal MAG_i_minMaxErrorGlobal MAG_i_meanMaxErrorGlobal\n";
	out_profile_c_i_max = {2};
	out_profile_c_i_min = {3};
	out_profile_c_i_mean = {4};
	// partial plane inputs
	// p1 p2 critParamMax critParamMin critParamMean
	// MAG_i_name MAG_i_maxErrorMetric MAG_i_minErrorMetric MAG_i_meanMaxErrorMetric MAG_i_medianMaxErrorMetric q12 q34
	// MAG_i_maxMaxErrorGlobal MAG_i_minMaxErrorGlobal MAG_i_meanMaxErrorGlobal\n";
	p_p_c_i_max = {2};
	p_p_c_i_min = {3};
	p_p_c_i_mean = {4}; // critical param
	// partial plane outputs
	// p1 p2 critParamMax critParamMin critParamMean
	// MAG_i_name MAG_i_maxErrorMetric MAG_i_minErrorMetric MAG_i_meanMaxErrorMetric MAG_i_medianMaxErrorMetric q12 q34
	// MAG_i_maxMaxErrorGlobal MAG_i_minMaxErrorGlobal MAG_i_meanMaxErrorGlobal\n";
	out_p_c_i_max = {2};
	out_p_c_i_min = {3};
	out_p_c_i_mean = {4};
	unsigned int magCount = 5; // MAG_i_name in input
	for( auto const &m: auxMetrics ){
		if(m->get_analyzable()){
			// for use inside MontecarloCriticalParameterNDParametersSweepSimulation
			p_p_c_i_max.push_back( magCount+1 );
			p_p_c_i_min.push_back( magCount+2 );
			p_p_c_i_mean.push_back( magCount+3 ); // metric
			p_p_c_i_max.push_back( magCount+7 );
			p_p_c_i_min.push_back( magCount+8 );
			p_p_c_i_mean.push_back( magCount+9 ); // global
			// for use outside MontecarloCriticalParameterNDParametersSweepSimulation
			out_profile_c_i_max.push_back( magCount+1 );
			out_profile_c_i_min.push_back( magCount+2 );
			out_profile_c_i_mean.push_back( magCount+3 ); // metric
			out_profile_c_i_max.push_back( magCount+7 );
			out_profile_c_i_min.push_back( magCount+8 );
			out_profile_c_i_mean.push_back( magCount+9 ); // global
			// plane
			out_p_c_i_max.push_back( magCount + 1 );
			out_p_c_i_min.push_back( magCount + 2 );
			out_p_c_i_mean.push_back( magCount + 3 ); // metric
			out_p_c_i_max.push_back( magCount + 7 );
			out_p_c_i_min.push_back( magCount + 8 );
			out_p_c_i_mean.push_back( magCount + 9 ); // global
			// update counters
			magCount += 10; // next MAG_i_name in input
		}
	}
	return true;
}

bool MontecarloCriticalParameterNDParametersSweepSimulation::GenerateAndPlotParameterPairResults(
		const std::vector<Metric*>& auxMetrics, const unsigned int& totalAnalizableMetrics,
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
		partialResults = partialResults && GenerateAndPlotItemizedPlane(
			auxMetrics, p1Index, p2Index, itemizedCount++,
			parameters2sweep, mapsFolder, gnuplotScriptFolder, imagesFolder, *planeIndexes, *planeStructure);
	}
	// add plane
	main_nd_simulation_results.AddPlaneResultsStructure( planeStructure );
	log_io->ReportPlainStandard( "#" + simulation_id + ", Processing itemized results with ResultsProcessor" );
	// Process itemized planes to extract the p1-p2 general plane results
	ResultsProcessor rp;
	std::string generalParameterResultsFile = mapsFolder + kFolderSeparator
		+ planeStructure->get_plane_id() + "_general" + kDataSufix;
	// process statistically
	partialResults = partialResults && rp.StatisticProcessStatisticsFiles(
		planeStructure->get_itemized_data_paths(), generalParameterResultsFile,
		std::move(p_p_c_i_max), std::move(p_p_c_i_min), std::move(p_p_c_i_mean));
	if( !partialResults ){
		log_io->ReportError2AllLogs( "[ERROR ResultsProcessor] Error processing " + generalParameterResultsFile );
		return partialResults;
	}
	planeStructure->set_general_data_path(generalParameterResultsFile);
	// plot parameter critical value and metric errors
	SimulationParameter* p1 = parameters2sweep.at(p1Index);
	SimulationParameter* p2 = parameters2sweep.at(p2Index);
	log_io->ReportPlainStandard("Processed GnuplotPlaneCriticalParam" );
	int gnuplotResult = GnuplotPlaneCriticalParam( *planeStructure, false, *p1, *p2,
		planeStructure->get_plane_id() + "_general_", generalParameterResultsFile,
		gnuplotScriptFolder, imagesFolder );
	gnuplotResult += GnuplotPlaneMetricResults( auxMetrics, *planeStructure,
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
	const std::vector<Metric*>& auxMetrics,
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
		gnuplotMapFile.setf(std::ios::scientific);
		gnuplotMapFile << "#" << p1->get_name() << " " << p2->get_name() << " "
			<< golden_critical_parameter->get_name() << "(max, min and mean)"
			<<" MAG_i_name MAG_i_maxErrorMetric MAG_i_minErrorMetric "
			<< "MAG_i_meanMaxErrorMetric MAG_i_medianMaxErrorMetric q12 q34 MAG_i_maxMaxErrorGlobal MAG_i_minMaxErrorGlobal MAG_i_meanMaxErrorGlobal\n";
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
					<< " " << mcSSim->get_montecarlo_simulation_results()->get_max_critical_parameter_value()
					<< " " << mcSSim->get_montecarlo_simulation_results()->get_min_critical_parameter_value()
					<< " " << mcSSim->get_montecarlo_simulation_results()->get_mean_critical_parameter_value();
				// metrics
				for( auto& m : *(mcSSim->get_montecarlo_simulation_results()->get_metric_montecarlo_results()) ){
					gnuplotMapFile << " " << m->metric_name << " " << m->max_error_metric << " " << m->min_error_metric
						<< " " << m->mean_max_error_metric << " " << m->median_max_error_metric << " " << m->q12_max_error_metric
						<< " " << m->q34_max_error_metric << " " << m->max_max_error_global
						<< " " << m->min_max_error_global<< " " << m->mean_max_error_global;
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
		int gnuplotResult = GnuplotPlaneCriticalParam( plane, true, *p1, *p2,
			"partialPlaneId_" + partialPlaneId, gnuplotMapFilePath,
			gnuplotScriptFolder, imagesFolder );
		gnuplotResult += GnuplotPlaneMetricResults( auxMetrics, plane, true,
			*p1, *p2, itemizedCount, "partialPlaneId_" + partialPlaneId,
			gnuplotMapFilePath, gnuplotScriptFolder, imagesFolder );
		if( gnuplotResult > 0 ){
			log_io->ReportError2AllLogs( "Sim " + simulation_id + ". Unexpected gnuplot result: " + number2String(gnuplotResult) );
		}
	}
	return partialResults;
}

bool MontecarloCriticalParameterNDParametersSweepSimulation::GenerateAndPlotGeneralResults(
		const std::vector<Metric*>& auxMetrics, const std::vector< SimulationParameter*>& parameters2sweep,
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
		gnuplotMapFile.setf(std::ios::scientific);
		gnuplotSpectreErrorMapFile.open( gnuplotSpectreErrorMapFilePath.c_str() );
		gnuplotMapFile << "#profileCount #Profile #max_critical_parameter_value #min_critical_parameter_value #mean_critical_parameter_value "
			<< "#MAG_i_name #MAG_i_maxErrorMetric #MAG_i_minErrorMetric #MAG_i_meanMaxErrorMetric"
			<< "#MAG_i_medianMaxErrorMetric q12 q34 MAG_i_maxMaxErrorGlobal MAG_i_minMaxErrorGlobal MAG_i_meanMaxErrorGlobal\n";
		gnuplotSpectreErrorMapFile << "#profileCount #Profile SpectreError \n";
		unsigned int profileCount = 0;
		bool severalSweepParameter = parameters2sweep.size()>1;
		auto sweepParameter = parameters2sweep.at(0);
		std::string auxIndexes;
		std::string auxSpectreError;
		for( auto const &simulation : *(montecarlo_critical_parameter_value_simulations_vector.get_spectre_simulations()) ){
			MontecarloCriticalParameterValueSimulation* mcSSim = dynamic_cast<MontecarloCriticalParameterValueSimulation*>(simulation);
			auxIndexes = getIndexCode( auxiliarIndexes );
			auxSpectreError = mcSSim->get_correctly_simulated() ? "0" : "1";
			auto auxMCResults = mcSSim->get_montecarlo_simulation_results();
			auto currentMaxCritParamValue = auxMCResults->get_max_critical_parameter_value();
			gnuplotMapFile << std::defaultfloat << ( severalSweepParameter ? profileCount : sweepParameter->GetSweepValue(profileCount) )
				<< " " << auxIndexes << " " << currentMaxCritParamValue
				<< " " << auxMCResults->get_min_critical_parameter_value() << " " << auxMCResults->get_mean_critical_parameter_value();
			// update maxCritParamValue
			maxCritParamValue = currentMaxCritParamValue>maxCritParamValue ? currentMaxCritParamValue : maxCritParamValue;
			// metrics
			for( auto const &m : *(mcSSim->get_montecarlo_simulation_results()->get_metric_montecarlo_results()) ){
				gnuplotMapFile << " " << m->metric_name << " " << m->max_error_metric << " " << m->min_error_metric
					<< " " << m->mean_max_error_metric << " " << m->median_max_error_metric << " " << m->q12_max_error_metric
					<< " " << m->q34_max_error_metric << " " << m->max_max_error_global
					<< " " << m->min_max_error_global<< " " << m->mean_max_error_global;
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
		int gnuplotResult = GnuplotGeneralCritParamValueResults( gnuplotSpectreErrorMapFilePath, gnuplotScriptFolder, imagesFolder );
		gnuplotResult += GnuplotGeneralMetricResults( auxMetrics,
			maxCritParamValue, mapsFolder, gnuplotScriptFolder, imagesFolder );
		if( gnuplotResult > 0 ){
			log_io->ReportError2AllLogs( "Sim " + simulation_id + ". Unexpected gnuplot result: " + number2String(gnuplotResult) );
		}
	}
	return partialResults;
}

int MontecarloCriticalParameterNDParametersSweepSimulation::GnuplotGeneralCritParamValueResults(
	const std::string& gnuplotSpectreErrorMapFilePath,
	const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
	log_io->ReportPlainStandard( k2Tab + "Plot with GnuplotGeneralCritParamValueResults" );
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
	gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight
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
	gnuplotScriptFile << kCustomBorders;
	// line style
	gnuplotScriptFile <<  kTransientSimilarLinesPalette;
	// legend
	gnuplotScriptFile <<  "set key bottom right\n";
	gnuplotScriptFile <<  "ntics = 10\n";
	// Plot
	// add xtics with xtic(2), u 1:3:xtic(2)
	if( correctly_simulated ){
		gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' u 1:5:4:3 axes x1y1 w errorbars ls 1 notitle, \\\n";
		gnuplotScriptFile <<  " '" << gnuplotDataFile << "' u 1:5 axes x1y1 w lp ls 2 title '" << golden_critical_parameter->get_title_name() << "'\n";
	}else{
		gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' u 1:5:4:3 axes x1y1 w errorbars ls 2 title '" << golden_critical_parameter->get_title_name() << "', \\\n";
		gnuplotScriptFile <<  " '" << gnuplotSpectreErrorMapFilePath << "' u 1:5 axes x1y2 with boxes ls 1 notitle\n";
		gnuplotScriptFile <<  " '" << gnuplotDataFile << "' u 1:5 axes x1y1 w lp ls 2 title '" << golden_critical_parameter->get_title_name() << "'\n";
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

int MontecarloCriticalParameterNDParametersSweepSimulation::GnuplotGeneralMetricResults(
	const std::vector<Metric*>& analyzedMetrics, double& maxCritParamValue,
	const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){

	int partialResult = 0;
	unsigned int magCount = 0;
	std::string gnuplotDataFile = main_nd_simulation_results.get_general_data_path();
	for( auto const &m : analyzedMetrics ){
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
			gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight
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
			gnuplotScriptFile << "set ylabel \"Error in metric "  << m->get_title_name() << "\"\n";
			// # remove border on top and right and set color to gray
			gnuplotScriptFile << kCustomBorders << "\n";
			gnuplotScriptFile << "set y2tics\n";
			// palete range
			if( maxCritParamValue < 0 ){
				maxCritParamValue = 1;
				// Palete
				gnuplotScriptFile << kMinimalPalette;
				gnuplotScriptFile << "set y2range [0:1]\n";
				gnuplotScriptFile << "set cbrange [0:1]\n";
			}else{
				// Color Paletes
				gnuplotScriptFile << kUpsetsPalette;
				gnuplotScriptFile << "set y2range [0:"<< maxCritParamValue << "]\n";
				gnuplotScriptFile << "set cbrange [0:"<< maxCritParamValue << "]\n";
			}
			// line style
			gnuplotScriptFile <<  kProfilesPalette;
			// Plot
			// #p1 p2 crit_max crit_min crit_mean  (6)MAG_i_name MAG_i_maxErrorMetric MAG_i_minErrorMetric MAG_i_meanMaxErrorMetric
			// (10)MAG_i_medianMaxErrorMetric q12 q34 MAG_i_maxMaxErrorGlobal MAG_i_minMaxErrorGlobal MAG_i_meanMaxErrorGlobal\n";
			int magDataMetricIndex = out_plane_gnuplot_first_mag_metric_offset + out_data_per_metric_per_line*magCount; // max
			int magDataGlobalIndex = out_plane_gnuplot_first_mag_global_offset + out_data_per_metric_per_line*magCount; // max
			gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' using 1:3 axis x1y2 with filledcurve x1 ls 4 title '" << golden_critical_parameter->get_title_name() << "_max', \\\n";
			gnuplotScriptFile <<  " '" << gnuplotDataFile << "' using 1:5 axis x1y2 with filledcurve x1 ls 6 title '" << golden_critical_parameter->get_title_name() << "_mean', \\\n";
			gnuplotScriptFile <<  " '" << gnuplotDataFile << "' using 1:4 axis x1y2 with filledcurve x1 ls 5 title '" << golden_critical_parameter->get_title_name() << "_min', \\\n";
			// candlesticks  # Data columns: X Min 1stQuartile Median 3rdQuartile Max
			// metric
			gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataMetricIndex+1)  <<  ":" << (magDataMetricIndex+4) << ":" << (magDataMetricIndex+3) << ":" << (magDataMetricIndex+5) << ":" << magDataMetricIndex << " axis x1y1  w candlesticks ls 1 notitle whiskerbars, \\\n";
			gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataMetricIndex+2) << " axis x1y1  w lp ls 2 title '" << m->get_title_name() << " (max_error_metric)', \\\n";
			// global
			gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataGlobalIndex+2)  <<  ":" << (magDataGlobalIndex+1) << ":" << magDataGlobalIndex << " axis x1y1  w errorbars ls 3 notitle, \\\n";
			gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataGlobalIndex+2) << " axis x1y1  w lp ls 3 title '" << m->get_title_name() << " (max_error_global)'\n";
			//
			gnuplotScriptFile << " # Uncomment the following for ploting the median\n";
			gnuplotScriptFile <<  "#     '" << gnuplotDataFile << "' u 1:" << (magDataMetricIndex+3) << " axis x1y1  w lp ls 5 title 'median max_err_" << m->get_title_name() << "'\n";
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
			main_nd_simulation_results.AddGeneralMetricImagePath( outputImagePath , title );
			// Image paths
			// criticalParameterValueSimulationsVector.set_group_processed_image_path(  outputImagePath + kSvgSufix  );
			// criticalParameterValueSimulationsVector.set_group_processed_latex_image_path( outputImagePath+ kLatexOutputImagesSufix  );
			magCount++;
		}
	}
	return partialResult;
}

int MontecarloCriticalParameterNDParametersSweepSimulation::GnuplotPlaneCriticalParam(
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
	gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight << " fname " << kSvgFont << "\n";
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
	gnuplotScriptFile << "set xlabel \""  << p1.get_title_name() << "\"\n";
	gnuplotScriptFile << "set ylabel \""  << p2.get_title_name() << "\"\n";
	gnuplotScriptFile << "set zlabel \"" << golden_critical_parameter->get_title_name() << "\" rotate by 90\n";

	gnuplotScriptFile << "set title \"" << title << "\"\n";
	// Color Paletes
	gnuplotScriptFile << kUpsetsPalette;
	gnuplotScriptFile << k3DProperties;
	if( interpolate_plots_ratio >= 0 ){
		gnuplotScriptFile << "set pm3d interpolate " << interpolate_plots_ratio << "," << interpolate_plots_ratio << "\n";
	}
	gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:3 notitle " << kElegantLine3D << " w pm3d\n";
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

int MontecarloCriticalParameterNDParametersSweepSimulation::GnuplotPlaneMetricResults(
	const std::vector<Metric*>& analyzedMetrics,
	PlaneResultsStructure& plane, const bool isPartialPlane,
	const SimulationParameter& p1, const SimulationParameter& p2,
	const unsigned int& partialPlaneCount,
	const std::string& partialPlaneId, const std::string& gnuplotDataFile,
	const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	unsigned int magCount = 0;
	int partialResults = 0;
	for( auto const &m : analyzedMetrics ){
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
			gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight << " fname " << kSvgFont << "\n";
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
			gnuplotScriptFile << "set xlabel \"" << p1.get_title_name() << "\"\n";
			gnuplotScriptFile << "set ylabel \"" << p2.get_title_name() << "\"\n";
			gnuplotScriptFile << "set zlabel \"" << m->get_title_name() << "\" rotate by 90\n";
			// Offset for xtics
			gnuplotScriptFile << "set ytics left offset 0,-0.5\n";
			// Format
			gnuplotScriptFile << "set format cb \"%g\"\n";
			gnuplotScriptFile << "set title \"" << title << " \"\n";
			// Color Paletes
			gnuplotScriptFile << kUpsetsPalette;
			gnuplotScriptFile << k3DProperties;
			if( interpolate_plots_ratio >= 0 ){
				gnuplotScriptFile << "set pm3d interpolate " << interpolate_plots_ratio << "," << interpolate_plots_ratio << "\n";
			}
			gnuplotScriptFile << kTransientSimilarLinesPalette;

			int magDataMetricIndex = out_plane_gnuplot_first_mag_metric_offset + out_data_per_metric_per_line*magCount; // max
			int magDataGlobalIndex = out_plane_gnuplot_first_mag_global_offset + out_data_per_metric_per_line*magCount; // max

			gnuplotScriptFile << "set multiplot layout 2, 1 \n";

			gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:" << (magDataMetricIndex+1) << " notitle w lp ls 2, \\\n";
			gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << magDataMetricIndex << " notitle w lp ls 1, \\\n";
			gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magDataMetricIndex+2) << " title 'max_err_metric_" << m->get_title_name() << "' " << kElegantLine3D << " w pm3d\n";
			gnuplotScriptFile << "# Uncomment for global error \n";
			gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:" << (magDataGlobalIndex+1) << " notitle w lp ls 2, \\\n";
			gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magDataGlobalIndex) << " notitle w lp ls 1, \\\n";
			gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magDataGlobalIndex+2) << " title 'max_err_global_" << m->get_title_name() << "' " << kElegantLine3D << " w pm3d\n";

			gnuplotScriptFile << "unset multiplot\n";
			gnuplotScriptFile << "unset output\n";
			// close file
			gnuplotScriptFile << "quit\n";
			gnuplotScriptFile.close();

			// Exec comand
			std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
			partialResults += std::system( execCommand.c_str() );
			// Image paths
			if( isPartialPlane ){
				plane.AddItemizedMetricImagePath( partialPlaneCount, outputImagePath, title );
			}else{
				plane.AddGeneralMetricImagePath( outputImagePath, title );
			}
			magCount++;
		}
	}
	return partialResults;
}
