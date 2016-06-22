/**
 * @file OceanEvalMetric.hpp
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

#ifndef OCEAN_EVAL_METRIC_H
#define OCEAN_EVAL_METRIC_H

#include "metric.hpp"

class OceanEvalMetric : public Metric {
public:
	/// Default Constructor
	OceanEvalMetric( const std::string& name );
	/// Copy Constructor
	OceanEvalMetric( const OceanEvalMetric& orig );

	OceanEvalMetric* GetCopy();
	virtual ~OceanEvalMetric();

	virtual double get_value_at( const int index ) override;

	double get_value() const{ return value; }
	double get_abs_error_margin() const{ return abs_error_margin; }
	std::string get_ocean_eval_expression() const{ return ocean_eval_expression; }

	void set_value( double value ){this->value = value;}
	void set_abs_error_margin( const double abs_error_margin ){this->abs_error_margin = abs_error_margin;}

	void set_ocean_eval_expression( const std::string& ocean_eval_expression ){this->ocean_eval_expression = ocean_eval_expression;}

private:
	double value;
	double abs_error_margin;
	std::string ocean_eval_expression;
};

#endif /* OCEAN_EVAL_METRIC_H */
