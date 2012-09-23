/*
 * NmIoProvider.h
 *
 *  Created on: 10.09.2012
 *      Author: Admin
 */

#ifndef NMIOPROVIDER_H_
#define NMIOPROVIDER_H_

#include <iostream>
#include <fstream>
#include <memory>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include <stdio.h>
#include "NmAnalyzerParams.h"

namespace io = boost::iostreams;

class NmIoProvider
{
public:
	NmIoProvider(const NmAnalyzerParams& params);
	virtual ~NmIoProvider();

	std::ostream& getOutputStream();
	std::istream& getInputStream();

private:
	const NmAnalyzerParams& params;
	std::auto_ptr<std::ofstream> outfile;
	std::auto_ptr<io::file_descriptor_source> nmStream;
	std::auto_ptr<io::filtering_istream> inputStream;
	FILE* nmInput;

};

#endif /* NMIOPROVIDER_H_ */
