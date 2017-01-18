#ifndef CHANNEL_H
#define CHANNEL_H
#include <mutex>
template<class A,int buf=65536/sizeof(A)> struct bufchannel{
	bufchannel(){
		ouT.lock();
	}
	void operator<<(A& a){
		if(i==buf){
			iN.lock();
			std::swap(in,mid);
			i=0;
			j=buf;
			ouT.unlock();
		}
		in[i]=std::move(a);
		++i;
	}
	void operator<<(A&& a){operator<<(a);}
	bool operator>>(A& a){
		if(k==K){
			ouT.lock();
			if(eof){if(!j)return false;else ouT.unlock();}
			std::swap(mid,out);
			K=j;
			j=0;
			k=0;
			iN.unlock();
		}
		a=std::move(out[k]);
		++k;
		return true;
	}
	void end(){
		iN.lock();
		eof=true;
		std::swap(in,mid);
		j=i;
		ouT.unlock();
	}
	A in[buf],mid[buf],out[buf];
	size_t i=0,j=0,k=0,K=0;
	std::mutex iN,ouT;
	bool eof=false;
};
template<class A,int buf=65536/sizeof(A)> struct channel{
	channel(){
		empty.lock();
	}
	void operator<<(A& a){
		full.lock();
		full.unlock();
		th.lock();
		empty.unlock();
		dat[i]=std::move(a);
		++i;
		if(i>=buf)i=0;
		if(i==j)full.lock();
		th.unlock();
	}
	void operator<<(A&& a){operator<<(a);}
	bool operator>>(A& a){
		empty.lock();
		empty.unlock();
		th.lock();
		full.unlock();
		++j;
		if(j>=buf)j=0;
		if(i==j){if(eof)return false;else empty.lock();}
		a=std::move(dat[j]);
		th.unlock();
		return true;
	}
	void end(){
		th.lock();
		eof=true;
		empty.unlock();
		th.unlock();
	}
	A dat[buf];
	size_t i=0,j=buf-1;
	std::mutex full,empty,th;
	bool eof=false;
};
#endif