#ifndef TEMPLATE_GLOBAL_FUNCTIONS_H
#define TEMPLATE_GLOBAL_FUNCTIONS_H

/// c++ std libraries includes
#include <algorithm>

#include <vector>
#include <set>
#include <string>
#include <sstream>
// std::cout
#include <iostream>


#define NON_UPDATE_INDEX 666666666


/// Get current date/time, myFormat is DD-MM-YYYY.HH:mm:ss ("%d-%m-%Y.%X")
template<class any_string_like_type>
std::string GetCurrentDateTime( const any_string_like_type& myFormat ) {
	time_t	 now = time(0);
	struct tm tstruct;
	char	 buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), myFormat, &tstruct);
	return buf;
}

/**
* Auxiliar Template,
* PlaneProfileIndexesStructure is a Vector of pointers to vectors of any_number
*/
template <class any_number>
using PlaneProfileIndexesStructure = std::vector<std::vector<any_number>*>;

template<class any_data_type>
bool vectorContains( const std::vector<any_data_type>& vector,
		const any_data_type& item ){
	return std::find(vector.begin(), vector.end(), item) != vector.end();
}

/**
* Template
*
* [References version]
* Delete the contents of the destination std::vector
* Performs a deep copy of the elements
* from the source to the destination
* @param source <std::vector<any_data_type*>> source vector
* @param destination <std::vector<any_data_type*>&> destination vector
*/
template<class any_data_type>
void deepCopyVectorOfPointers(const std::vector<any_data_type*> source,
		std::vector<any_data_type*>& destination) {
	if( destination.size()>0 ){
		deleteContentsOfVectorOfPointers( destination );
	}
	for ( auto& pElem : source ){
		destination.push_back( new any_data_type( *pElem ) );
	}
}

/**
* Template
* [References version]
* Delete the contents of the destination std::vector
* @param myVector <std::vector<any_data_type*>&> vector whose elements
* are going to be deleted
*/
template<class any_data_type>
void deleteContentsOfVectorOfPointers(std::vector<any_data_type*>& myVector) {
	if( myVector.size()>0 ){
		for ( auto& pElem : myVector ){
			delete pElem;
		}
		myVector.clear();
	}
}

/**
* Template
* [References version]
* Delete the contents of the destination std::vector
* @param myVector <std::vector<any_data_type*>&> vector whose elements
* are going to be deleted
*/
template<class any_number>
void deletePlaneProfileIndexesStructure(std::vector<std::vector<any_number>*>& myVector) {
	if( myVector.size()>0 ){
		for ( auto& auxVectorOfPointers : myVector ){
			delete auxVectorOfPointers;
		}
		myVector.clear();
	}
}

/**
* Template
* Converts any kind of number ussing stringstream
* @param number <any_number> number to be converted
* @return number.to_string
*/
template<class any_number>
std::string number2String( const any_number& number){
	// Boost
	// double value = kNotDefinedInt;
	// try {
	// 	value = boost::lexical_cast<double>(sNumber);
	// }
	// catch (boost::bad_lexical_cast const&) {
	// 	log_io->ReportError2AllLogs( "error parsing the number " + sNumber );
	// 	value = 0;
	// }
	// return value;
	std::stringstream ss;
	ss << number;
	return ss.str();
}

/**
* Template
* Debugs a vector of numbers
*/
template<class any_number>
void PrintIndexes( const std::vector<any_number>& indexes ){
	// fgarcia test
	for (auto& i : indexes ){
		std::cout << number2String(i) << ' ';
	}
	std::cout << "\n";
}

/**
* Template
* Updates a vector containing reference to a given count
* of each sweep for every parameter in vector 'params'
*
*/
template<class any_number, class any_parameter>
void UpdateParameterSweepIndexes(
		std::vector<any_number>& parameterSweepIndexes,
		const std::vector<any_parameter*>& params ){
	auto lastParam = parameterSweepIndexes.size()-1;
	// update
	parameterSweepIndexes.at(lastParam) = parameterSweepIndexes.at(lastParam)+1;
	// check overflow
	for( any_number pIndex=lastParam; pIndex>0; --pIndex ){
		any_number length = params.at(pIndex)->get_sweep_steps_number();
		if( parameterSweepIndexes.at(pIndex)==length ){
			parameterSweepIndexes.at(pIndex) = 0;
			parameterSweepIndexes.at(pIndex-1) = parameterSweepIndexes.at(pIndex-1)+1;
		}
	}
}

