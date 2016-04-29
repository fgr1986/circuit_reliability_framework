 /**
 * @file id.hpp
 *
 * @date Created on: Jan 2, 2014
 * 
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 * 
 * @section DESCRIPTION
 * 
 * This Class contains the statements regarding a simulation mode
 * 		+ Simulation mode id
 * 		+ Radiation Mode
 * 		+ Analysis Statement
 * 		+ Control Statements
 * 
 */


#ifndef SIMULATION_MODE_H
#define SIMULATION_MODE_H

///SimulationMode Class
// c++ std libraries includes
#include <string>
#include <vector>
// radiation io simulator includes
#include "alteration_mode.hpp"
#include "../io_handling/log_io.hpp"
// Netlist modeling includes
#include "../netlist_modeling/statements/analysis_statement.hpp"
#include "../netlist_modeling/statements/control_statement.hpp"

class SimulationMode {
public:	
	/// Default constructor
	SimulationMode( );
	/// Default constructor
	// SimulationMode( int id_type );
	/// Default destructor
	virtual ~SimulationMode();

	/**
 	* @details sets log_io
 	* @param log_io <LogIO*> 
 	*/
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }
	
	/**
	 * @brief sets short_description
	 * 
	 * @param short_description
	 */
	void set_short_description( std::string short_description){ this->short_description = short_description;}

	/**
	 * @brief gets short_description
	 * @return short_description
	 */
	std::string get_short_description() const{ return short_description; }
	
	/**
	 * @brief sets description
	 * 
	 * @param description
	 */
	void set_description( std::string description){ this->description = description;}

	/**
	 * @brief gets description
	 * @return description
	 */
	std::string get_description() const{ return description; }

	/**
	 * @brief gets analysis_statement
	 * @return analysis_statement
	 */
	AnalysisStatement* get_analysis_statement(){ return analysis_statement; }

	/**
	 * @brief set analysis_statement
	 * 
	 * @param analysis_statement Analysis statement
	 */
	void set_analysis_statement( AnalysisStatement* analysis_statement){
		this->analysis_statement = analysis_statement; }

	/**
	 * @brief gets main_transient_analysis
	 * @return main_transient_analysis
	 */
	AnalysisStatement* get_main_transient_analysis(){ return main_transient_analysis; }

	/**
	 * @brief set main_transient_analysis
	 * 
	 * @param main_transient_analysis Analysis statement
	 */
	void set_main_transient_analysis( AnalysisStatement* main_transient_analysis){
		this->main_transient_analysis = main_transient_analysis; }

	/**
	 * @brief gets control_statements
	 * @return control_statements vector
	 */
	std::vector<ControlStatement*>* get_control_statements() {return &control_statements;}
	
	/**
	 * @brief set the id
	 * 
	 * @param id AlterationMode
	 */
	void set_id( int id){
		this->id = id;
	}

	/**
 	* @brief get_id method
 	* @return id <int>
 	*/
 	int get_id() const{ return id;}

	/**
	 * @brief set the alteration_mode
	 * @details sets alteration_mode
	 * 
	 * @param alteration_mode AlterationMode
	 */
	void set_alteration_mode( AlterationMode* alteration_mode){
		this->alteration_mode = alteration_mode;
	}

		/**
	 * @brief set the alteration_mode
	 * @details sets alteration_mode
	 * 
	 * @param alteration_mode AlterationMode
	 */
	AlterationMode* get_alteration_mode(){ return alteration_mode; }
	
	/**
	 * @details add a control statement to the list
	 * 
	 * @param controlStatement Control Statement
	 */
	void AddControlStatement(
		ControlStatement* controlStatement );
	/**
	 * @brief select a radiation source from the list using its index
	 * 
	 * @param index 
	 */
	void SelectRadiationSource( int index);

private:
	/// simulation mode
	int id;
	/// Short Description
	std::string short_description;
	/// Description
	std::string description;
	/// Logger
	LogIO* log_io;
	/// Radiation mode
	AlterationMode* alteration_mode;
	/// Main Analysis statement associated with sensitive nodes detection.
	/// Can nest other analysis
	AnalysisStatement* analysis_statement;
	/// Main Transient Analysis
	AnalysisStatement* main_transient_analysis;
	/// Control statements associated with sensitive nodes detection.
	std::vector<ControlStatement*> control_statements;
};

#endif /* SIMULATION_MODE_H */
