/*
 * NmAnalyzerParams.cpp
 *
 *  Created on: 30.08.2012
 *      Author: Admin
 */

#include <sstream>
#include "NmAnalyzerParams.h"
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include "NmAnalyzer.h"

namespace po = boost::program_options;

const NmAnalyzerParams NmAnalyzerParams::DefaultParams =
		NmAnalyzerParams(std::vector<std::string>(),"",true,false,"","",false,false,false);

#define OptHelp "help"
#define OptOutfile "out-file"
#define OptTextInput "text-input"
#define OptVerbose "verbose"
#define OptFilter "filter"
#define OptSymbolTypes "symbol-types"
#define OptShowSizeInKB "kb"
#define OptInputFile "input-file"
#define OptNamespaceFilters "ns"
#define OptClassFilters "class"
#define OptShowNamespaceSummary "ns-summary"
#define OptShowClassSummary "class-summary"
#define OptShowInternalNs "show-internal-ns"
#define OptQuiet "quiet"
#define OptXmlOutputFilename "xml"
#define OptAlternateNmExec "alt-nm-path"

NmAnalyzerParams::NmAnalyzerParams()
{
	*this = DefaultParams;
}

NmAnalyzerParams::NmAnalyzerParams(int argc, char** argv)
{
	if(!parseCmdLine(argc,argv,*this))
	{
		*this = DefaultParams;
	}
}

NmAnalyzerParams::~NmAnalyzerParams()
{
}

void printHelp(const std::string& progName, const po::options_description& desc)
{
	std::cout << "Usage: " << progName << " [option(s)] [file(s)]" << std::endl;
	std::cout << "  Filters and analyzes nm output to view various summaries for groups of" << std::endl
			  << "  symbols." << std::endl;
    std::cout << desc << std::endl;
    std::cout << progName << " builds size summaries for demangled C++ nm symbol information." << std::endl;
    std::cout << "Additionally symbols from the input can be filtered by certain namespace or class" << std::endl;
    std::cout << "names." << std::endl;
    std::cout << "By default " << progName << " takes one or more object, library or executable input files, calls" << std::endl;
    std::cout << "'nm -C -S --size-sort' internally and filters and analyzes the output directly." << std::endl;
    std::cout << "If the input is provided through stdin, or the --" << OptTextInput << " option is specified" << std::endl;
    std::cout << "the input must conform the output produced by 'nm -C -S --size-sort'." << std::endl;
    std::cout << std::endl;
    std::cout << progName << " declares certain 'internal' namespace names to match special symbols from the" << std::endl;
    std::cout << "input:" << std::endl;
    std::cout << "  <linkmap>     matches symbols starting with '.'" << std::endl;
    std::cout << "  <sysinternal> matches symbols starting with '__'" << std::endl;
    std::cout << "  <system>      matches symbols starting with '_'" << std::endl;
    std::cout << "  <global>      matches any other symbols that have no namespace prefix." << std::endl;
    std::cout << std::endl;
}

