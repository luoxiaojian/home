#include "utils.h"

void runset::addSon(int ind) {
	assert(ind>=0);
	sons.insert(ind);
}

void runset::addPeriod(int p) {
	assert(p>0);
	periods.insert(p);
}

void runset::setC(const fraction& cc) {
	c=cc;
}

range::range() : start(0), end(0) { }

range::range(const fraction& st, const fraction& en) {
	assert(!(st>en));
	start=st;
	end=en;
}

range::range(const range& r) : start(r.start), end(r.end) { }

range& range::operator=(const range& r) {
	start=r.start;
	end=r.end;
	return *this;
}

bool range::priorTo(const range& r) const{
	return (!(end>r.start));
}

fraction range::length() const{
	fraction res=end-start;
	return res;
}

bool operator<(const range& r1, const range& r2) {
	return (!(r1.end>r2.start));
}

interval::interval() {
	cap=0;
}

interval::interval(const range& r) {
	data.insert(r);	
	cap=r.end-r.start;
}

void interval::append(const range& r) {
	if(data.empty()) {
		data.insert(r);
		cap=r.end-r.start;
		return ;
	}
	set<range>::iterator last=(--data.end());
	assert(last->priorTo(r));
	if(last->end==r.start) {
		fraction newstart=last->start;
		fraction newend=r.end;
		data.erase(last);
		data.insert(range(newstart, newend));
	}
	else {
		data.insert(range(r.start, r.end));
	}
	fraction inccap=r.length();
	cap=cap+inccap;
}

interval interval::complement(const range& r) {
	interval res;
	if(cap==0) {
		res.append(r);
		return res;
	}
	set<range>::iterator first=data.begin();
	set<range>::iterator last=(--data.end());
//	cout<<"r.start="<<r.start.strval()<<endl;
//	cout<<"first->start="<<first->start.strval()<<endl;
	assert(!(r.start>first->start));
//	cout<<"r.end="<<r.end.strval()<<endl;
//	cout<<"last->end="<<last->end.strval()<<endl;
	assert(!(r.end<last->end));
	fraction loc1=r.start;
	fraction loc2;
	for(set<range>::iterator it=data.begin(); it!=data.end(); it++) {
		loc2=it->start;
		if(loc1==loc2) {
			loc1=it->end;
		} else {
			res.append(range(loc1, loc2));
			loc1=it->end;
		}
	}
	if(!(last->end==r.end)) {
		res.append(range(last->end, r.end));
	}
	res.cap=(r.end-r.start)-cap;
	return res;
}

interval interval::subtract(const fraction& len) {
	assert(!(cap<len));
	cap=cap-len;
	interval res;
	fraction remains(len);
	while(remains>0) {
		set<range>::iterator it=data.begin();
		if(!(it->length()>remains)) {
			remains=remains-it->length();
			res.append(*it);
			data.erase(it);
		} else {
			res.append(range(it->start, it->start+remains));
			fraction newstart=it->start+remains;
			fraction newend=it->end;
			data.erase(it);
			data.insert(range(newstart, newend));
			remains=0;
		}
	}
	res.cap=len;
	return res;
}

void interval::show() {
	for(set<range>::iterator it=data.begin(); it!=data.end(); it++)
		cout<<"("<<it->start.strval()<<", "<<it->end.strval()<<") ";
	cout<<endl;
}
