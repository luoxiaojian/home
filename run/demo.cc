#include "run.h"
#include <fstream>
#include <iostream>
#include "fraction.h"
#include <stdio.h>
#include "utils.h"
#include <string.h>

using namespace std;


int main(int argc, char **argv) {
	if(argc<3) {
		cout<<"usage: ./run <taskset_filename> <log_filename>"<<endl;
		return 0;
	}
	FILE *fin, *flog;

	fin=fopen(argv[1], "r");
	flog=fopen(argv[2], "w");

	run r(fin);

	r.schedule();
	
	if(r.checkSchedule()) 
		cout<<argv[1]<<" schedule success"<<endl;
	else
		cout<<argv[1]<<" schedule error"<<endl;

	r.countPreemption();

	r.dispatchToCore();

	r.countMigration();
	r.countEvent();

	r.flushLog(flog);
	return 0;
}