bool NmAnalyzerParams::parseCmdLine(int argc, char** argv, NmAnalyzerParams& params)
{
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	    (OptHelp, "produce help message")
	    (OptOutfile ",o", po::value<std::string>(), "output-filename")
	    (OptTextInput ",T", "process text input produced by 'nm -C -S --size-sort ...'")
	    (OptVerbose ",V", po::value<int>()->implicit_value(3), "verbose output")
	    (OptFilter ",E", po::value<std::string>()->implicit_value(""), "the regex filter to use for the nm output before analyzing results")
	    (OptSymbolTypes ",S", po::value<std::string>(), "list of symbol types to analyze")
	    (OptShowSizeInKB, "shows the sizes in KB")
	    (OptNamespaceFilters ",n", po::value<std::string>(), "filters from namespace list")
	    (OptClassFilters ",c", po::value<std::string>(), "filters from class list")
	    (OptShowNamespaceSummary ",N", "shows summary by namespace")
	    (OptShowClassSummary ",C", "shows summary by class")
	    (OptShowInternalNs ",I", "shows all internal namespaces")
	    (OptQuiet ",Q", "suppresses output to stdout")
	    (OptXmlOutputFilename, po::value<std::string>(), "specifies an XML output file for the results")
	    (OptAlternateNmExec, po::value<std::string>(), "specifies an alternate nm executable to call")
	;
	po::options_description hidden("Hidden options");
	hidden.add_options()
		(OptInputFile, po::value< std::vector<std::string> >()->composing(), "input file")
	;
	po::options_description cmdline_opts("Command line options");
	cmdline_opts.add(desc).add(hidden);

	po::variables_map vm;
	po::positional_options_description p;
	p.add(OptInputFile, -1);

	std::string progName(argv[0]);
	if((progName.length() > 4) &&
	   (progName.substr(progName.length() - 5) == ".exe"))
	{
		progName = progName.substr(0,progName.length() - 4);
	}
	size_t pos = progName.find_last_of("\\/");
	if(pos != std::string::npos)
	{
		progName = progName.substr(pos + 1);
	}
	try
	{
		po::store(po::command_line_parser(argc, argv).
				  options(cmdline_opts).positional(p).run(), vm);
		po::notify(vm);
	}
	catch(const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		printHelp(progName,desc);
		return false;
	}

	if (vm.count(OptHelp))
	{
		printHelp(progName,desc);
	    return false;
	}

	if (vm.count(OptOutfile))
	{
	    params.outputFileName = vm[OptOutfile].as<std::string>();
	}
	if (vm.count(OptTextInput))
	{
	    params.callNm = false;
	}
	if (vm.count(OptVerbose))
	{
		params.verbosityLevel = vm[OptVerbose].as<int>();
	}
	if (vm.count(OptFilter))
	{
	    params.filterExpr = vm[OptFilter].as<std::string>();
	}
	if (vm.count(OptSymbolTypes))
	{
	    params.symbolTypesList = vm[OptSymbolTypes].as<std::string>();
	}
	if (vm.count(OptShowSizeInKB))
	{
	    params.showSizeInKB = true;
	}
	if (vm.count(OptInputFile))
	{
	    params.inputFileNames = vm[OptInputFile].as< std::vector<std::string> >();
	}
	if(params.inputFileNames.empty())
	{
		// Do text processing from stdin by default
		params.callNm = false;
	}
	if(vm.count(OptNamespaceFilters))
	{
		std::vector<std::string> namespaceFilters;
		boost::algorithm::split(namespaceFilters,vm[OptNamespaceFilters].as<std::string>(),boost::is_any_of(";"),boost::token_compress_on);
		for(std::vector<std::string>::iterator it = namespaceFilters.begin();
			it != namespaceFilters.end();
			++it)
		{
			if(params.namespaceFilters.find(*it) == params.namespaceFilters.end())
			{
				params.namespaceFilters.insert(*it);
			}
		}
		params.showNamespaceSummary = true;
	}
	if(vm.count(OptClassFilters))
	{
		std::vector<std::string> classFilters;
		boost::algorithm::split(classFilters,vm[OptClassFilters].as<std::string>(),boost::is_any_of(";"),boost::token_compress_on);
		for(std::vector<std::string>::iterator it = classFilters.begin();
			it != classFilters.end();
			++it)
		{
			if(params.classFilters.find(*it) == params.classFilters.end())
			{
				params.classFilters.insert(*it);
			}
		}
		params.showClassSummary = true;
	}
	if (vm.count(OptShowNamespaceSummary))
	{
	    params.showNamespaceSummary = true;
	}
	if (vm.count(OptShowClassSummary))
	{
	    params.showClassSummary = true;
	}
	if (vm.count(OptShowInternalNs))
	{
		const std::vector<std::string>& internal_namespaces = NmAnalyzer::getInternalNamespaces();
	    params.namespaceFilters.insert(internal_namespaces.begin(),internal_namespaces.end());
	}
	if (vm.count(OptQuiet))
	{
	    params.quiet = true;
	}
	if (vm.count(OptXmlOutputFilename))
	{
	    params.xmlOutputFilename = vm[OptXmlOutputFilename].as<std::string>();
	}
	if (vm.count(OptAlternateNmExec))
	{
	    params.alternateNmExec = vm[OptAlternateNmExec].as<std::string>();
	}

	return true;
}
