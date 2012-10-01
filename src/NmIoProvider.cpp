/*
 * NmIoProvider.cpp
 *
 *  Created on: 10.09.2012
 *      Author: Admin
 */

#include <iostream>
#include <sstream>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/stdio.hpp>
#include "NmIoProvider.h"

NmIoProvider::NmIoProvider(const NmAnalyzerParams& argParams)
: params(argParams)
, nmInput(NULL)
{
}

NmIoProvider::~NmIoProvider()
{
	if(nmInput)
	{
		pclose(nmInput);
	}
}

std::ostream& NmIoProvider::getOutputStream()
{
	if(!outfile.get() && !params.outputFileName.empty())
	{
		outfile = std::auto_ptr<std::ofstream>(new std::ofstream(params.outputFileName.c_str()));
	}

	if(outfile.get())
	{
		return *outfile;
	}
	return std::cout;
}

std::istream& NmIoProvider::getInputStream()
{
	if(params.callNm && !nmStream.get())
	{
		std::ostringstream nmCommand;
		if(params.alternateNmExec.empty())
		{
			nmCommand << "nm";
		}
		else
		{
			nmCommand << params.alternateNmExec;
		}
		nmCommand << " -C -S --size-sort";
		for(std::vector<std::string>::const_iterator it = params.inputFileNames.begin();
			it != params.inputFileNames.end();
			++it)
		{
			nmCommand << " " << *it;
		}
		nmInput = popen(nmCommand.str().c_str(),"r");
		if(nmInput == NULL)
		{
			perror("popen");
			throw std::runtime_error("running nm failed");
		}
		nmStream = std::auto_ptr<io::file_descriptor_source>(new io::file_descriptor_source(nmInput->_file,io::never_close_handle));
	}

	if(!inputStream.get())
	{
		inputStream = std::auto_ptr<io::filtering_istream>(new io::filtering_istream());
		if(nmStream.get())
		{
			inputStream->push(*nmStream);
		}
		else
		{
			if(!params.inputFileNames.empty())
			{
				for(std::vector<std::string>::const_iterator it = params.inputFileNames.begin();
					it != params.inputFileNames.end();
					++it)
				{
					inputStream->push(io::file_source(*it));
				}
			}
			else
			{
				inputStream->push(io::file_descriptor_source(0,io::never_close_handle));
			}
		}
	}

	if(inputStream.get())
	{
		return *inputStream;
	}

	return std::cin;
}

