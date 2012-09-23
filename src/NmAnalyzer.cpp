/*
 * NmAnalyzer.cpp
 *
 *  Created on: 11.09.2012
 *      Author: Admin
 */

#include "NmAnalyzer.h"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/find.hpp>

#define InternalNsLinkmap "<linkmap>"
#define InternalNsSysinternal "<sysinternal>"
#define InternalNsSystem "<system>"
#define InternalNsGlobal "<global>"

std::vector<std::string> init_internal_namespaces();
std::map<std::string,std::string> init_internal_namespace_keys();

static const std::vector<std::string> internal_namespaces = init_internal_namespaces();
static const std::map<std::string,std::string> internal_namespace_keys = init_internal_namespace_keys();

bool containsInternalNamespace(const std::set<std::string>& filter)
{
	for(std::vector<std::string>::const_iterator it = internal_namespaces.begin();
		it != internal_namespaces.end();
		++it)
	{
		if(filter.find(*it) != filter.end())
		{
			return true;
		}
	}

	return false;
}

bool isInternalNamespace(const std::string& ns)
{
	for(std::vector<std::string>::const_iterator it = internal_namespaces.begin();
		it != internal_namespaces.end();
		++it)
	{
		if(*it == ns)
		{
			return true;
		}
	}

	return false;
}

std::vector<std::string> init_internal_namespaces()
{
	std::vector<std::string> result;
	result.push_back(InternalNsLinkmap);
	result.push_back(InternalNsSysinternal);
	result.push_back(InternalNsSystem);
	result.push_back(InternalNsGlobal);
	return result;
}

std::map<std::string,std::string> init_internal_namespace_keys()
{
	std::map<std::string,std::string> result;
	result[InternalNsLinkmap] = ".";
	result[InternalNsSysinternal] = "__";
	result[InternalNsSystem] = "_";
	return result;
}

bool matchesInternalNamespace(const std::string& internalNs, const std::string& symbol)
{
	std::map<std::string,std::string>::const_iterator foundKey = internal_namespace_keys.find(internalNs);
	if((foundKey != internal_namespace_keys.end()) && !symbol.empty())
	{
		return boost::find_first(symbol,foundKey->second).begin() == symbol.begin();
	}
	return false;
}

NmAnalyzer::NmAnalyzer(std::istream& argInput, const NmAnalyzerParams& argParams)
: input(argInput)
, params(argParams)
{
}

NmAnalyzer::~NmAnalyzer()
{
}

bool NmAnalyzer::analyzeInputData()
{
	if(readAllRecords())
	{
		buildSymbolTypeSummaries();
		if(params.showNamespaceSummary)
		{
			buildNamespaceSummaries();
		}
		if(params.showClassSummary)
		{
			buildClassSummaries();
		}
		return true;
	}
	return false;
}

std::ostream& NmAnalyzer::printResults(std::ostream& os)
{
	printSymbolTypeSummaries(os);
	if(params.showNamespaceSummary)
	{
		printNamespaceSummaries(os);
	}
	if(params.showClassSummary)
	{
		printClassSummaries(os);
	}
	if(params.verbosityLevel >= 1)
	{
		printAllSymbols(os);
	}
	return os;
}

const std::vector<std::string>& NmAnalyzer::getInternalNamespaces()
{
	return internal_namespaces;
}


