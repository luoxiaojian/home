#ifndef _RUNTIME_H_
#define _RUNTIME_H_

#include "taskset.h"
#include "fraction.h"

#include <vector>
#include <set>
#include <algorithm>
#include <iostream>

#define MIN(a, b) ((a>b)?b:a)

using std::vector;
using std::cout;
using std::endl;
using std::set;

class taskstate {
public:
	explicit taskstate(int, int, int, int);

	int tid;
	fraction *al;
	fraction ret;
	fraction deadline;
};

class job {
public:
	explicit job(int, const fraction&, const fraction&);

	int tid;
	fraction dur;
	fraction start, end;
};

class allot {
public:
	explicit allot(int pi, const fraction& st, const fraction& en) : start(st), end(en), pid(pi) { assert(!(start>end)); }

	fraction start, end;
	int pid;
};

bool operator<(const allot& r1, const allot& r2);

class runtime {
public:
	explicit runtime(const taskset& );

	void updateUMat();
	void updateAL(int cur);
	void fakeAlloc(const fraction& cap);
	void alloc(int start, int end);
	void updateState(int cur, int next);

	void processAllotment();

	void run(FILE *flog);

	vector<taskstate> tslist;
	fraction **umat;
	vector<job> *allocation;

	set<allot> *allotment;

	fraction *ui;


	void outputAlloc() const;
	void outputAllot() const;

private:
	void initTSlist(const taskset&);
	void sortTSlist();

	void countPreemption();
	void countMigration();
	void countEvent();

	void flushLog(FILE *flog);

	fraction bdg(int tid, int pid, int tend);

	int ttnum, tnum, pnum, hp;
	int *execute, *period;

	int *preemption, *migration, *event;

	int rnum;
	int *b;
};

#endif
