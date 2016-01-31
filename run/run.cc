#include "run.h"
#include <iostream>

using std::cout;
using std::endl;

#define INF 0x3fffffff

bool operator<(const unit& r1, const unit& r2) {
	return !(r1.time.end>r2.time.start);
}

run::run(FILE *fin) {
	fscanf(fin, "%d%d%d%d", &ttnum, &tnum, &pnum, &hp);
	period=(int *)malloc(sizeof(int)*tnum);
	execute=(int *)malloc(sizeof(int)*tnum);
	for(int i=0; i<tnum; i++)
		fscanf(fin, "%d", &period[i]);
	for(int i=0; i<tnum; i++)
		fscanf(fin, "%d", &execute[i]);
	dispatch=new vector<unit>[pnum];
	locate=new set<unit>[tnum];

	preemption=(int*)malloc(sizeof(int)*ttnum);
	migration=(int*)malloc(sizeof(int)*ttnum);
	event=(int*)malloc(sizeof(int)*ttnum);

	for(int i=0; i<tnum; i++)
		tasks.push_back(runtask(execute[i], period[i]));

	for(int i=0; i<MAX_DEEP; i++)
	{
		numOfLevel[i]=0;
		indOfLevel[i]=0;
	}
	levelNum=0;

	for(int i=0; i<tnum; i++)
	{
		runset tmps(DUALED);
		tmps.setC(fraction(execute[i], period[i]));
		tmps.deadline=0;
		tmps.addPeriod(period[i]);
		sets.push_back(tmps);
	}

/*	for(int i=0; i<tnum; i++) {
		cout<<sets[i].c.strval()<<endl;
	}*/

	numOfLevel[0]=tnum;
	indOfLevel[0]=0;
	indOfLevel[1]=tnum;
	levelNum++;

	do{
		int from=indOfLevel[levelNum-1];
		int to=from;
		int target=indOfLevel[levelNum];
		fraction sum(0);
		while(1) {
			fraction tmp(0);
			if(to==indOfLevel[levelNum] || sum+sets[to].c>1) {
				sets.push_back(runset(PACKED));
				sets[target].setC(sum);
				for(int i=from; i<to; i++) {
					sets[i].father=target;
					sets[target].addSon(i);
					for(set<int>::iterator it=sets[i].periods.begin(); it!=sets[i].periods.end(); it++)
						sets[target].addPeriod(*it);
				}
				from=to;
				sum=0;
				target++;
				if(to==indOfLevel[levelNum])
					break;
			}
			else {
				sum=sum+sets[to].c;
				to++;
			}
		}
		levelNum++;
		numOfLevel[levelNum-1]=target-indOfLevel[levelNum-1];
		indOfLevel[levelNum]=target;

		if(sumOfLevel(levelNum-1)==1)
			break;

		for(int i=0; i<numOfLevel[levelNum-1]; i++) {
			int src=i+indOfLevel[levelNum-1];
			int tar=i+indOfLevel[levelNum];
			sets.push_back(runset(DUALED));
			sets[tar].setC(1-sets[src].c);
			sets[tar].addSon(src);
			sets[src].father=tar;
			for(set<int>::iterator it=sets[src].periods.begin(); it!=sets[src].periods.end(); it++)
				sets[tar].addPeriod(*it);
		}
		levelNum++;
		numOfLevel[levelNum-1]=numOfLevel[levelNum-2];
		indOfLevel[levelNum]=indOfLevel[levelNum-1]+numOfLevel[levelNum-1];
	} while(sumOfLevel(levelNum-1)!=1);

	if(numOfLevel[levelNum-1]==1)
		root=indOfLevel[levelNum-1];
	else {
		root=indOfLevel[levelNum];
		sets.push_back(runset(PACKED));
		sets[root].setC(1);
		for(int i=indOfLevel[levelNum-1]; i!=root; i++) {
			sets[i].father=root;
			sets[root].addSon(i);
			for(set<int>::iterator it=sets[i].periods.begin(); it!=sets[i].periods.end(); it++)
				sets[root].addPeriod(*it);
		}
	}

	updateDeadline(0);
	generateRow();
}

run::~run() {
	free(preemption);
	free(migration);
	free(event);
	free(execute);
	free(period);
	
	free(rind);

	delete[] dispatch;
	delete[] locate;
}

