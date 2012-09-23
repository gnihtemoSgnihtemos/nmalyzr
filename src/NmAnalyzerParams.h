/*
 * NmAnalyzerParams.h
 *
 *  Created on: 30.08.2012
 *      Author: Admin
 */

#ifndef NMANALYZERPARAMS_H_
#define NMANALYZERPARAMS_H_

#include <string>
#include <vector>
#include <set>

class NmAnalyzerParams
{
public:
	NmAnalyzerParams();
	NmAnalyzerParams(int argc, char** argv);
	NmAnalyzerParams(const NmAnalyzerParams& rhs)
	: inputFileNames(rhs.inputFileNames)
	, outputFileName(rhs.outputFileName)
	, callNm(rhs.callNm)
	, verbosityLevel(rhs.verbosityLevel)
	, filterExpr(rhs.filterExpr)
	, symbolTypesList(rhs.symbolTypesList)
	, showSizeInKB(rhs.showSizeInKB)
	, namespaceFilters(rhs.namespaceFilters)
	, classFilters(rhs.classFilters)
	, showNamespaceSummary(rhs.showNamespaceSummary)
	, showClassSummary(rhs.showClassSummary)
	{
	}

	virtual ~NmAnalyzerParams();

	NmAnalyzerParams& operator=(const NmAnalyzerParams& rhs)
	{
		inputFileNames = rhs.inputFileNames;
		outputFileName = rhs.outputFileName;
		callNm = rhs.callNm;
		verbosityLevel = rhs.verbosityLevel;
		filterExpr = rhs.filterExpr;
		symbolTypesList = rhs.symbolTypesList;
		showSizeInKB = rhs.showSizeInKB;
		namespaceFilters = rhs.namespaceFilters;
		classFilters = rhs.classFilters;
		showNamespaceSummary = rhs.showNamespaceSummary;
		showClassSummary = rhs.showClassSummary;
		return *this;
	};

	static const NmAnalyzerParams DefaultParams;

	std::vector<std::string> inputFileNames;
	std::string outputFileName;
	bool callNm;
	int verbosityLevel;
	std::string filterExpr;
	std::string symbolTypesList;
	bool showSizeInKB;
	std::set<std::string> namespaceFilters;
	std::set<std::string> classFilters;
	bool showNamespaceSummary;
	bool showClassSummary;

	static bool parseCmdLine(int argc, char** argv, NmAnalyzerParams& params);
private:
	NmAnalyzerParams
		( std::vector<std::string> argInputFileNames
		, std::string argOutputFileName
		, bool argCallNm
		, bool argVerbose
		, std::string argFilterExpr
		, std::string argSymbolTypesList
		, bool argShowSizeInKB
		, bool argShowNamespaceSummary
		, bool argShowClassSummary
		)
	: inputFileNames(argInputFileNames)
	, outputFileName(argOutputFileName)
	, callNm(argCallNm)
	, verbosityLevel(argVerbose ? 3 : 0)
	, filterExpr(argFilterExpr)
	, symbolTypesList(argSymbolTypesList)
	, showSizeInKB(argShowSizeInKB)
	, namespaceFilters()
	, classFilters()
	, showNamespaceSummary(argShowNamespaceSummary)
	, showClassSummary(argShowClassSummary)
	{
	}

};

#endif /* NMANALYZERPARAMS_H_ */
