/**
 * @file Metric.hpp
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

#ifndef METRIC_H
#define METRIC_H

/// c++ std required libraries
#include <string>

class Metric {
public:

	virtual ~Metric();
	// Get a deep copy of the statement
	virtual Metric* GetCopy() = 0;

	bool is_transient_magnitude() const{ return transient_magnitude; }

	void set_name(std::string name);
	std::string get_name() const {return name;}
	std::string get_title_name() const;
	std::string get_file_name() const;
	std::string get_enclosed_name() const;

	bool get_analyzable() const {return analyzable;}
	void set_analyzable(bool analyzable) { this->analyzable = analyzable; }

	bool get_found_in_results() const {return found_in_results;}
	void set_found_in_results(bool found_in_results) { this->found_in_results = found_in_results; }

	virtual double get_value_at( const int index )= 0;

protected:
	/// transient magnitude or oceanEval like metric
	bool transient_magnitude;
	std::string name;
	mutable std::string title_name;
	mutable std::string file_name;
	mutable std::string enclosed_name;
	mutable bool valid_formatted_names{false};
	bool analyzable;
	bool found_in_results;

};

#endif /* METRIC_H */
