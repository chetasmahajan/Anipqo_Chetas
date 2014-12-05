//============================================================================
// Name        : AnipqoDSL.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include "Nipqo.h"

#pragma GCC diagnostic ignored "-Wwrite-strings"

using namespace std;
int main(int argc, char *argv[]) {
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
//	string prefix("../inputs/");
	string prefix("./inputs/");
//	char buf[128];
//	int j = 0;
//	FILE *fp = fopen("./inputs/input.txt", "r");
//	assert(fp);
//	fscanf(fp, "%c", buf + j);
//	while(buf[j] != '\n')
//		fscanf(fp, "%c", buf + (++j));
//	buf[j] = '\0';
//	string par(buf);
	string par(argv[1]);

//	j = 0;
//	fscanf(fp, "%c", buf + j);
//	while(buf[j] != '\n')
//		fscanf(fp, "%c", buf + (++j));
//	buf[j] = '\0';
//	string selConst(buf);
////////	selConst = prefix + selConst + ".csv";
	string selConst(argv[2]);

//	j = 0;
//	fscanf(fp, "%c", buf + j);
//	while(buf[j] != '\n')
//		fscanf(fp, "%c", buf + (++j));
//	buf[j] = '\0';
//	string resolution(buf);
	string resolution(argv[3]);

//	j = 0;
//	fscanf(fp, "%c", buf + j);
//	while(buf[j] != '\n')
//		fscanf(fp, "%c", buf + (++j));
//	buf[j] = '\0';
//	string threshold(buf);
	string threshold(argv[4]);

//	j = 0;
//	fscanf(fp, "%c", buf + j);
//	while(buf[j] != '\n')
//		fscanf(fp, "%c", buf + (++j));
//	buf[j] = '\0';
//	string query(buf);
	string query(argv[5]);
	query = prefix + query + ".txt";

//	j = 0;
//	fscanf(fp, "%c", buf + j);
//	while(buf[j] != '\n')
//		fscanf(fp, "%c", buf + (++j));
//	buf[j] = '\0';
//	string mode(buf);
	string mode(argv[6]);

	int argc1 = 9;
	char *argv1[] = {"", "pqo", "singleDAG", "noCostLimit", "printTrace", "noVerify", (char*) par.c_str(), "0", (char*) selConst.c_str(), (char*) resolution.c_str(), (char*) threshold.c_str(), (char*) query.c_str(), (char*) mode.c_str()};
//	MEMORYSTATUSEX status;
//	status.dwLength = sizeof(status);
//	GlobalMemoryStatusEx(&status);
//	cout<<"RAM currently used in GBs = "<<(float) (status.ullTotalPhys - status.ullAvailPhys) / (1024 * 1024 * 1024)<<endl<<endl;
	Nipqo_t::PQO(argc1, argv1);
	return 0;
}
