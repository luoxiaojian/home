#include "runtime.h"

taskstate::taskstate(int t, int pnum, int period, int execute) : tid(t), al(new fraction[pnum]), ret(execute), deadline(period) {
	for(int i=0; i<pnum; i++)
		al[i]=0;
}

job::job(int t, const fraction& s, const fraction& e) {
	tid=t;
	start=s;
	end=e;
	dur=end-start;
}

bool operator<(const allot& r1, const allot& r2) {
	return (!(r1.end>r2.start));
}

void runtime::initTSlist(const taskset& ts) {
	int pnum=ts.pnum;
	int tnum=ts.tnum;
	for(int i=0; i<tnum; i++)
		tslist.push_back(taskstate(i, pnum, ts.period[i], ts.execute[i]));
}

runtime::runtime(const taskset& ts) : tnum(ts.tnum), pnum(ts.pnum), execute(ts.execute), period(ts.period), hp(ts.lcm_period), rnum(ts.rnum), b(ts.b), ttnum(ts.ttnum){
	initTSlist(ts);
	umat=(fraction**)malloc(sizeof(fraction*)*tnum);
	for(int i=0; i<tnum; i++)
		umat[i]=new fraction[pnum];
	ui=new fraction[tnum];
	for(int i=0; i<tnum; i++)
		ui[i]=fraction(ts.execute[i], ts.period[i]);
	allocation=new vector<job>[pnum];
	allotment=new set<allot>[tnum];

	preemption=(int*)malloc(sizeof(int)*ttnum);
	migration=(int*)malloc(sizeof(int)*ttnum);
	event=(int*)malloc(sizeof(int)*ttnum);
}

static bool cmpByDeadline(const taskstate& t1, const taskstate& t2) {
	return (t1.deadline<t2.deadline);
}

void runtime::sortTSlist() {
	sort(tslist.begin(), tslist.end(), cmpByDeadline);
}

void runtime::updateUMat() {
	for(int i=0; i<tnum; i++)
		for(int j=0; j<pnum; j++)
			umat[i][j]=0;
	sortTSlist();
	fraction acc=0;
	int cur=0;
	for(vector<taskstate>::iterator it=tslist.begin(); it!=tslist.end(); it++) {
		int tid=it->tid;
		fraction start=acc;
		fraction end=start+ui[tid];
		if(end>1) {
			umat[tid][cur]=1-start;
			umat[tid][cur+1]=end-1;
			cur++;
			acc=end-1;
		} else {
			umat[tid][cur]=ui[tid];
			acc=end;
		}
/*		cout<<tid<<": ";
		for(int i=0; i<pnum; i++)
			cout<<umat[tid][i].value()<<" | ";
		cout<<endl;*/
	}
}

fraction runtime::bdg(int tid, int pid, int tend) {
	fraction res;
	vector<taskstate>::iterator it;
	for(it=tslist.begin(); it!=tslist.end(); it++)
	{
		if(it->tid==tid) {
			res=it->al[pid]+(umat[tid][pid]*(tend-it->deadline));
			break;
		}
	}
	return res;
}

void runtime::updateAL(int cur) {
	vector<int> hp;
	for(vector<taskstate>::iterator it=tslist.begin(); it!=tslist.end(); it++) {
		int tid=it->tid;
		fraction pacc;
		for(int i=0; i<pnum; i++) {
			fraction almax=it->deadline-cur;
			for(vector<int>::iterator iit=hp.begin(); iit!=hp.end(); iit++)
				almax=almax-bdg(*iit, i, it->deadline.value());
			almax=almax-pacc;
			it->al[i]=MIN(almax, it->ret-pacc);
			pacc=pacc+it->al[i];
		}
		hp.push_back(tid);
//		cout<<tid<<": ";
//		for(int i=0; i<pnum; i++)
//			cout<<it->al[i].value()<<" | ";
//		cout<<endl;
	}
}

void runtime::fakeAlloc(const fraction& cap) {
	fraction *remains=new fraction[pnum];
	for(int i=0; i<pnum; i++)
		remains[i]=cap;
	for(vector<taskstate>::iterator it=tslist.begin(); it!=tslist.end(); it++) {
		for(int i=0; i<pnum; i++)
		{
			fraction all=MIN(remains[i], it->al[i]);
			if(all>0)
			{
				remains[i]=remains[i]-all;
				it->al[i]=it->al[i]-all;
				it->ret=it->ret-all;
			}
		}
	}
}

