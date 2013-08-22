/* 
    File: scheduler.C

    Author: Shyam S Ramachandran
    Date  : 10/25/2012



*/

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "utils.H"
#include "scheduler.H"
#include "console.H"
#include "frame_pool.H"

/*--------------------------------------------------------------------------*/
/* LOCAL VARIABLES */
/*--------------------------------------------------------------------------*/


Scheduler::Scheduler() {

	/* initialize the ready queue */
	for(int i=0;i<MAX_THREADS;i++) {
		readyq[i]=NULL;
	}

	/*initialize queue head to 0 and number of threads in the system to 0 */
	q_head = 0;
	thread_count = 0;

}

/*

yield() operation involves dequeueing the readyq and run the dequeued thread 

*/
void Scheduler::yield() {
	/* dequeue the first element */
	Thread* next = (Thread*) readyq[q_head];
	readyq[q_head] = NULL;
	q_head++;
	if(q_head >= MAX_THREADS) {
		q_head=0;
	}

	/* disable interrupts because diispatch_to is not re-enterant! */
        if(Machine::interrupts_enabled()) {
        	Machine::disable_interrupts();
        }

	/* Now dispatch the dequeued Thread to run */
	Thread::dispatch_to(next);

	/* we will return with the context of the new thread here: re-enable interrupts */
        if(!Machine::interrupts_enabled()) {
        	Machine::enable_interrupts();
        }

}

/*
Simply Add the thread pointed to by _thread to the ready queue tail
*/
void Scheduler::resume(Thread * _thread) {
	unsigned int next = q_head + thread_count;
	if(next >= MAX_THREADS) {
	// if we reach out of array, then start from first
		next = next - MAX_THREADS;
	}
	if(readyq[next]!=NULL) {
                Console::puts("The ready queue is full: inside resume");
		for(;;);
        } else {
                readyq[next] = (unsigned long*)_thread;
      		
        }

	
	return;	

}
/*

It is also similar to resume(), but check for some conditions isuch as max_threads

*/
void Scheduler::add(Thread * _thread) {

	/* If the limit for number for threads for the system has
	been reached then, just return. This thread will not be scheduled
	at all */
	if(thread_count>=MAX_THREADS) {
		return;
	}

	/* now add this thread to the ready queue */
	unsigned int next = q_head + thread_count;
        if(next >= MAX_THREADS) {
        // if we reach out of array, then start from first
                next = next - MAX_THREADS;
        }
        if(readyq[next]!=NULL) {
                Console::puts("The ready queue is full: inside add");
		for(;;);
        } else {
                readyq[next] = (unsigned long*)_thread;
                
        } 
	thread_count++;

}
/*

Called from Thread::thread_shutdown() 

*/
void Scheduler::terminate(Thread * _thread) {
	/* give back the resources of this thread */
	delete _thread;
	thread_count--;
	return;
}

