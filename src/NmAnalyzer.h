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

namespace Poco
{
namespace XML
{
	class Element;
	class Document;
}
}
class NmAnalyzer
{
public:
	NmAnalyzer(std::istream& input, const NmAnalyzerParams& params);
	virtual ~NmAnalyzer();

	bool analyzeInputData();
	std::ostream& printResults(std::ostream& os);
	std::ostream& writeXmlResults(std::ostream& os);
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
	Poco::XML::Document* createXmlSymbolTypeSummaries(Poco::XML::Document* doc, Poco::XML::Element* parentNode);
	void buildNamespaceSummaries();
	std::ostream& printNamespaceSummaries(std::ostream& os);
	Poco::XML::Document* createXmlNamespaceSummaries(Poco::XML::Document* doc, Poco::XML::Element* parentNode);
	void buildClassSummaries();
	std::ostream& printClassSummaries(std::ostream& os);
	Poco::XML::Document* createXmlClassSummaries(Poco::XML::Document* doc, Poco::XML::Element* parentNode);
	std::ostream& printAllSymbols(std::ostream& os);
	Poco::XML::Document* createXmlAllSymbols(Poco::XML::Document* doc, Poco::XML::Element* parentNode);
	std::string extractNamespace(const std::string& symbol);
	std::string extractClass(const std::string& symbol);
	std::ostream& printSymbolTypeMap(std::ostream& os, const std::map<std::string,SymbolTypeInfo>& symbolTypeMap, bool printSymbol = false, bool verbose = false);
	Poco::XML::Document* createXmlSymbolTypeMap(Poco::XML::Document* doc, Poco::XML::Element* parentNode, const std::map<std::string,SymbolTypeInfo>& symbolTypeMap, bool printSymbol = false, bool verbose = false);
	std::string stripBracketPair(char openingBracket,char closingBracket,const std::string& symbol, std::string& strippedPart);
	bool isClass(const std::string& classSymbol);
	std::string buildNamespaceFromSymbolPath(const std::vector<std::string>& symbolPathParts);
	std::ostream& printConsideredSymbols(std::ostream& os, char symbolType, const std::string& group, const SymbolTypeInfo& symbolTypeInfo);
	Poco::XML::Document* createXmlConsideredSymbols(Poco::XML::Document* doc, Poco::XML::Element* parentNode, char symbolType, const std::string& group, const SymbolTypeInfo& symbolTypeInfo);

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
