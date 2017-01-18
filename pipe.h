#ifndef PIPE_H
#define PIPE_H
#include <mutex>
#include <thread>
template<class A,int n=65536/sizeof(A)> struct chan{
	chan(){
		empty.lock();
	}
	void operator<<(const A& a){
		full.lock();
		full.unlock();
		th.lock();
		empty.unlock();
		dat[i]=std::move(a);
		++i;
		if(i>=n)i=0;
		if(i==j)full.lock();
		th.unlock();
	}
	void operator<<(auto&& a){
		full.lock();
		full.unlock();
		th.lock();
		empty.unlock();
		dat[i]=std::move(a);
		++i;
		if(i>=n)i=0;
		if(i==j)full.lock();
		th.unlock();
	}
	bool operator>>(A& a){
		empty.lock();
		empty.unlock();
		th.lock();
		full.unlock();
		++j;
		if(j>=n)j=0;
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
	A dat[n];
	size_t i=0,j=n-1;
	std::mutex full,empty,th;
	bool eof=false;
};
template<class A,int n=65536/sizeof(A)> struct bufchan{
	bufchan(){
		empty.lock();
	}
	void operator<<(const A& a){
		if(i==n){
			full.lock();
			std::swap(in,mid);
			i=0;
			j=n;
			empty.unlock();
		}
		in[i]=a;
		++i;
	}
	void operator<<(auto&& a){
		if(i==n){
			full.lock();
			std::swap(in,mid);
			i=0;
			j=n;
			empty.unlock();
		}
		in[i]=std::move(a);
		++i;
	}
	bool operator>>(A& a){
		if(k==K){
			empty.lock();
			if(eof){if(!j)return false;else empty.unlock();}
			std::swap(mid,out);
			K=j;
			j=0;
			k=0;
			full.unlock();
		}
		a=std::move(out[k]);
		++k;
		return true;
	}
	void end(){
		full.lock();
		eof=true;
		std::swap(in,mid);
		j=i;
		empty.unlock();
	}
	A in[n],mid[n],out[n];
	size_t i=0,j=0,k=0,K=0;
	std::mutex full,empty;
	bool eof=false;
};
template<class A> struct pipE{
	pipE(){}
	pipE(pipE<A>&& a){
		p=a.p;
		a.p=0;
		t=std::move(a.t);
	}
	template<class B>
	pipE<B> operator|(void(*f)(A&,B&)){
		pipE<B> P;
		P.p=new B;
		t=std::thread(f,std::ref(*p),std::ref(*P.p));
		return P;
	}
	~pipE(){
		try{t.join();}catch(std::system_error){};
		delete p;
	}
	A *p=0;
	std::thread t;
};
pipE<bufchan<void*>> pipe;
#endif