/*
 * NmAnalyzerMain.cpp
 *
 *  Created on: 30.08.2012
 *      Author: Admin
 */

#include "NmAnalyzerMain.h"
#include "NmAnalyzerParams.h"
#include "NmIoProvider.h"
#include "NmAnalyzer.h"

NmAnalyzerMain::NmAnalyzerMain(int ac, char** av)
: argc(ac)
, argv(av)
{
}

NmAnalyzerMain::~NmAnalyzerMain()
{
}

int NmAnalyzerMain::run()
{
	NmAnalyzerParams params;
	if(NmAnalyzerParams::parseCmdLine(argc,argv,params))
	{
		NmIoProvider ioProvider(params);
		NmAnalyzer analyzer(ioProvider.getInputStream(),params);
		if(analyzer.analyzeInputData())
		{
			if(!params.quiet)
			{
				analyzer.printResults(ioProvider.getOutputStream());
			}
			if(!params.xmlOutputFilename.empty())
			{
				std::fstream xmlOutStream;
				xmlOutStream.open(params.xmlOutputFilename.c_str(),std::ios_base::out);
				analyzer.writeXmlResults(xmlOutStream);
			}
			return 0;
		}
	}
	return 1;
}
