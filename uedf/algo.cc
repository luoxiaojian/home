#include <iostream>
#include <fstream>

#include "taskset.h"
#include "runtime.h"

using namespace std;


int main(int argc, char **argv)
{
	if(argc<3)
	{
		cout<<"./run <taskset_filename> <log_filename>"<<endl;
		return 0;
	}

	ifstream fin(argv[1]);
	taskset ts(fin);

	cout<<argv[1]<<endl;

	FILE *flog=fopen(argv[2], "w");

	runtime rt(ts);
	rt.run(flog);

	cout<<"Schedule success"<<endl;

//	rt.outputAlloc();
//	rt.outputAllot();

	fflush(flog);
	fclose(flog);

	return 0;

}
