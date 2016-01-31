#include <iostream>
#include <fstream>

#include "taskset.h"
#include "algo.h"

using namespace std;

int main(int argc, char **argv) {
	ifstream fin(argv[1]);
	taskset ts(fin);
	algo al(ts);
	al.schedule();
}
