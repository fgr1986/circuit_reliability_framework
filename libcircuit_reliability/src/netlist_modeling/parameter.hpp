/*
 * transistor.h
 *
 *  Created on: Feb 20, 2013
 *      Author: fernando
 */

#ifndef PARAMETER_H
#define PARAMETER_H

/// c++ std required libraries
#include <string>

class Parameter {
public:
	Parameter();
	Parameter( const std::string& name, const std::string& value);
	Parameter( const std::string& name );
	Parameter(const Parameter& orig);
	virtual ~Parameter();

	void set_name(std::string name);
	std::string get_title_name() const;
	std::string get_file_name() const;
	std::string get_name() const {return name;}
	std::string get_value() const {return value;}
	void set_value(std::string value) { this->value = value; this->has_value=true; }
	bool get_has_value() const { return has_value; }
	void set_has_value( const bool has_value ){ this->has_value=has_value; }

	std::string ExportParameter();

	virtual std::string GetInfo();

protected:
	std::string value;
	std::string name;
	mutable std::string title_name;
	mutable std::string file_name;
	mutable bool valid_formatted_names{false};
	bool has_value;
};

#endif /* PARAMETER_H_ */
