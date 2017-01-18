This is implementation of C++ pipe/channel. It provides 3 classes and 1 instance:

```
// "chan" is single buffer thread-safe channel which involves a lot of mutex operations
chan <type,optionalBufferSize> ch;
// in producer thread
for ( ... )
	ch << ...;
ch.end()			// "end" must be called or consumer would not know when to stop
// in consumer thread
while (ch >> ...)
	...;
// both trying to read when buffer is empty or trying to write when buffer is full will pause that thread until data is ready/removed, transparent to thread


// "bufchan" is trible buffer thread-safe channel which do not involve any mutex operation in most cases
// everything else same as above

// "pipE" is OO interface for easier multithreading, use the pipe operator '|', just like shell. Operations will be automatically multithread and joined.
pipE<chanType> p;	// "chanType" can either be chan<...>, bufchan<...> or your class. How it's implemented is abstract to "pipE". Following example uses "chan"
pipE<chan<void*>> | rand<1000> | plus<2> | mut<3> | print ;	// my implementation of "pipable" functions ("rand", "plus", "mut", and "print" here) uses "void*" 
															// to indicate "nothing". It's not forced
void pipableFunction(chan<...>& in,chan<...>& out)
	...

// "pipe" is recommended "starting" of "pipE". "bufchan" is recommended since it should act faster than chan. Use "chan" only if there are low memory space.
pipE<bufchan<void*>> pipe;