run::run(int tt, int t, int p, int h, int *pe, int *e) {
	ttnum=tt;
	tnum=t;
	pnum=p;
	hp=h;

	period=(int *)malloc(sizeof(int)*tnum);
	execute=(int *)malloc(sizeof(int)*pnum);
	for(int i=0; i<tnum; i++)
		period[i]=pe[i];
	for(int i=0; i<tnum; i++)
		execute[i]=e[i];
	dispatch=new vector<unit>[pnum];


	for(int i=0; i<tnum; i++)
		tasks.push_back(runtask(execute[i], period[i]));

	for(int i=0; i<MAX_DEEP; i++)
	{
		numOfLevel[i]=0;
		indOfLevel[i]=0;
	}
	levelNum=0;

	for(int i=0; i<tnum; i++)
	{
		runset tmps(DUALED);
		tmps.setC(fraction(execute[i], period[i]));
		tmps.deadline=0;
		tmps.addPeriod(period[i]);
		sets.push_back(tmps);
	}

	numOfLevel[0]=tnum;
	indOfLevel[0]=0;
	indOfLevel[1]=tnum;
	levelNum++;

	do{
		int from=indOfLevel[levelNum-1];
		int to=from;
		int target=indOfLevel[levelNum];
		fraction sum(0);
		while(1) {
			fraction tmp(0);
			if(to==indOfLevel[levelNum] || sum+sets[to].c>1) {
				sets.push_back(runset(PACKED));
				sets[target].setC(sum);
				for(int i=from; i<to; i++) {
					sets[i].father=target;
					sets[target].addSon(i);
					for(set<int>::iterator it=sets[i].periods.begin(); it!=sets[i].periods.end(); it++)
						sets[target].addPeriod(*it);
				}
				from=to;
				sum=0;
				target++;
				if(to==indOfLevel[levelNum])
					break;
			}
			else {
				sum=sum+sets[to].c;
				to++;
			}
		}
		levelNum++;
		numOfLevel[levelNum-1]=target-indOfLevel[levelNum-1];
		indOfLevel[levelNum]=target;

		if(sumOfLevel(levelNum-1)==1)
			break;

		for(int i=0; i<numOfLevel[levelNum-1]; i++) {
			int src=i+indOfLevel[levelNum-1];
			int tar=i+indOfLevel[levelNum];
			sets.push_back(runset(DUALED));
			sets[tar].setC(1-sets[src].c);
			sets[tar].addSon(src);
			sets[src].father=tar;
			for(set<int>::iterator it=sets[src].periods.begin(); it!=sets[src].periods.end(); it++)
				sets[tar].addPeriod(*it);
		}
		levelNum++;
		numOfLevel[levelNum-1]=numOfLevel[levelNum-2];
		indOfLevel[levelNum]=indOfLevel[levelNum-1]+numOfLevel[levelNum-1];
	} while(sumOfLevel(levelNum-1)!=1);

	if(numOfLevel[levelNum-1]==1)
		root=indOfLevel[levelNum-1];
	else {
		root=indOfLevel[levelNum];
		sets.push_back(runset(PACKED));
		sets[root].setC(1);
		for(int i=indOfLevel[levelNum-1]; i!=root; i++) {
			sets[i].father=root;
			sets[root].addSon(i);
			for(set<int>::iterator it=sets[i].periods.begin(); it!=sets[i].periods.end(); it++)
				sets[root].addPeriod(*it);
		}
	}

	updateDeadline(0);
	generateRow();
}


int run::sumOfLevel(int lid) {
	fraction sum(0);
	for(int i=indOfLevel[lid]; i!=indOfLevel[lid+1]; i++)
	{
		sum=sum+sets[i].c;
	}
//	assert(sum.isInt());
//cout<<"level "<<lid<<"\t"<<sum.strval()<<endl;
	return sum.intval();
}

static bool isDeadline(int cur, const set<int> &periods) {
	for(set<int>::iterator it=periods.begin(); it!=periods.end(); it++)
		if(cur%(*it)==0)
			return true;
	return false;
}