bool NmAnalyzer::readAllRecords()
{
	boost::regex regexFilter;
	bool expressionFilter = false;
	boost::regex regexNamespace;
	bool namespaceFilter = false;
	boost::regex regexClass;
	bool classFilter = false;

	allClasses.clear();

	if(!params.filterExpr.empty())
	{
		regexFilter = boost::regex(params.filterExpr);
		expressionFilter = true;
	}
	if(!params.namespaceFilters.empty())
	{
		std::ostringstream filterExpr;
		filterExpr << "^";
		bool foundNonInternalFilter = false;
		for(std::set<std::string>::const_iterator it = params.namespaceFilters.begin();
			it != params.namespaceFilters.end();
			++it)
		{
			if(!isInternalNamespace(*it))
			{
				if(it != params.namespaceFilters.begin())
				{
					filterExpr << "|";
				}
				filterExpr << "(" << *it << ")";
				foundNonInternalFilter = true;
			}
		}
		if(foundNonInternalFilter)
		{
			filterExpr << "::.+";
			//std::cout << "*** Namespace filter expression: '" << filterExpr.str() << "'" << std::endl;
			regexNamespace = boost::regex(filterExpr.str());
			namespaceFilter = true;
		}
	}
	if(!params.classFilters.empty())
	{
		std::ostringstream filterExpr;
		filterExpr << "([^:]+::)*(";
		bool first = true;
		for(std::set<std::string>::const_iterator it = params.classFilters.begin();
			it != params.classFilters.end();
			++it)
		{
			if(!first)
			{
				filterExpr << "|";
			}
			else
			{
				first = false;
			}
			filterExpr << "(" << *it << ")";
		}
		filterExpr << ")::[^:]+";
		//std::cout << "*** Class filter expression: '" << filterExpr.str() << "'" << std::endl;
		regexClass = boost::regex(filterExpr.str());
		classFilter = true;
	}

	while(!input.eof())
	{
		std::string line;
		std::getline(input,line);
		if(!line.empty())
		{
			if(expressionFilter && !boost::regex_search(line,regexFilter))
			{
				continue;
			}

			std::istringstream iss(line);
			NmRecord newRecord;
			if(!(iss >> std::hex >> newRecord.symbolSize))
			{
				continue;
			}
			if(!(iss >> std::hex >> newRecord.symbolValue))
			{
				continue;
			}
			if(!(iss >> newRecord.symbolType))
			{
				continue;
			}
			std::string symbolPart;
			bool firstPart = true;
			while(iss >> symbolPart)
			{
				if(firstPart)
				{
					firstPart = false;
				}
				else
				{
				    newRecord.symbolName += " ";
				}
			    newRecord.symbolName += symbolPart;
			}

			if(!params.symbolTypesList.empty() &&
			   (params.symbolTypesList.find(newRecord.symbolType) == std::string::npos))
			{
				continue;
			}

			if(namespaceFilter && !regex_match(newRecord.symbolName,regexNamespace))
			{
				continue;
			}

			if(classFilter && !regex_match(newRecord.symbolName,regexClass))
			{
				continue;
			}

			allRecords.push_back(newRecord);
			if(allSymbolTypes.find(newRecord.symbolType) == allSymbolTypes.end())
			{
				allSymbolTypes.insert(newRecord.symbolType);
			}
		}
	}

	for(std::vector<NmRecord>::iterator it = allRecords.begin();
		it != allRecords.end();
		++it)
	{
		std::pair<char,std::string> key = std::make_pair(it->symbolType,it->symbolName);
		std::map<std::pair<char,std::string>,NmRecord>::iterator foundEntry = allSymbolInfos.find(key);
		if(foundEntry == allSymbolInfos.end())
		{
			allSymbolInfos[key] = *it;
		}
		else
		{
			allSymbolInfos[key].symbolSize += it->symbolSize;
		}
	}
	return true;
}

void NmAnalyzer::buildSymbolTypeSummaries()
{
	for(std::vector<NmRecord>::iterator it = allRecords.begin();
		it != allRecords.end();
		++it)
	{
		std::string key;
		key.assign(&it->symbolType,1);
		symbolTypeSummaries[key].symbolType = it->symbolType;
		symbolTypeSummaries[key].totalSize += it->symbolSize;
		symbolTypeSummaries[key].allSymbolNames.push_back(it->symbolName);
	}
}

std::ostream& NmAnalyzer::printSymbolTypeSummaries(std::ostream& os)
{
	os << "Summary by symbol type:" << std::endl
	   << "==========================" << std::endl;
	if(params.showSizeInKB)
	{
		os << "Type    Size (KB) #Symbols " << std::endl;
	}
	else
	{
		os << "Type         Size #Symbols " << std::endl;
	}
	return printSymbolTypeMap(os,symbolTypeSummaries);
}

