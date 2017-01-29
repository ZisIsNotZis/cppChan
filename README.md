# C++ Channel Class
C++ channel implementation like what's in Go: Instead of using the traditional, compilcated, and troublesome mutex/condition\_variable to do data passing / thread progress syncing, it would be much easier for programmer to build inter-thread communitaion using channels. 

Any thread can push data to a channel. After that, any thread (including the pusher) will be able to fetch that data from that channel. If a thread tries to push to a channel while it's full (or have no buffer), the thread will be freezed until free push is possible. Similarly, pull will be freezed when there's no data.

A single channel can have multiple sender/worker threads inputing to it and multiple receiver/consumer threads getting from it.

The repo also includes a pipeline helper class, which is still being designed. It's working, but I'm thinking about some better design:

C++ pipeline implementation like what's in shells (e.g. BASH): Instead of using the traditional, compilcated and troublesome std::thread, things looks easy and clear if written in pipeline format (i.e. `func1(...) | func2(...) | func3(...) | func4(...)`).

Pipeline is a connected "thread/process graph" showing the flow direction of data. For the example above, `func1` is a function which generates data. The data is received by `func2`, porcessed, and passed to `func3`. After that `func3` does the same thing and finally `func4` takes the data and does something with it (like printing to stdout, writing to file, making sound, or etc).

Here all datas are streamed, which means as soon as `func1` outputs the first bit, `func2` will start working, and so does `func3` and `func4`. All these 4 functions will work at the same time in 4 threads.

# Usage
```
#include "chan.hpp"
int main(){

	chan<int,0> cH;	// create a channel with no buffer

	cH << 123;		// push 123 to the channel, wait for somebody to get it
					// THIS IS WRONG! no-buffer channel will not be able to hold anything
					// the program will be sleeping until some threads gets the data, but
					// there is no other thread! So this will generate a dead lock which
					// is only detectable using some runtime sanitizer.
	
	chan<int> ch;	// this time, create a channel with default buffer size (64KB)

	for(i=0;i<100;++i)
		ch<<i;		// this time we have big enough buffer and there's no dead lock.
					// when passing more compilcated classes `ch<<a`, if you want to use `a`
					// after that, use `ch<<std::cref(a)`. The logic of this implementation
					// bascally requires std::cref() for not-move-constructing.
					// move-constrcut of `int` don't delete the orignal data, so `int` is fine here
	
	ch.end()		// not necessary, but a good approach to tell downstream when is the end

	int i;
	while(ch>>i)	// returns false when buffer is empty and it's `end`ed
					// sleeps if it's not `end`ed. As a result `end` is  good practice.
		...

	// following are for pipeline helper class

	pipe | func1<...> | func2<...> | func3<...>

	// sigiture of `func1<...>` should be `void (*) (chan<int,0>&, chan<something>&)
	// here chan<int,0> is a standard notation for "nothing", as `pipe` is just a helper
	// which does nothing, `func1` don't accept any data.

	// things in <...> are just templates used to specify other arguments like "grep -v" or
	// "ls -a". It's not necessary

	// second argument type of `func1` should match first argument type of `func2`, so does below

	// due to C++ destructing logic, all threads will be joined and all tmp channels will be destryed
	// after this chain of `... | ... | ...`. Just like the `... | ... | ...` is a single function and
	// the program will not go down until it finishes.
```
