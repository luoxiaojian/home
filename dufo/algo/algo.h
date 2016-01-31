#include "taskset.h"

#ifndef _ALGO_H_

class algo{
	public:
		explicit algo(taskset&);

		void schedule();

		int **alloc;

	private:
		void clearRowFlags();
		int searchEmpty(int tid, int rid);

		int *execute, *period;
		int ttnum, tnum, pnum, rnum, hp;
		int *rwidth, *b;

		int *remains;

		int *most_to_exe;
		int *least_to_exe;
		int *alloced_to_exe;
		int *laxity;
		int *jstart;

		int *rflag;
		int currentRowId;
};

#define _ALGO_H_
#endif