void run::updateDeadline(int cur) {
	for(int i=0; i<tnum; i++) {
//		cout<<"in task"<<i<<":\t"<<"toExecute="<<sets[i].toExecute.strval()<<", executed="<<sets[i].executed.strval()<<endl;
		if(sets[i].deadline==cur) {
			assert(sets[i].toExecute==sets[i].executed);
			sets[i].deadline+=period[i];
			sets[i].toExecute=execute[i];
			sets[i].executed=0;
		}
	}
	for(int i=tnum; i<=root; i++) {
//		cout<<"in set"<<i<<":\t"<<"toExecute="<<sets[i].toExecute.strval()<<", executed="<<sets[i].executed.strval()<<endl;
		if(isDeadline(cur, sets[i].periods)) {
			assert(sets[i].executed==sets[i].toExecute);
			sets[i].executed=0;
			int tmp=INF;
			for(set<int>::iterator it=sets[i].sons.begin(); it!=sets[i].sons.end(); it++) {
				if(tmp>sets[*it].deadline)
					tmp=sets[*it].deadline;
			}
			sets[i].deadline=tmp;
			int dur=tmp-cur;
			sets[i].toExecute=dur*sets[i].c;
		}
	}
}

void run::allocARow(int rid) {
	assert(rid<rnum);
	int from=rind[rid];
	int to=rind[rid+1];
	sets[root].alloc=interval(range(from, to));
	sets[root].executed=sets[root].executed+sets[root].alloc.cap;
	allocAServer(root, from, to);
	updateDeadline(to);
}

void run::schedule() {
	for(int i=0; i<rnum; i++) {
//		cout<<"in row"<<i<<"\t["<<rind[i]<<", "<<rind[i+1]<<"]"<<endl;
		allocARow(i);
//		cout<<"-----------------------------------------"<<endl;
	}
//	cout<<"schedule finished!"<<endl;
}

void run::allocAServer(int sid, int start, int end) {
//	cout<<"In server"<<sid<<endl;
	if(sets[sid].sons.empty()) {
		assert((sid<tnum) && (sid>=0));
		for(set<range>::iterator it=sets[sid].alloc.data.begin(); it!=sets[sid].alloc.data.end(); it++)
		{
//			cout<<"------>task"<<sid<<" is alloced with["<<it->start.strval()<<", "<<it->end.strval()<<"]"<<endl;
			tasks[sid].alloc.append(*it);
		}
	}
	else if(sets[sid].state==DUALED) {
		int sonid=*(sets[sid].sons.begin());
		sets[sonid].alloc=sets[sid].alloc.complement(range(start, end));
//		cout<<"set"<<sonid<<"was allocated with ";
//		sets[sonid].alloc.show();
		sets[sonid].executed=sets[sonid].executed+sets[sonid].alloc.cap;
//		cout<<"length is"<<sets[sonid].executed.strval()<<"\t"<<endl;
		allocAServer(sonid, start, end);
	} else if(sets[sid].state==PACKED) {
		int sonnum=sets[sid].sons.size();
		int *slist=(int *)malloc(sizeof(int)*sonnum);
		int i=0;
		for(set<int>::iterator it=sets[sid].sons.begin(); it!=sets[sid].sons.end(); it++) {
			slist[i]=*it;
			i++;
		}
		for(int i=1; i<sonnum; i++)
			for(int j=0; j<sonnum-i; j++)
			{
				int tmp;
				if(sets[slist[j]].deadline>sets[slist[j+1]].deadline) {
					tmp=slist[j];
					slist[j]=slist[j+1];
					slist[j+1]=tmp;
				}
			}
		fraction remains(sets[sid].alloc.cap);
		for(int i=0; i<sonnum; i++) {
			int sonid=slist[i];
			fraction toAlloc=sets[sonid].toExecute-sets[sonid].executed;
			if(toAlloc>remains)
				toAlloc=remains;
			sets[sonid].executed+=toAlloc;
			sets[sonid].alloc=sets[sid].alloc.subtract(toAlloc);
			remains=remains-toAlloc;
		}
		for(int i=0; i<sonnum; i++)
			allocAServer(slist[i], start, end);

	} else {
		assert(0);
	}
}

void run::generateRow() {
	int *flag=(int*)malloc(sizeof(int)*hp);
	for(int i=0; i<hp; i++)
		flag[i]=0;
	for(int i=0; i<tnum; i++)
		for(int j=0; j<hp/period[i]; j++)
			flag[j*period[i]]=1;
	rnum=0;
	for(int i=0; i<hp; i++)
		rnum+=flag[i];
	rind=(int*)malloc(sizeof(int)*(rnum+1));
	int j=0;
	for(int i=0; i<hp; i++)
	{
		if(flag[i]==1) {
			rind[j]=i;
			j++;
		}
	}
	rind[rnum]=hp;
}