void NmAnalyzer::buildNamespaceSummaries()
{
	for(std::vector<NmRecord>::iterator it = allRecords.begin();
		it != allRecords.end();
		++it)
	{
		std::string ns = extractNamespace(it->symbolName);
		if(ns.empty() && containsInternalNamespace(params.namespaceFilters))
		{
			if((params.namespaceFilters.find(InternalNsLinkmap) != params.namespaceFilters.end()) &&
			   matchesInternalNamespace(InternalNsLinkmap,it->symbolName))
			{
				ns = InternalNsLinkmap;
			}
			else if((params.namespaceFilters.find(InternalNsSysinternal) != params.namespaceFilters.end()) &&
			        matchesInternalNamespace(InternalNsSysinternal,it->symbolName))
			{
				ns = InternalNsSysinternal;
			}
			else if((params.namespaceFilters.find(InternalNsSystem) != params.namespaceFilters.end()) &&
			        matchesInternalNamespace(InternalNsSystem,it->symbolName))
			{
				ns = InternalNsSystem;
			}
			else if(params.namespaceFilters.find(InternalNsGlobal) != params.namespaceFilters.end())
			{
				ns = InternalNsGlobal;
			}
		}

		if(!ns.empty())
		{
			namespaceSummaries[it->symbolType][ns].symbol = ns;
			namespaceSummaries[it->symbolType][ns].allSymbolNames.push_back(it->symbolName);
			namespaceSummaries[it->symbolType][ns].symbolType = it->symbolType;
			namespaceSummaries[it->symbolType][ns].totalSize += it->symbolSize;
		}
	}
}

std::ostream& NmAnalyzer::printNamespaceSummaries(std::ostream& os)
{
	os << "Namespace summaries:" << std::endl
	   << "==============================================================================" << std::endl;
	if(params.showSizeInKB)
	{
		os << "Type    Size (KB) Namespace " << std::endl;
	}
	else
	{
		os << "Type         Size Namespace " << std::endl;
	}
	for(std::set<char>::iterator it = allSymbolTypes.begin();
		it != allSymbolTypes.end();
		++it)
	{
		std::map<char,std::map<std::string,SymbolTypeInfo> >::iterator foundEntry = namespaceSummaries.find(*it);
		if(foundEntry != namespaceSummaries.end())
		{
			printSymbolTypeMap(os,foundEntry->second,true);
			if(params.verbosityLevel >= 2)
			{
	//			printConsideredSymbols(os,it->first,it->second);
			}
		}
	}
	return os;
}

void NmAnalyzer::buildClassSummaries()
{
	for(std::vector<NmRecord>::iterator it = allRecords.begin();
		it != allRecords.end();
		++it)
	{
		std::string cls = extractClass(it->symbolName);
		if(!cls.empty())
		{
			classSummaries[it->symbolType][cls].symbol = cls;
			classSummaries[it->symbolType][cls].allSymbolNames.push_back(it->symbolName);
			classSummaries[it->symbolType][cls].symbolType = it->symbolType;
			classSummaries[it->symbolType][cls].totalSize += it->symbolSize;
		}
	}
}

std::ostream& NmAnalyzer::printClassSummaries(std::ostream& os)
{
	os << "Class summaries:" << std::endl
	   << "==============================================================================" << std::endl;
	if(params.showSizeInKB)
	{
		os << "Type    Size (KB) Class " << std::endl;
	}
	else
	{
		os << "Type         Size Class " << std::endl;
	}
	for(std::set<char>::iterator it = allSymbolTypes.begin();
		it != allSymbolTypes.end();
		++it)
	{
		std::map<char,std::map<std::string,SymbolTypeInfo> >::iterator foundEntry = classSummaries.find(*it);
		if(foundEntry != classSummaries.end())
		{
			printSymbolTypeMap(os,foundEntry->second,true);
			if(params.verbosityLevel >= 2)
			{
	//			printConsideredSymbols(os,it->first,it->second);
			}
		}
	}
	return os;
}

