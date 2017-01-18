This is c++ implementation of thread-safe channel (interthread pipeline). `channel` is single-buffered implementation which involks a lot of mutex, while `bufchannel` is triple-buffered implementation which do not involk any mutex in most cases, and can hold three times more (in maximum). Usage:
```
channel<type,optionalBufferSize> ch;

#in producer thread
for ( ... )
	ch << ... ;
ch.end();			// "end" must be called otherwise consumer would not know when to stop

#in consumer thread
	ch >> ... ;
```