void runtime::alloc(int start, int end) {
	fraction *cur=new fraction[pnum];
	fraction *cap=new fraction[pnum];
	for(int i=0; i<pnum; i++)
	{
		cur[i]=start;
		cap[i]=end-start;
	}
	for(vector<taskstate>::iterator it=tslist.begin(); it!=tslist.end(); it++) {
		for(int i=0; i<pnum; i++)
		{
			fraction all=MIN(cap[i], it->al[i]);
			if(all>0) {
				cap[i]=cap[i]-all;
				it->al[i]=it->al[i]-all;
				it->ret=it->ret-all;
				vector<job>::iterator last=(--allocation[i].end());
				if((!allocation[i].empty()) && last->tid==it->tid) {
					last->end=cur[i]+all;
					last->dur=last->end-last->start;
				} else
					allocation[i].push_back(job(it->tid, cur[i], cur[i]+all));
				allotment[it->tid].insert(allot(i, cur[i], cur[i]+all));
				cur[i]=cur[i]+all;
			}
		}
	}

}

void runtime::updateState(int cur, int next) {
	for(vector<taskstate>::iterator it=tslist.begin(); it!=tslist.end(); it++) {
		if(it->deadline==next) {
			if(!(it->ret==0)) {
				cout<<"!!!!!"<<it->tid<<"alloc error"<<"\tret="<<it->ret.strval()<<endl;
				exit(0);
			}
	//		else
	//			cout<<"---->task "<<it->tid<<" meet its deadline"<<endl;
			it->ret=execute[it->tid];
			it->deadline=next+period[it->tid];
		}
	}
}

void runtime::outputAlloc() const {
	for(int i=0; i<pnum; i++)
	{
		cout<<"p"<<i<<": |";
		for(vector<job>::iterator it=allocation[i].begin(); it!=allocation[i].end(); it++)
			cout<<" "<<it->tid<<", "<<it->dur.strval()<<" |";
		cout<<endl;
	}
}

void runtime::outputAllot() const {
	for(int i=0; i<ttnum; i++) {
		cout<<"t"<<i<<": |";
		for(set<allot>::iterator it=allotment[i].begin(); it!=allotment[i].end(); it++)
			cout<<" "<<it->pid<<", ["<<it->start.strval()<<", "<<it->end.strval()<<"] |";
		cout<<endl;
	}
}

void runtime::processAllotment() {
	for(int i=0; i<tnum; i++) {
		set<allot>::iterator cur=allotment[i].begin();
		while(cur!=allotment[i].end()) {
			set<allot>::iterator next=cur;
			next++;
			if((next->pid==cur->pid) && (next->start==cur->end)) {
				allot tmp(cur->pid, cur->start, next->end);
				allotment[i].erase(cur);
				allotment[i].erase(next);
				allotment[i].insert(tmp);
				cur=allotment[i].begin();
			}
			else {
				cur++;
			}
		}
	}
}

void runtime::run(FILE *flog) {
	for(int i=0; i<rnum; i++) {
		updateUMat();
		updateAL(b[i]);
		alloc(b[i], b[i+1]);
		updateState(b[i], b[i+1]);
	}
	processAllotment();

	countPreemption();
	countMigration();
	countEvent();

	flushLog(flog);
}

void runtime::countPreemption() {
	for(int i=0; i<ttnum; i++) {
		int count=0;
		for(set<allot>::iterator it=allotment[i].begin(); it!=allotment[i].end(); it++) {
			fraction st=it->start/period[i];
			fraction en=it->end/period[i];
			int lb=st.floor();
			int hb=en.ceil()-1;
			if(hb==lb)
				count+=1;
			else
				count+=2;
		}
		count=count-(hp/period[i]);
		preemption[i]=count;
	}
}

void runtime::countMigration() {
	for(int i=0; i<ttnum; i++) {
		int count=0;
		int last=-1;
		for(set<allot>::iterator it=allotment[i].begin(); it!=allotment[i].end(); it++) {
			if(last==-1) {
				last=it->pid;
			}
			else {
				if(last!=it->pid)
					count+=1;
				last=it->pid;
			}
		}
		migration[i]=count;
	}
}

void runtime::countEvent() {
	for(int i=0; i<ttnum; i++) {
		int count=0;
		int lastloc=-1;
		fraction last;
		for(set<allot>::iterator it=allotment[i].begin(); it!=allotment[i].end(); it++) {
			if(lastloc==-1)
				lastloc=it->pid;
			else {
				if(lastloc!=it->pid) {
					fraction cur=it->start/period[i];
					int small=last.ceil()-1;
					int big=cur.floor();
					if(small==big)
						count++;
				}
				lastloc=it->pid;
			}
			last=it->end/period[i];
		}
		event[i]=preemption[i]+migration[i]-count;
	}
}

void runtime::flushLog(FILE *flog) {
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

