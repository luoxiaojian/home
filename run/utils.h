#ifndef _UTILS_H_
#define _UTILS_H_

#include <set>
#include "fraction.h"
#include <utility>
#include <assert.h>
#include <iostream>

using std::cout;
using std::endl;

using std::set;

#define PACKED 0
#define DUALED 1

class range {
	public:
		range();
		explicit range(const fraction&, const fraction&);
		explicit range(const range& r);

		range& operator=(const range& r);

		bool priorTo(const range&) const;
		fraction length() const;
		fraction start, end;
};

bool operator<(const range& r1, const range& r2);

class interval {
	public:
		interval();
		explicit interval(const range&);

		void append(const range&);
		interval complement(const range&);
		interval subtract(const fraction& len);

		void show();

		set<range> data;
		fraction cap;
};

class runtask {
	public:
		explicit runtask(int e, int p) : execute(e), period(p) { }	

		int execute;
		int period;

		interval alloc;
};

class runset {
	public:
		explicit runset(int st) : state(st), c(0, 1), toExecute(0), executed(0) { }

		void addSon(int i);
		void addPeriod(int p);
		void setC(const fraction&);

		set<int> sons;
		set<int> periods;
		int father;

		fraction c;
		int deadline;

		int state;

		fraction toExecute, executed;

		interval alloc;
};


#endif
