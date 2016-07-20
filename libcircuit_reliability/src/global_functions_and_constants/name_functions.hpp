#ifndef NAME_GLOBAL_FUNCTIONS_H
#define NAME_GLOBAL_FUNCTIONS_H

#include <string>
// for boost:: replace_all
#include <boost/algorithm/string.hpp>

static const std::string kFileNameIllegalChars = "\\/:?\"<>|";

static std::string FilenameWithoutIllegalChars( const std::string& fileName ) {
	std::string resultName = fileName;
	for ( auto & c : resultName ){
		if( kFileNameIllegalChars.find(c) != std::string::npos ){
			c = '.';
		}
	}
	// return fileName;
	return resultName;
}

static std::string TitleWithoutIllegalChars( const std::string& fileName ) {
	// std::string resultName = fileName;
	// boost::replace_all( resultName, "_", "\\_");
	// we now use
	return fileName;
}

#endif /* NAME_GLOBAL_FUNCTIONS_H */
