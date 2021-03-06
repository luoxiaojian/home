#include "algo.h"
#include <iostream>
#include <set>
#include <stdlib.h>

#define MIN(a, b) ((a)>(b)?(b):(a))
#define MAX(a, b) ((a)<(b)?(b):(a))

//#define DISPLAY

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
#ifdef _DISPLAY
		cout<<"laxity["<<i<<"]="<<laxity[i]<<endl;
#endif
		jstart[i]=0;
	}

	rflag=(int*)malloc(sizeof(int)*rnum);
	for(int i=0; i<rnum; i++)
		rflag[i]=0;
}

void algo::clearFlags() {
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

int algo::searchEmptyInDirectly(int tid, int rid) {
	rflag[rid]=1;
#ifdef DISPLAY
	cout<<"["<<tid<<", "<<rid<<"] ";
#endif
	int startTime=(b[rid]/period[tid])*period[tid];
	int endTime=startTime+period[tid];
	int startRowId=rid;
	while(b[startRowId]>startTime)
		startRowId--;
	int endRowId=rid;
	while(b[endRowId]<endTime)
		endRowId++;
	endRowId=MIN(endRowId, currentRowId);
	for(int i=startRowId; i<endRowId; i++) {
		if(remains[i]>0 && alloc[tid][i]<rwidth[i]) {
			remains[i]--;
			alloc[tid][i]++;
			rflag[rid]=0;
#ifdef DISPLAY
			cout<<"("<<i<<") success"<<endl;
#endif
			return 1;
		}
	}
	for(int i=startRowId; i<endRowId; i++) {
		if(rflag[i]==1)
			continue;
		if(alloc[tid][i]==rwidth[i])
			continue;
		for(int j=0; j<tnum; j++) {
			if(alloc[j][i]>0) {
				alloc[j][i]--;
				alloc[tid][i]++;
				if(searchEmptyInDirectly(j, i)==1) {
			//		rflag[rid]=0;
					return 1;
				}
				alloc[j][i]++;
				alloc[tid][i]--;
			}	
		}
	}
//	rflag[rid]=0;
	return 0;
}

int algo::searchTaskDirectly(int tid, int overflow) {
	int rstart=jstart[tid];
	int rend=currentRowId;
	int totalModified=0;
	for(int kid=0; kid<tnum; kid++) {
		if(kid==tid)
			continue;
		if(!isDstTask(kid))
			continue;
		for(int i=MAX(rstart, jstart[kid]); i<rend; i++) {
			if(alloc[tid][i]<rwidth[i] && alloc[kid][i]>0) {
				int toModify=MIN(alloc[tid][currentRowId], overflow);
				toModify=MIN(toModify, laxity[kid]);
				toModify=MIN(toModify, rwidth[i]-alloc[tid][i]);
				toModify=MIN(toModify, alloc[kid][i]);
				alloc[tid][currentRowId]-=toModify;
				alloc[tid][i]+=toModify;
				alloc[kid][i]-=toModify;
				laxity[kid]-=toModify;
				laxity[tid]-=toModify;
				overflow-=toModify;
				totalModified+=toModify;
			}
			if(overflow==0 || alloc[tid][currentRowId]==0)
				return totalModified;
			if(laxity[kid]==0)
				break;
		}
	}
	return totalModified;
}
 
int algo::searchTaskInDirectly(int tid, int rid) {
	int startTime=(b[rid]/period[tid])*period[tid];
	int endTime=MIN(startTime+period[tid], currentRowId);
	int rstart=rid;
	while(b[rstart]>startTime)
		rstart--;
	int rend=rid;
	while(b[rend]<endTime)
		rend++;
	rflag[rid]=1;
	for(int i=rstart; i<rend; i++) {
		if(rflag[i]==1)
			continue;
		if(alloc[tid][i]==rwidth[i])
			continue;
		for(int j=0; j<tnum; j++) {
			if(j==tid)
				continue;
			if(alloc[j][i]==0)
				continue;
			if(isDstTask(j) && i>=jstart[j]) {
				alloc[tid][i]--;
				alloc[j][i]++;
				laxity[j]--;
				return 1;
			} else {
				if(searchTaskInDirectly(j, i)) {
					alloc[tid][i]--;
					alloc[j][i]++;
					laxity[j]--;
					return 1;
				}
			}
		}
	}
	return 0;
}

int algo::isSrcTask(int tid) {
	if(alloc[tid][currentRowId]>0)
		return 1;
	else
		return 0;
}

int algo::isDstTask(int tid) {
	int curLaxity=laxity[tid]+alloc[tid][currentRowId]-b[currentRowId+1]+b[currentRowId];
	if(curLaxity>0)
		return 1;
	else
		return 0;
}

int algo::schedule() {
	for(int rid=0; rid<rnum; rid++) {
		currentRowId=rid;
		int rbegin=b[rid];
		int rend=b[rid+1];
#ifdef DISPLAY
		cout<<"rbegin="<<rbegin<<", rend="<<rend<<endl;
#endif
		int window=rend-rbegin;
		int capacity=window*pnum;
		int ub=0, lb=0;

#ifdef DISPLAY
		cout<<"tnum="<<tnum<<endl;
#endif

		for(int i=0; i<tnum; i++) {
			most_to_exe[i]=MIN(window, execute[i]-alloced_to_exe[i]);
			ub+=most_to_exe[i];
			least_to_exe[i]=MAX(0, window-laxity[i]);
			lb+=least_to_exe[i];
#ifdef DISPLAY
			cout<<"lte["<<i<<"]="<<least_to_exe[i]<<endl;
#endif
		}

#ifdef DISPLAY
		cout<<"lb="<<lb<<endl;
#endif

		if(lb<=capacity) {
			int left=remains[rid];
			for(int i=0; i<tnum; i++) {
				alloc[i][rid]=least_to_exe[i];
				left-=least_to_exe[i];
			}
			for(int i=0; i<tnum; i++) {
				int extra=MIN(left, most_to_exe[i]-least_to_exe[i]);
				alloc[i][rid]+=extra;
				left-=extra;
				if(left==0)
					break;
			}
		} else {
			int overflow=lb-capacity;
			for(int i=0; i<tnum; i++) {
				alloc[i][rid]=least_to_exe[i];
			}
			for(int i=0; i<tnum; i++) {
				if(alloc[i][currentRowId]>0) {
					int toModify=searchEmptyDirectly(i, overflow);
					overflow=overflow-toModify;
					if(overflow==0)
						break;
				}
			}
			for(int i=0; i<tnum; i++) {
				while(alloc[i][currentRowId]>0 && overflow>0) {
					alloc[i][currentRowId]--;
					overflow--;
					clearFlags();
					if(searchEmptyInDirectly(i, currentRowId)!=1) {
#ifdef DISPLAY
						cout<<endl;
#endif
						alloc[i][currentRowId]++;
						overflow++;
						break;
					}
				}
				if(overflow==0)
					break;
			}
			if(overflow>0) {
				for(int i=0; i<tnum; i++) {
					if(!isSrcTask(i))
						continue;
					int toModify=searchTaskDirectly(i, overflow);
					overflow=overflow-toModify;
					if(overflow==0)
						break;
				}
				for(int i=0; i<tnum; i++) {
					if(!isSrcTask(i))
						continue;
					clearFlags();
					while(searchTaskInDirectly(i, currentRowId)) {
						alloc[i][currentRowId]--;
						overflow--;
						clearFlags();
						if(overflow==0)
							break;
					}
					if(overflow==0)
						break;
				}
			}
		}
		for(int i=0; i<tnum; i++) {
			alloced_to_exe[i]=0;
			int elapsedTime=0;
			for(int j=jstart[i]; j<=currentRowId; j++) {
				alloced_to_exe[i]+=alloc[i][j];
				elapsedTime+=rwidth[j];
			}
			laxity[i]=period[i]-execute[i]+alloced_to_exe[i]-elapsedTime;
			remains[currentRowId]-=alloc[i][currentRowId];
			if(rend%period[i]==0) {
				if(alloced_to_exe[i]!=execute[i]) {
					cout<<"error: deadline miss!!!"<<endl;
					return -1;
				}
				if(laxity[i]<0) {
					cout<<"error: laxity is negative!!!"<<endl;
					return -1;
				}
				alloced_to_exe[i]=0;
				laxity[i]=period[i]-execute[i];
				jstart[i]=currentRowId+1;
			}
		}
#ifdef DISPLAY
		cout<<"currentRow is "<<currentRowId<<endl;
		for(int i=0; i<=currentRowId; i++)
		{
			cout<<"row"<<i<<":\t";
			for(int j=0; j<tnum; j++)
				cout<<alloc[j][i]<<" ";
			cout<<"remains="<<remains[i]<<endl;
		}
#endif
	}
	return 1;
}

void algo::checkSchedule() {
	int *sumExe;
	sumExe=(int*)malloc(sizeof(int)*tnum);
	for(int i=0; i<tnum; i++) 
		sumExe[i]=0;
	for(int i=0; i<rnum; i++) {
		int tmpSum=0;
		for(int j=0; j<tnum; j++) {
			tmpSum+=alloc[j][i];
			if(alloc[j][i]>rwidth[i]) {
				cout<<"error: alloc exceed row width..."<<endl;
				return ;
			}
		}
		int capacity=rwidth[i]*pnum;
		if(tmpSum>capacity) {
			cout<<"error: sum of a row exceed capacity..."<<endl;
			return ;
		}
	}
	for(int i=0; i<rnum; i++) {
		for(int j=0; j<tnum; j++) {
			sumExe[j]+=alloc[j][i];
			if(b[i+1]%period[j]==0) {
				if(sumExe[j]!=execute[j]) {
					cout<<"error: a task miss deadline..."<<endl;
					return ;
				}
				sumExe[j]=0;
			}
		}
	}
	cout<<"schedule success..."<<endl;
	return ;
}
