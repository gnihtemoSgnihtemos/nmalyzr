/*
 * NmAnalyzer.h
 *
 *  Created on: 11.09.2012
 *      Author: Admin
 */

#ifndef NMANALYZER_H_
#define NMANALYZER_H_

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <cctype>
#include "NmAnalyzerParams.h"

class NmAnalyzer
{
public:
	NmAnalyzer(std::istream& input, const NmAnalyzerParams& params);
	virtual ~NmAnalyzer();

	bool analyzeInputData();
	std::ostream& printResults(std::ostream& os);
	static const std::vector<std::string>& getInternalNamespaces();
private:
	struct NmRecord
	{
		size_t symbolSize;
		size_t symbolValue;
		char symbolType;
		std::string symbolName;
	};

	struct SymbolTypeSort
	{
		bool operator() (const char& lhs, const char& rhs) const
		{
			if(lhs == rhs)
			{
				return false;
			}
			if(lhs == std::tolower(rhs))
			{
				return true;
			}
			return std::tolower(lhs) < std::tolower(rhs);
		}
	};

	struct SymbolTypeInfo
	{
		char symbolType;
		std::string symbol;
		size_t totalSize;
		std::vector<std::string> allSymbolNames;
	};

	struct SymbolTypeInfoSortBySize
	{
		bool operator() (const SymbolTypeInfo& lhs, const SymbolTypeInfo& rhs) const
		{

			return lhs.totalSize < rhs.totalSize;
		}
	};

	bool readAllRecords();
	void buildSymbolTypeSummaries();
	std::ostream& printSymbolTypeSummaries(std::ostream& os);
	void buildNamespaceSummaries();
	std::ostream& printNamespaceSummaries(std::ostream& os);
	void buildClassSummaries();
	std::ostream& printClassSummaries(std::ostream& os);
	std::ostream& printAllSymbols(std::ostream& os);
	std::string extractNamespace(const std::string& symbol);
	std::string extractClass(const std::string& symbol);
	std::ostream& printSymbolTypeMap(std::ostream& os, const std::map<std::string,SymbolTypeInfo>& symbolTypeMap, bool printSymbol = false, bool verbose = false);
	std::string stripBracketPair(char openingBracket,char closingBracket,const std::string& symbol, std::string& strippedPart);
	bool isClass(const std::string& classSymbol);
	std::string buildNamespaceFromSymbolPath(const std::vector<std::string>& symbolPathParts);
	std::ostream& printConsideredSymbols(std::ostream& os, char symbolType, const std::string& group, const SymbolTypeInfo& symbolTypeInfo);

	std::istream& input;
	const NmAnalyzerParams& params;
	std::vector<NmRecord> allRecords;
	std::set<char,SymbolTypeSort> allSymbolTypes;
	std::map<std::string,SymbolTypeInfo> symbolTypeSummaries;
	std::map<char,std::map<std::string,SymbolTypeInfo> > namespaceSummaries;
	std::map<char,std::map<std::string,SymbolTypeInfo> > classSummaries;
	std::set<std::string> allClasses;
	std::map<std::pair<char,std::string>,NmRecord> allSymbolInfos;
};

#endif /* NMANALYZER_H_ */
