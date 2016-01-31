#ifndef _RUN_H_
#define _RUN_H_

#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include <assert.h>
#include <vector>

#include "fraction.h"

using std::ifstream;
using std::vector;

#define MAX_DEEP 100

class unit {
	public:
		explicit unit(int t, const range& r): tid(t), time(r) { }

		int tid;
		range time;
};

bool operator<(const unit& r1, const unit& r2);

class run {
	public:
		explicit run(FILE *);
		explicit run(int tt, int t, int p, int h, int *pe, int *e);

		int ttnum;

		int tnum, pnum, hp;
		int *period, *execute;

		int *preemption, *migration, *event;

		vector<runset> sets;
		vector<runtask> tasks;

		vector<unit> *dispatch;

		set<unit> *locate;

		int numOfLevel[MAX_DEEP];
		int indOfLevel[MAX_DEEP];
		int levelNum;

		int root;
		int *rind;
		int rnum;

		void schedule();
		void dispatchToCore();

		bool checkSchedule();

		void countPreemption();
		void countMigration();
		void countEvent();

		void flushLog(FILE *fout);

		~run();

	private:
		int sumOfLevel(int i);
		void updateDeadline(int cur);
		void generateRow();
		void allocARow(int rid);
		void allocAServer(int , int, int);
};

#endif
