/*
 * transistor.h
 *
 *  Created on: Feb 20, 2013
 *      Author: fernando
 */



#ifndef SIMULATION_PARAMETER_H
#define SIMULATION_PARAMETER_H

#include <string>
#include "parameter.hpp"

class SimulationParameter : public Parameter{
public:
	SimulationParameter();
	SimulationParameter( const std::string& name, const std::string& value, const bool& fixed,
		const double& value_min, const double& value_max,
		const double& value_stop_percentage,
		const int& value_change_mode,
		const unsigned int& sweep_steps_number );
	SimulationParameter(const SimulationParameter& orig);
	virtual ~SimulationParameter();

	bool get_fixed() const {return fixed;}
	bool get_golden_fixed() const {return golden_fixed;}
	bool get_allow_find_critical_value() const {return allow_find_critical_value;}
	double get_value_max() const {return value_max;}
	double get_value_min() const {return value_min;}
	double get_value_stop_percentage() const {return value_stop_percentage;}
	int get_value_change_mode() const{ return value_change_mode; }

	void set_fixed(bool fixed) { this->fixed = fixed; }
	void set_golden_fixed(bool golden_fixed) { this->golden_fixed = golden_fixed; }
	void set_allow_find_critical_value(bool allow_find_critical_value) {
		this->allow_find_critical_value = allow_find_critical_value; }

	void set_value_max( double value_max ) { this->value_max = value_max; }
	void set_value_min( double value_min ) { this->value_min = value_min; }
	void set_value_stop_percentage( double value_stop_percentage ) { this->value_stop_percentage = value_stop_percentage; }
	void set_value_change_mode( int value_change_mode ){ this->value_change_mode = value_change_mode;}

	void set_sweep_steps_number( unsigned  int sweep_steps_number ){
		this->sweep_steps_number = sweep_steps_number; }

	unsigned int get_sweep_steps_number() const{ return sweep_steps_number; }

	bool get_allow_sweep();

	std::string ExportSimulationParameter();

	/**
	 * @brief Computes next value in series
	 *
	 * @param currentStep
	 */
	double GetSweepValue(const int currentStep );

	/**
	 * @brief Retrieve the parameter info
	 * @details  Retrieve the parameter info
	 * @return string with main info
	 */
	virtual std::string GetInfo();

	/**
	 * @brief Inits sweep
	 */
	void InitSweep( );

private:
	bool fixed;
	bool golden_fixed;
	bool allow_find_critical_value;
	double value_min;
	double value_max;
	double value_stop_percentage;
	// double value_step;
	int value_change_mode;
	unsigned int sweep_steps_number;

	bool positive;
	double lineal_inc;
	double log_inc;
	double abs_a_b;
};

#endif /* SIMULATION_PARAMETER_H_ */
