/**
 * @file magnitude.hpp
 *
 * @date Created on: December 16, 2013
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class is responsible of the simulations results files as a web page.
 * Several html web pages are created (hierarchical analysis,
 * dependency analysis, simulations results analysis).
 *
 */

#ifndef MAGNITUDE_H
#define MAGNITUDE_H

/// c++ std required libraries
#include <string>
#include <vector>

class Magnitude {
public:
	/// Default Constructor
	Magnitude( const std::string& name );
	/// Copy Constructor
	Magnitude(const Magnitude& orig);
	/// Copy Constructor (copy values selection)
	Magnitude(const Magnitude& orig, bool copyValues);
	virtual ~Magnitude();

	void set_name(std::string name);
	std::string get_name() const {return name;}
	std::string get_title_name() const;
	std::string get_file_name() const;
	bool get_found_in_results() const {return found_in_results;}
	std::string get_enclosed_name() const {return "\"" + name + "\"";}
	void set_found_in_results(bool found_in_results) { this->found_in_results = found_in_results; }
	std::vector<double>* get_values() { return &values; }

	// Analysis attributes
	bool get_analyzable() const {return analyzable;}
	void set_analyzable(bool analyzable) { this->analyzable = analyzable; }
	void set_analyze_error_in_time(bool analyze_error_in_time) {
		this->analyze_error_in_time = analyze_error_in_time; }
	bool get_analyze_error_in_time() const {return analyze_error_in_time;}
	void set_analyze_error_in_time_window(bool analyze_error_in_time_window) {
		this->analyze_error_in_time_window = analyze_error_in_time_window; }
	bool get_analyze_error_in_time_window() const {return analyze_error_in_time_window;}
	double get_analyzable_time_window_t0() const {return analyzable_time_window_t0;}
	void set_analyzable_time_window_t0(double analyzable_time_window_t0) { this->analyzable_time_window_t0 = analyzable_time_window_t0; }
	double get_analyzable_time_window_tf() const {return analyzable_time_window_tf;}
	void set_analyzable_time_window_tf(double analyzable_time_window_tf) { this->analyzable_time_window_tf = analyzable_time_window_tf; }

	/// Ones error detection
	double get_abs_error_margin_ones() const {return abs_error_margin_ones;}
	double get_error_time_span_ones() const {return error_time_span_ones;}
	double get_error_threshold_ones() const {return error_threshold_ones;}
	void set_abs_error_margin_ones(double abs_error_margin_ones) { this->abs_error_margin_ones = abs_error_margin_ones; }
	void set_error_time_span_ones(double error_time_span_ones) { this->error_time_span_ones = error_time_span_ones; }
	void set_error_threshold_ones(double error_threshold_ones) { this->error_threshold_ones = error_threshold_ones; }

	/// Zeros error detection
	double get_abs_error_margin_zeros() const {return abs_error_margin_zeros;}
	double get_error_time_span_zeros() const {return error_time_span_zeros;}
	double get_error_threshold_zeros() const {return error_threshold_zeros;}
	void set_abs_error_margin_zeros(double abs_error_margin_zeros) { this->abs_error_margin_zeros = abs_error_margin_zeros; }
	void set_error_time_span_zeros(double error_time_span_zeros) { this->error_time_span_zeros = error_time_span_zeros; }
	void set_error_threshold_zeros(double error_threshold_zeros) { this->error_threshold_zeros = error_threshold_zeros; }

	/// Default error detection
	double get_abs_error_margin_default() const {return abs_error_margin_default;}
	double get_error_time_span_default() const {return error_time_span_default;}
	void set_abs_error_margin_default(double abs_error_margin_default) { this->abs_error_margin_default = abs_error_margin_default; }
	void set_error_time_span_default(double error_time_span_default) { this->error_time_span_default = error_time_span_default; }

	/// do not take into account higher voltages than logic '1' or lower than logic '0'
	void set_ommit_upper_threshold( bool ommit_upper_threshold ){ this->ommit_upper_threshold = ommit_upper_threshold; }
	void set_ommit_lower_threshold( bool ommit_lower_threshold ){ this->ommit_lower_threshold = ommit_lower_threshold; }
	bool get_ommit_upper_threshold() const{ return ommit_upper_threshold; }
	bool get_ommit_lower_threshold() const{ return ommit_lower_threshold; }

	bool get_plottable() const{ return plottable; }
	void set_plottable( bool plottable ){this->plottable = plottable;}
	bool get_plottable_in_golden() const{ return plottable_in_golden; }
	void set_plottable_in_golden( bool plottable_in_golden ){this->plottable_in_golden = plottable_in_golden;}

	void AddValue( double value );
	double GetValue( int position );

private:
	std::string name;
	mutable std::string title_name;
	mutable std::string file_name;
	mutable bool valid_formatted_names{false};
	std::vector<double> values;
	bool found_in_results;
	/// plottable
	bool plottable;
	bool plottable_in_golden;
	/// Analysis attributes
	bool analyzable;
	bool analyze_error_in_time;
	/// time_window in which is analyzable
	bool analyze_error_in_time_window;
	double analyzable_time_window_t0;
	double analyzable_time_window_tf;
	/// ones
	double abs_error_margin_ones;
	double error_time_span_ones;
	double error_threshold_ones;
	/// zeros
	double abs_error_margin_zeros;
	double error_time_span_zeros;
	double error_threshold_zeros;
	/// default
	double abs_error_margin_default;
	double error_time_span_default;
	/// do not take into account higher voltages
	bool ommit_upper_threshold;
	/// do not take into account lower voltages
	bool ommit_lower_threshold;
};

#endif /* MAGNITUDE_H */