/**
* Template
* Updates a vector containing reference to a given count
* of each sweep for every parameter in vector 'params'
* ommiting masked elements with NON_UPDATE_INDEX
*
*/
template<class any_number, class any_parameter>
void UpdateNonSelectedParameterSweepIndexes(
		std::vector<any_number>& parameterSweepIndexes,
		const std::vector<any_parameter*>& params ){
	// fgarcia debug
	// PrintIndexes(parameterSweepIndexes);
	auto lastParam = parameterSweepIndexes.size()-1;
	// search lastParam that can be updated
	while( parameterSweepIndexes.at(lastParam)==NON_UPDATE_INDEX && lastParam>0 ){
		--lastParam;
	}
	// update last param
	if( parameterSweepIndexes.at(lastParam)!=NON_UPDATE_INDEX ){
		parameterSweepIndexes.at(lastParam) = parameterSweepIndexes.at(lastParam)+1;
	}else{
		// fgarcia debug there is no param that can be updated
		// std::cout << "there is no param that can be updated, lastParam: " << lastParam << "\n";
		return;
	}
	// check overflow
	for( any_number pIndex=lastParam; pIndex>0; --pIndex ){
		any_number length = params.at(pIndex)->get_sweep_steps_number();
		// update index
		if( parameterSweepIndexes.at(pIndex)!=NON_UPDATE_INDEX && pIndex>0
				&& parameterSweepIndexes.at(pIndex)==length ){
			parameterSweepIndexes.at(pIndex) = 0;
			while( pIndex>1 //pIndex-1>0
					&& parameterSweepIndexes.at(pIndex-1)==NON_UPDATE_INDEX){
				--pIndex;
			}
			// in case that the last param is not to be updated
			// we do not update
			if( parameterSweepIndexes.at(pIndex-1)!=NON_UPDATE_INDEX ){
				parameterSweepIndexes.at(pIndex-1) = parameterSweepIndexes.at(pIndex-1)+1;
			}
		}
	}
	// fgarcia debug
	// PrintIndexes(parameterSweepIndexes);
}

/**
* Template
* Updates a vector containing reference to a given count
* of each sweep for every parameter in vector 'params'
* ommiting masked elements with NON_UPDATE_INDEX
*
*/
template<class any_number, class any_parameter>
std::vector<any_number>* GetProfilesInPlane(
		const std::vector<any_number>& indexesPattern, const any_number& pAIndex,
		const any_number& pBIndex, const std::vector<any_parameter*>& params ){

	// return indexes
	std::vector<any_number>* indexesInPlane = new std::vector<any_number>();
	// aux structure
	std::vector<any_number> parameterCountIndexes(params.size(), 0);
	any_number totalProfiles = 1;
	for(auto const & p : params){
		totalProfiles*=p->get_sweep_steps_number();
	}
	// iterate
	auto lastParamIndex = indexesPattern.size();
	for( any_number i=0; i<totalProfiles; ++i ){
		bool inPlane = true;
		for( any_number pIndex=0; pIndex<lastParamIndex; ++pIndex ){
			if( pIndex!=pAIndex && pIndex!=pBIndex ){
				inPlane = inPlane && indexesPattern.at(pIndex)== parameterCountIndexes.at(pIndex);
				// if(inPlane){
				// 	std::cout << "pIndex " << number2String(pIndex) << " is " << number2String(indexesPattern.at(pIndex))  << "\n";
				// }
			}
		}
		if (inPlane){
			// std::cout << "adding indexes ->" ;
			// PrintIndexes( parameterCountIndexes );
			indexesInPlane->push_back(i);
		}
		UpdateParameterSweepIndexes(parameterCountIndexes, params);
	}
	return indexesInPlane;
}

/**
* Template
* Gets the different planes relative to p1-p2 parameters,
* including their related profile indexes.
*
*/
template<class any_number, class any_parameter>
PlaneProfileIndexesStructure<any_number>* GetPlanesForParams(
		const any_number& pAIndex, const any_number& pBIndex,
		const std::vector<any_parameter>& params){
	any_number pCount = 0;
	any_number totalPlanes = 1;
	any_number totalPointsInPlane = 1;
	any_number totalProfiles = 1;
	for(auto const &p : params) {
		if( pCount!= pAIndex && pCount!= pBIndex){
			totalPlanes *= p->get_sweep_steps_number();
		}else{
			totalPointsInPlane *= p->get_sweep_steps_number();
		}
		totalProfiles *= p->get_sweep_steps_number();
		++pCount;
	}
	// output
	PlaneProfileIndexesStructure<any_number>* planes = new PlaneProfileIndexesStructure<any_number>();
	// create patterns such as {NON_UPDATE_INDEX, 0, 0, NON_UPDATE_INDEX, 0};
	std::vector<any_number> planeIndexesPattern( params.size(), 0 );
	planeIndexesPattern[pAIndex] = NON_UPDATE_INDEX;
	planeIndexesPattern[pBIndex] = NON_UPDATE_INDEX;
	for( any_number i=0; i<totalPlanes; ++i){
		planes->push_back( GetProfilesInPlane( planeIndexesPattern, pAIndex, pBIndex, params) );
		UpdateNonSelectedParameterSweepIndexes( planeIndexesPattern, params );
	}
	return planes;
}


template<class any_parameter>
unsigned int CountInvolvedPlanes( const std::vector<any_parameter>& parameters2sweep ){
	std::set<std::pair<unsigned int,unsigned int>> exportedParamTuples;
	unsigned int planesCount = 0;
	for( unsigned int p1Index=0; p1Index<parameters2sweep.size(); ++p1Index ){
		for( unsigned int p2Index = 0; p2Index<parameters2sweep.size(); ++p2Index ){
			auto auxPair1 = std::make_pair( p1Index, p2Index);
			auto auxPair2 = std::make_pair( p2Index, p1Index);
			if( p1Index!=p2Index &&
				exportedParamTuples.end()==exportedParamTuples.find( auxPair1 ) &&
				exportedParamTuples.end()==exportedParamTuples.find( auxPair2 ) ){
				exportedParamTuples.insert(auxPair1);
				++planesCount;
			}
		}
	} // end of p1 vs p2 countt
	return planesCount;
}

#endif /* TEMPLATE_GLOBAL_FUNCTIONS_H */
