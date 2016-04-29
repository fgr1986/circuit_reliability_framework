 /**
 * @file simulation_modes_handler.hpp
 *
 * @date Created on: Jan 24, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class contain the Simulation configuration
 *
 */


#ifndef SIMULATION_MODES_HANDLER_H
#define SIMULATION_MODES_HANDLER_H

///SimulationModesHandler Class
// c++ std libraries includes
#include <string>
#include <vector>
// Radiation Simulator includes
#include "simulation_mode.hpp"
// radiation io simulator includes
#include "../io_handling/log_io.hpp"
#include "../netlist_modeling/statements/radiation_source_subcircuit_statement.hpp"

class SimulationModesHandler {
public:
	/// Default constructor
	SimulationModesHandler();
	/// Default destructor
	virtual ~SimulationModesHandler();

	/**
	* @brief get critistandard_simulation_modecal_value_mode
	*
	* @return pointer to standard_simulation_mode
	*/
	SimulationMode* get_standard_simulation_mode(){ return &standard_simulation_mode; }

	/**
	* @brief get critical_value_mode
	*
	* @return pointer to critical_value_mode
	*/
	SimulationMode* get_critical_value_mode(){ return &critical_value_mode; }

	/**
	* @brief get critical_parameter_nd_parameters_sweep_mode
	*
	* @return pointer to critical_parameter_nd_parameters_sweep_mode
	*/
	SimulationMode* get_critical_parameter_nd_parameters_sweep_mode(){ return &critical_parameter_nd_parameters_sweep_mode; }

	/**
	* @brief get montecarlo_critical_parameter_nd_parameters_sweep_mode
	*
	* @return pointer to montecarlo_critical_parameter_nd_parameters_sweep_mode
	*/
	SimulationMode* get_montecarlo_critical_parameter_nd_parameters_sweep_mode(){ return &montecarlo_critical_parameter_nd_parameters_sweep_mode; }

	/**
	* @brief get montecarlo_nd_parameters_sweep_mode
	*
	* @return pointer to montecarlo_nd_parameters_sweep_mode
	*/
	SimulationMode* get_montecarlo_nd_parameters_sweep_mode(){ return &montecarlo_nd_parameters_sweep_mode; }

	/**
	* @brief get selected_mode method
	* @return selected_mode
	*/
	SimulationMode* get_selected_mode() const{ return selected_mode; }

	/**
	 * @brief return pointer to injection_alteration_mode
	 * @return injection_alteration_mode
	 */
	AlterationMode* get_injection_alteration_mode() {return &injection_alteration_mode;}

	/**
	 * @brief return pointer to replacement_alteration_mode
	 * @return replacement_alteration_mode
	 */
	AlterationMode* get_replacement_alteration_mode() {return &replacement_alteration_mode;}

	/**
	* @brief get export_other_analysis method
	* @return export_other_analysis <bool>
	*/
	bool get_export_other_analysis() const{ return export_other_analysis; }
	/**
	* @brief Sets log manager
	* @param log_io <LogIO*> log manager
	*/
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }

	/**
	* @brief Simulation mode selection
	* param injectionRadiationSources <std::vector<RadiationSourceSubcircuitStatement*>&> injection sources available
	* param replacementRadiationSources <std::vector<RadiationSourceSubcircuitStatement*>&> replacement sources available
	* param injectableStatements <std::vector<std::string>&> injectable sources available
	* param unalterableStatements <std::vector<std::string>&> unalterable statements available
	* @return true if the user has correctly selected the simulation method.
	*/
	bool RadiationSimulationModeSelection(
	std::vector<std::string>* unalterable_statements, std::vector<std::string>* unalterable_nodes );

	/**
	 * @brief Creates Radiation Simulation and Radiation modes.
	 * @details  Creates Radiation Simulation and Radiation modes.
	 */
	void CreateRadiationProgramModes();

	/**
	 * @brief Creates Radiation Simulation and Radiation modes.
	 * @details  Creates Radiation Simulation and Radiation modes.
	 */
	void CreateVariabilityProgramModes();

	/**
	* @brief Simulation mode selection
	* param injectionRadiationSources <std::vector<RadiationSourceSubcircuitStatement*>&> injection sources available
	* param replacementRadiationSources <std::vector<RadiationSourceSubcircuitStatement*>&> replacement sources available
	* param injectableStatements <std::vector<std::string>&> injectable sources available
	* param unalterableStatements <std::vector<std::string>&> unalterable statements available
	* @return true if the user has correctly selected the simulation method.
	*/
	bool VariabilitySimulationModeSelection( );

private:
	/// Log Manager
	LogIO* log_io;
	/// export_other_analysis
	bool export_other_analysis;
	/// Selected mode
	SimulationMode* selected_mode;
	/// standard mode
	SimulationMode standard_simulation_mode;
	/// kCriticalValueMode
	SimulationMode critical_value_mode;
	/// kParameterSweepMode
	SimulationMode critical_parameter_nd_parameters_sweep_mode;
	/// kMontecarloNDSweepMode
	SimulationMode montecarlo_critical_parameter_nd_parameters_sweep_mode;
	/// kMontecarloNDSweepMode
	SimulationMode montecarlo_nd_parameters_sweep_mode;
	/// Available radiation modes list
	AlterationMode injection_alteration_mode;
	AlterationMode replacement_alteration_mode;
	AlterationMode non_alteration_mode;

	/**
	* @brief Radiation mode selection
	* param injectionRadiationSources <std::vector<RadiationSourceSubcircuitStatement*>&> injection sources available
	* param replacementRadiationSources <std::vector<RadiationSourceSubcircuitStatement*>&> replacement sources available
	* param injectableStatements <std::vector<std::string>&> injectable sources available
	* param unalterableStatements <std::vector<std::string>&> unalterable statements available
	* @return true if the user has correctly selected the simulation method.
	*/
	bool RadiationAlterationModeSelection(
	std::vector<std::string>* unalterable_statements, std::vector<std::string>* unalterable_nodes );

	/**
	* @brief Injection mode selection
	* param injectionRadiationSources <std::vector<RadiationSourceSubcircuitStatement*>&> injection sources available
	* param replacementRadiationSources <std::vector<RadiationSourceSubcircuitStatement*>&> replacement sources available
	* param injectableStatements <std::vector<std::string>&> injectable sources available
	* param unalterableStatements <std::vector<std::string>&> unalterable statements available
	* @return true if the user has correctly selected the simulation method.
	*/
	bool RadiationInjectionModeSelection(
	std::vector<std::string>* unalterable_statements, std::vector<std::string>* unalterable_nodes );

	/**
	 * @brief export_other_analysis selection
	 * @details Select if the analysis present in the netlists should be preserved.
	 * @return true if the analysis present in the netlist should be preserved.
	 */
	bool ExportRadiationAnalysisSelection(
	std::vector<std::string>* unalterable_statements, std::vector<std::string>* unalterable_nodes );
};

#endif /* SIMULATION_MODES_HANDLER_H */