std::ostream& NmAnalyzer::printAllSymbols(std::ostream& os)
{
	os << "All symbols considered:" << std::endl
	   << "==============================================================================" << std::endl;
	if(params.showSizeInKB)
	{
		os << "Type    Size (KB) Category Symbol " << std::endl;
	}
	else
	{
		os << "Type         Size Category Symbol " << std::endl;
	}
	for(std::map<std::pair<char,std::string>,NmRecord>::iterator it = allSymbolInfos.begin();
		it != allSymbolInfos.end();
		++it)
	{
		os << it->second.symbolType << "     "
		   << std::setw(11) << std::right;
		if(params.showSizeInKB)
		{
			os << std::fixed << std::setprecision(1) << (double)it->second.symbolSize / 1024.0;
		}
		else
		{
			os << it->second.symbolSize;
		}
		std::string cls = extractClass(it->first.second);
		if(!cls.empty())
		{
			os << " class";
		}
		else
		{
			os << " global";
		}
		os << " '"
		   << it->first.second
		   << "'" << std::endl;
	}
	return os;
}


std::string NmAnalyzer::extractNamespace(const std::string& symbol)
{
	std::string ns;
	std::string strippedPart;
	std::string cls = extractClass(symbol);
	if(!cls.empty())
	{
		cls = stripBracketPair('<','>',cls,strippedPart);
		std::vector<std::string> classPathParts;
		boost::split(classPathParts,cls,boost::is_any_of("::"),boost::token_compress_on);
		ns = buildNamespaceFromSymbolPath(classPathParts);
	}
	else
	{
		// Assume this symbol is a namespace global function/variable
		std::string globalSymbolName = stripBracketPair('(',')',symbol,strippedPart);
		globalSymbolName = stripBracketPair('<','>',globalSymbolName,strippedPart);
		std::vector<std::string> symbolPathParts;
		boost::split(symbolPathParts,globalSymbolName,boost::is_any_of("::"),boost::token_compress_on);
		ns = buildNamespaceFromSymbolPath(symbolPathParts);
		std::vector<std::string> wsSplitted;
		boost::split(wsSplitted,ns,boost::is_any_of(" \t"),boost::token_compress_on);
		if(wsSplitted.size() > 1)
		{
			ns = wsSplitted[wsSplitted.size() - 1];
		}
	}

	if(isClass(ns))
	{
		ns = "";
	}
	return ns;
}

std::string NmAnalyzer::extractClass(const std::string& symbol)
{
	std::string cls;
	std::string strippedPart;
	std::string fullSymbol = symbol;
	boost::trim(fullSymbol);
	fullSymbol = stripBracketPair('(',')',symbol,strippedPart);
	fullSymbol = stripBracketPair('<','>',fullSymbol,strippedPart);

	size_t pos = fullSymbol.find_last_of(':');
	if(pos != std::string::npos)
	{
		--pos;
		cls = fullSymbol.substr(0,pos);
        std::string untemplatedClassName = stripBracketPair('<','>',cls,strippedPart);
        if(untemplatedClassName.find('<') == std::string::npos &&
           untemplatedClassName.find(' ') != std::string::npos)
        {
        	cls = "";
        }
	}

	if(!cls.empty() && !isClass(cls))
	{
		cls = "";
	}
	return cls;
}

std::ostream& NmAnalyzer::printSymbolTypeMap(std::ostream& os, const std::map<std::string,SymbolTypeInfo>& symbolTypeMap, bool printSymbol)
{
	std::vector<SymbolTypeInfo> symbolTypeInfos;
//	for(std::set<char>::iterator it = allSymbolTypes.begin();
//		it != allSymbolTypes.end();
//		++it)
//	{
//		std::map<char,SymbolTypeInfo>::const_iterator foundSummary = symbolTypeMap.find(*it);
//		if(foundSummary != symbolTypeMap.end())
//		{
//			symbolTypeInfos.push_back(foundSummary->second);
//		}
//	}
	for(std::map<std::string,SymbolTypeInfo>::const_iterator it = symbolTypeMap.begin();
		it != symbolTypeMap.end();
		++it)
	{
		symbolTypeInfos.push_back(it->second);
	}

	std::sort(symbolTypeInfos.begin(),symbolTypeInfos.end(),SymbolTypeInfoSortBySize());
	for(std::vector<SymbolTypeInfo>::iterator it = symbolTypeInfos.begin();
		it != symbolTypeInfos.end();
		++it)
	{
		os << it->symbolType << "     "
		   << std::setw(11) << std::right;
		if(params.showSizeInKB)
		{
			os << std::fixed << std::setprecision(1) << (double)it->totalSize / 1024.0;
		}
		else
		{
			os << it->totalSize;
		}
		if(printSymbol)
		{
			os << " '"
			   << it->symbol
			   << "'" << std::endl;
		}
		else
		{
			os << " "
			   << std::setw(8) << std::right << it->allSymbolNames.size()
			   << std::endl;
		}
	}
	return os;
}

