/*
 * NmAnalyzerMain.h
 *
 *  Created on: 30.08.2012
 *      Author: Admin
 */

#ifndef NMANALYZERMAIN_H_
#define NMANALYZERMAIN_H_

class NmAnalyzerMain
{
public:
	NmAnalyzerMain(int argc, char** argv);
	virtual ~NmAnalyzerMain();

	int run();

private:
	int argc;
	char** argv;
};

#endif /* NMANALYZERMAIN_H_ */
