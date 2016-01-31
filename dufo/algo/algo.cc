#include "algo.h"
#include <iostream>
#include <set>

#define MIN(a, b) ((a)>(b)?(b):(a))
#define MAX(a, b) ((a)<(b)?(b):(a))

using std::cout;
using std::endl;
using std::set;

algo::algo(taskset& ts) {
	ttnum=ts.ttnum;
	tnum=ts.tnum;
	rnum=ts.rnum;
	pnum=ts.pnum;
	execute=ts.execute;
	period=ts.period;
	rwidth=ts.rwidth;
	b=ts.b;

	alloc=(int**)malloc(sizeof(int*)*tnum);
	for(int i=0; i<tnum; i++)
		alloc[i]=(int*)malloc(sizeof(int)*rnum);
	for(int i=0; i<tnum; i++)
		for(int j=0; j<rnum; j++)
			alloc[i][j]=0;

	remains=(int*)malloc(sizeof(int)*rnum);
	for(int i=0; i<rnum; i++)
		remains[i]=pnum*rwidth[i];

	most_to_exe=(int*)malloc(sizeof(int)*tnum);
	least_to_exe=(int*)malloc(sizeof(int)*tnum);
	laxity=(int*)malloc(sizeof(int)*tnum);
	jstart=(int*)malloc(sizeof(int)*tnum);

	for(int i=0; i<tnum; i++) {
		most_to_exe[i]=0;
		least_to_exe[i]=0;
		alloced_to_exe[i]=0;
		laxity[i]=period[i]-execute[i];
		cout<<"laxity["<<i<<"]="<<laxity[i]<<endl;
		jstart[i]=0;
	}

	rflag=(int*)malloc(sizeof(int)*rnum);
}

void algo::clearRowFlags() {
	for(int i=0; i<rnum; i++)
		rflag[i]=0;
}

int algo::searchEmptyDirectly(int tid, int overflow) {
	int totalModified=0;
	for(int rid=jstart[tid]; rid<currentRowId; rid++) {
		if(alloc[tid][rid]<rwidth[rid] && remains[rid]>0) {
			int toModify=MIN(remains[rid], rwidth[rid]-alloc[tid][rid]);
			toModify=MIN(overflow, toModify);
			toModify=MIN(toModify, alloc[tid][currentRowId]);
			alloc[tid][rid]+=toModify;
			alloc[tid][currentRowId]-=toModify;
			totalModified+=toModify;
			overflow-=toModify;
		}
	}
	return totalModified;
}

int algo::searchEmpty(int tid, int rid) {
	int jstartTime=(b[rid]/period[tid])*period[tid];
	int jendTime=jstartTime+period[tid];
	int rstart=rid;
	while(b[rstart]>jstartTime)
		rstart--;
	int rend=rid;
	while(b[rend]<jendTime && rend<currentRowId)
		rend++;
	for(int rowId=rstart; rowId!=rend; rowId++) {
		if(rflag[rowId]==1)
			continue;
		if(rowId==rid)
			continue;
		if(remains[rowId]>0 && alloc[tid][rowId]<rwidth[rowId]) {
			alloc[tid][rowId]++;
			return 1;
		}
	}
	for(int rowId=rstart; rowId!=rend; rowId++) {
		if(rflag[rowId]==1)
			continue;
		if(rowId==rid)
			continue;
		if(alloc[tid][rowId]==rwidth[rowId])
			continue;
		for(int i=0; i<tnum; i++) {
			if(tid==i)
				continue;
			if(alloc[i][rowId]>0) {
				alloc[tid][rowId]++;
				alloc[i][rowId]--;
				rflag[rowId]=1;
				if(searchEmpty(i, rowId)==1)
					return 1;
				else {
					alloc[tid][rowId]--;
					alloc[i][rowId]++;
					rflag[rowId]=0;
				}
			}
		}
	}
	return 0;
}

void algo::schedule() {
	for(int rid=0; rid<rnum; rid++) {
		currentRowId=rid;
		int rbegin=b[rid];
		int rend=b[rid+1];
		cout<<"rbegin="<<rbegin<<", rend="<<rend<<endl;
		int window=rend-rbegin;
		int capacity=window*pnum;
		int ub=0, lb=0;

		cout<<"tnum="<<tnum<<endl;

		for(int i=0; i<tnum; i++) {
			most_to_exe[i]=MIN(window, execute[i]-alloced_to_exe[i]);
			ub+=most_to_exe[i];
			least_to_exe[i]=MAX(0, window-laxity[i]);
			lb+=least_to_exe[i];
			cout<<"lte["<<i<<"]="<<least_to_exe[i]<<endl;
		}

		cout<<"lb="<<lb<<endl;

		if(lb<=capacity) {
			for(int i=0; i<tnum; i++) {
				alloc[i][rid]=least_to_exe[i];
				remains[rid]-=least_to_exe[i];
			}
			for(int i=0; i<tnum; i++) {
				int extra=MIN(remains[rid], most_to_exe[i]-least_to_exe[i]);
				alloc[i][rid]+=extra;
				remains[rid]-=extra;
				if(remains[rid]==0)
					break;
			}
			for(int i=0; i<tnum; i++) {
				alloced_to_exe[i]+=alloc[i][rid];
				laxity[i]=laxity[i]+alloc[i][rid]-window;
			}
			for(int i=0; i<tnum; i++) {
				if(rend%period[i]==0) {
					if(alloced_to_exe[i]!=execute[i]) {
						cout<<"deadline miss!!!!"<<endl;					
						return ;
					}
					alloced_to_exe[i]=0;
					laxity[i]=period[i]-execute[i];
					jstart[i]=rid+1;
				}
			}
		} else {
			cout<<"something cannot solve happened!!"<<endl;
			int overflow=lb-capacity;
			for(int i=0; i<tnum; i++) {
				if(alloc[i][currentRowId]>0) {
					int toModify=searchEmptyDirectly(i, overflow);
					overflow=overflow-toModify;
					if(overflow==0)
						break;
				}
			}
		}
		cout<<"currentRow is "<<currentRowId<<endl;
		for(int i=0; i<=currentRowId; i++)
		{
			cout<<"row"<<i<<":\t";
			for(int j=0; j<tnum; j++)
				cout<<alloc[j][i]<<" ";
			cout<<"remains="<<remains[i]<<endl;
		}
	}
}