void run::dispatchToCore() {
	for(int i=0; i<pnum; i++) {
		fraction start=0;
		while(!(start==hp)) {
			for(int j=0; j<tnum; j++) {
				for(set<range>::iterator it=tasks[j].alloc.data.begin(); it!=tasks[j].alloc.data.end(); it++) {
					if(it->start>start)
						break;
					if(it->start==start)
					{
						dispatch[i].push_back(unit(j, range(it->start, it->end)));
						locate[j].insert(unit(i, range(it->start, it->end)));
						start=it->end;
						tasks[j].alloc.data.erase(it);
					}
				}
			}
		}	
//		cout<<"Finished processor"<<i<<endl;
	}
}

bool run::checkSchedule() {
	for(int i=0; i<tnum; i++) {
		for(int j=0; j<hp/period[i]; j++) {
			int js=j*period[i];
			int je=js+period[i];
			fraction count=0;
			for(set<range>::iterator it=tasks[i].alloc.data.begin(); it!=tasks[i].alloc.data.end(); it++) {
				if(!(it->end>js))
					continue;
				if(!(it->start<je))
					break;
				fraction loc1, loc2;
				loc1=it->start;
				if(it->start<js)
					loc1=js;
				loc2=it->end;
				if(it->end>je)
					loc2=je;
				count=count+(loc2-loc1);
			}
			if(!(count==execute[i])) {
//				cout<<"task"<<i<<" alloc error!!"<<endl;
//				cout<<"count="<<count.strval()<<" while execute="<<execute[i]<<endl;
				return false;
			}
		}
	}
	return true;
}

void run::countPreemption() {
	for(int i=0; i<ttnum; i++) {
		int count=0;
		for(set<range>::iterator it=tasks[i].alloc.data.begin(); it!=tasks[i].alloc.data.end(); it++) {
			fraction st=it->start/period[i];
			fraction en=it->end/period[i];
			int lb=st.floor();
			int hb=en.ceil()-1;
			if(hb==lb)
				count+=1;
			else
				count+=2;
		}
		count-=(hp/period[i]);
		preemption[i]=count;
	}
}

void run::countMigration() {
	for(int i=0; i<ttnum; i++) {
		int count=0;
		int last=-1;
		for(set<unit>::iterator it=locate[i].begin(); it!=locate[i].end(); it++)
		{
			if(last==-1)
				last=it->tid;
			else {
				if(last!=it->tid)
					count+=1;
				last=it->tid;
			}
		}
		migration[i]=count;
	}
}

void run::countEvent() {
	for(int i=0; i<ttnum; i++) {
		int count=0;
		int lastloc=-1;
		fraction last;
		for(set<unit>::iterator it=locate[i].begin(); it!=locate[i].end(); it++)
		{
			if(lastloc==-1)
				lastloc=it->tid;
			else {
				if(lastloc!=it->tid) {
					fraction cur=it->time.start/period[i];
					int small=last.ceil()-1;
					int big=cur.floor();
					if(small==big)
						count++;
				}
				lastloc=it->tid;
			}
			last=it->time.end/period[i];
		}
		event[i]=preemption[i]+migration[i]-count;
	}
}

void run::flushLog(FILE *flog) {
	int i;
	fprintf(flog, "preemption:\n");
	for(i=0; i<ttnum; i++)
	{
		double ppj=((double)preemption[i])/((double)(hp/period[i]));
		fprintf(flog, "%lf ", ppj);
	}
	fprintf(flog, "\n");
	fprintf(flog, "migration:\n");
	for(i=0; i<ttnum; i++)
	{
		double mpj=((double)migration[i])/((double)(hp/period[i]));
		fprintf(flog, "%lf ", mpj);
	}
	fprintf(flog, "\n");
	fprintf(flog, "event:\n");
	for(i=0; i<ttnum; i++)
	{
		double epj=((double)event[i])/((double)(hp/period[i]));
		fprintf(flog, "%lf ", epj);
	}
	fprintf(flog, "\n");
}