std::string NmAnalyzer::stripBracketPair(char openingBracket,char closingBracket,const std::string& symbol, std::string& strippedPart)
{
	std::string result = symbol;

	//std::cout << "*** Symbol ends with '" << result[result.length()] << "'" << std::endl;
	if(!result.empty() &&
	   result[result.length() -1] == closingBracket)
	{
		size_t openPos = result.find_first_of(openingBracket);
		if(openPos != std::string::npos)
		{
			//std::cout << "*** Removing brackets" << std::endl;
			strippedPart = result.substr(openPos);
			result = result.substr(0,openPos);
		}
	}
	return result;
}

bool NmAnalyzer::isClass(const std::string& classSymbol)
{
	std::set<std::string>::iterator foundClass = allClasses.find(classSymbol);
	if(foundClass != allClasses.end())
	{
		return true;
	}

	std::string strippedPart;
	std::string destructorName = stripBracketPair('<','>',classSymbol,strippedPart);
	std::vector<std::string> destructorPathParts;
	boost::split(destructorPathParts,destructorName,boost::is_any_of("::"),boost::token_compress_on);
	if(!destructorPathParts.empty())
	{
	    destructorName = classSymbol + std::string("::~") + destructorPathParts.back() + std::string("()");
		for(std::vector<NmRecord>::iterator it = allRecords.begin();
			it != allRecords.end();
			++it)
		{
			if(boost::find_last(it->symbolName,destructorName) == it->symbolName)
			{
				allClasses.insert(classSymbol);
				return true;
			}
		}
	}
	return false;
}

std::string NmAnalyzer::buildNamespaceFromSymbolPath(const std::vector<std::string>& symbolPathParts)
{
	if(symbolPathParts.size() >= 2)
	{
		std::ostringstream oss;
		bool firstItem = true;
		for(unsigned int i = 0;i < symbolPathParts.size() - 1;++i)
		{
			if((symbolPathParts[i].find('<') != std::string::npos) ||
			   (symbolPathParts[i].find('(') != std::string::npos))
			{
				break;
			}
			if(!firstItem)
			{
				oss << "::";
			}
			else
			{
				firstItem = false;
			}
			oss << symbolPathParts[i];
		}
		return oss.str();
	}
	return "";
}

std::ostream& NmAnalyzer::printConsideredSymbols(std::ostream& os, const std::string& group, const std::map<char,SymbolTypeInfo>& symbolTypeMap)
{
	os << "Considered symbols for '" << group << "':" << std::endl
	   << "------------------------------------------------------------------------------" << std::endl;
	for(std::map<char,SymbolTypeInfo>::const_iterator it = symbolTypeMap.begin();
		it != symbolTypeMap.end();
		++it)
	{
		for(std::vector<std::string>::const_iterator symbolIt = it->second.allSymbolNames.begin();
			symbolIt != it->second.allSymbolNames.end();
			++symbolIt)
		{
			os << "       "
			   << std::setw(11) << std::right;
			std::map<std::pair<char,std::string>,NmRecord>::iterator foundSymbol = allSymbolInfos.find(std::make_pair(it->first,*symbolIt));
			if(foundSymbol != allSymbolInfos.end())
			{
				if(params.showSizeInKB)
				{
					os << std::fixed << std::setprecision(1) << (double)foundSymbol->second.symbolSize / 1024.0;
				}
				else
				{
					os << foundSymbol->second.symbolSize;
				}
				os << " " << *symbolIt << std::endl;
			}
		}
	}
	return os;
}
