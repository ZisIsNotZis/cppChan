#ifndef PIPE_H
#define PIPE_H
#include <condition_variable>
#include <thread>
template<class A,int N=65536/sizeof(A)+1> struct chan{
	void operator<<(const A& a){
		{
			std::unique_lock<std::mutex> l(m);
			if(i==j)full.wait(l);
			d[i]=a;
			++i;
			if(i>=N)i=0;
		}
		empty.notify_one();
	}
	void operator<<(A&& a){
		{
			std::unique_lock<std::mutex> l(m);
			if(i==j)full.wait(l);
			std::swap(a,d[i]);
			++i;
			if(i>=N)i=0;
		}
		empty.notify_one();
	}
	bool operator>>(A& a){
		size_t t=j+1;
		if(t>=N)t=0;
		if(eof&&i==j)return false;
		{
			std::unique_lock<std::mutex> l(m);
			if(i==t)empty.wait(l);
			++j;
			if(j>=N)j=0;
			if(eof&&i==t)return false;
			std::swap(a,d[j]);
		}
		full.notify_one();
		return true;
	}
	void end(){
		eof=true;
		empty.notify_all();
	}
	std::mutex m;
	std::condition_variable full,empty;
	A d[N];
	bool eof=false;
	size_t i=0,j=N-1;
};
template<class A> struct chan<A,0>{
	void operator<<(A&& a){
		{
			std::unique_lock<std::mutex> l(m);
			if(d)full.wait(l);
			d=&a;
			now.wait(l);
		}
		empty.notify_one();
	}
	bool operator>>(A& a){
		if(eof&&!d)return false;
		{
			std::unique_lock<std::mutex> l(m);
			if(!d)empty.wait(l);
			std::swap(*d,a);
			d=0;
		}
		if(eof&&!d)return false;
		full.notify_one();
		now.notify_one();
		return true;
	}
	void end(){
		eof=true;
		empty.notify_all();
	}
	std::mutex m;
	std::condition_variable full,empty,now;
	A *d;
	bool eof=false;
};
template<class A> struct pipE{
	pipE(){}
	pipE(pipE<A>&& a){
		p=a.p;
		a.p=0;
		t=std::move(a.t);
	}
	template<class B> pipE<B> operator|(void(*f)(A&,B&)){
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
pipE<chan<int,0>> pipe;
#endif
