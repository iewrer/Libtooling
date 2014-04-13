//==========================================================================
//
//      sched/mlqueue.cxx
//
//      Multi-level queue scheduler class implementation
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg
// Contributors: jlarmour
// Date:         1999-02-17
// Purpose:      Multilevel queue scheduler class implementation
// Description:  This file contains the implementations of
//               Cyg_Scheduler_Implementation and
//               Cyg_SchedThread_Implementation.
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/kernel.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <cyg/kernel/sched.hxx>        // our header

#include <cyg/hal/hal_arch.h>          // Architecture specific definitions

#include <cyg/kernel/thread.inl>       // thread inlines
#include <cyg/kernel/sched.inl>        // scheduler inlines

//deleted

//==========================================================================
// Cyg_Scheduler_Implementation class static members

//deleted

cyg_ucount32 Cyg_Scheduler_Implementation::timeslice_count[CYGNUM_KERNEL_CPU_MAX];

//deleted


//==========================================================================
// Cyg_Scheduler_Implementation class members

// -------------------------------------------------------------------------
// Constructor.

Cyg_Scheduler_Implementation::Cyg_Scheduler_Implementation()
{
    CYG_REPORT_FUNCTION();
        
    queue_map   = 0;

//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
    
    for( int i = 0; i < CYGNUM_KERNEL_CPU_MAX; i++ )
    {
//deleted
        timeslice_count[i] = CYGNUM_KERNEL_SCHED_TIMESLICE_TICKS;
//deleted
        need_reschedule[i] = true;
    }
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Choose the best thread to run next

Cyg_Thread *
Cyg_Scheduler_Implementation::schedule(void)
{
    CYG_REPORT_FUNCTYPE("returning thread %08x");

    // The run queue may _never_ be empty, there is always
    // an idle thread at the lowest priority.

    CYG_ASSERT( queue_map != 0, "Run queue empty");
    CYG_ASSERT( queue_map & (1<<CYG_THREAD_MIN_PRIORITY), "Idle thread vanished!!!");
    CYG_ASSERT( !run_queue[CYG_THREAD_MIN_PRIORITY].empty(), "Idle thread vanished!!!");

//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted

    register cyg_uint32 index;

    HAL_LSBIT_INDEX(index, queue_map);

    Cyg_RunQueue *queue = &run_queue[index];
    
    CYG_ASSERT( !queue->empty(), "Queue for index empty");

    Cyg_Thread *thread = queue->get_head();

//deleted
    
    CYG_INSTRUMENT_MLQ( SCHEDULE, thread, index);
    
    CYG_ASSERT( thread != NULL , "No threads in run queue");
    CYG_ASSERT( thread->queue == NULL , "Runnable thread on a queue!");
   
    CYG_REPORT_RETVAL(thread);

    return thread;
}

// -------------------------------------------------------------------------

void
Cyg_Scheduler_Implementation::add_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("thread=%08x", thread);

    cyg_priority pri                               = thread->priority;
    Cyg_RunQueue *queue = &run_queue[pri];

    CYG_INSTRUMENT_MLQ( ADD, thread, pri);
    
    CYG_ASSERT((CYG_THREAD_MIN_PRIORITY >= pri) 
               && (CYG_THREAD_MAX_PRIORITY <= pri),
               "Priority out of range!");

    CYG_ASSERT( ((queue_map & (1<<pri))!=0) == ((!run_queue[pri].empty())!=0), "Map and queue disagree");

    // If the thread is on some other queue, remove it
    // here.
    if( thread->queue != NULL )
    {
        thread->queue->remove(thread);
    }
    
    if( queue->empty() )
    {
        // set the map bit and ask for a reschedule if this is a
        // new highest priority thread.
      
        queue_map |= (1<<pri);

    }
    // else the queue already has an occupant, queue behind him

    queue->add_tail(thread);

    // If the new thread is higher priority than any
    // current thread, request a reschedule.

    set_need_reschedule(thread);

    // Also reset the timeslice_count so that this thread gets a full
    // timeslice once it begins to run.
    
    thread->timeslice_reset();
    
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
    
    CYG_ASSERT( thread->queue == NULL , "Runnable thread on a queue!");
    CYG_ASSERT( queue_map != 0, "Run queue empty");
    CYG_ASSERT( queue_map & (1<<pri), "Queue map bit not set for pri");
    CYG_ASSERT( !run_queue[pri].empty(), "Queue for pri empty");
    CYG_ASSERT( ((queue_map & (1<<pri))!=0) == ((!run_queue[pri].empty())!=0), "Map and queue disagree");    
    CYG_ASSERT( queue_map & (1<<CYG_THREAD_MIN_PRIORITY), "Idle thread vanished!!!");
    CYG_ASSERT( !run_queue[CYG_THREAD_MIN_PRIORITY].empty(), "Idle thread vanished!!!");
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------

void
Cyg_Scheduler_Implementation::rem_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("thread=%08x", thread);
        
    CYG_ASSERT( queue_map != 0, "Run queue empty");

    cyg_priority pri    = thread->priority;
    Cyg_RunQueue *queue = &run_queue[pri];

    CYG_INSTRUMENT_MLQ( REM, thread, pri);
    
    CYG_ASSERT( pri != CYG_THREAD_MIN_PRIORITY, "Idle thread trying to sleep!");
    CYG_ASSERT( !run_queue[CYG_THREAD_MIN_PRIORITY].empty(), "Idle thread vanished!!!");

//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
        
    CYG_ASSERT( queue_map & (1<<pri), "Queue map bit not set for pri");
    CYG_ASSERT( !run_queue[pri].empty(), "Queue for pri empty");
    
    // remove thread from queue
    queue->remove(thread);

    if( queue->empty() )
    {
        // If this was only thread in
        // queue, clear map.
      
        queue_map &= ~(1<<pri);
    }

    CYG_ASSERT( queue_map != 0, "Run queue empty");
    CYG_ASSERT( queue_map & (1<<CYG_THREAD_MIN_PRIORITY), "Idle thread vanished!!!");
    CYG_ASSERT( !run_queue[CYG_THREAD_MIN_PRIORITY].empty(), "Idle thread vanished!!!");
    CYG_ASSERT( ((queue_map & (1<<pri))!=0) == ((!run_queue[pri].empty())!=0), "Map and queue disagree");
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Set the need_reschedule flag
// This function overrides the definition in Cyg_Scheduler_Base and tests
// for a reschedule condition based on the priorities of the given thread
// and the current thread(s).

void Cyg_Scheduler_Implementation::set_need_reschedule(Cyg_Thread *thread)
{
//deleted

    if( current_thread[0]->priority > thread->priority ||
        current_thread[0]->get_state() != Cyg_Thread::RUNNING )
        need_reschedule[0] = true;
    
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
}

// -------------------------------------------------------------------------
// Set up initial idle thread

void Cyg_Scheduler_Implementation::set_idle_thread( Cyg_Thread *thread, HAL_SMP_CPU_TYPE cpu )
{
    // Make the thread the current thread for this CPU.
    
    current_thread[cpu] = thread;

    // This will insert the thread in the run queues and make it
    // available to execute.
    thread->resume();

//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
    
}

// -------------------------------------------------------------------------
// register thread with scheduler

void
Cyg_Scheduler_Implementation::register_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("thread=%08x", thread);
    // No registration necessary in this scheduler
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------

// deregister thread
void
Cyg_Scheduler_Implementation::deregister_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("thread=%08x", thread);
    // No registration necessary in this scheduler    
    CYG_REPORT_RETURN();
}
    
// -------------------------------------------------------------------------
// Test the given priority for uniqueness

cyg_bool
Cyg_Scheduler_Implementation::unique( cyg_priority priority)
{
    CYG_REPORT_FUNCTYPE("returning %d");
    CYG_REPORT_FUNCARG1("priority=%d", priority);
    // Priorities are not unique
    CYG_REPORT_RETVAL(true);
    return true;
}

//==========================================================================
// Support for timeslicing option

//deleted

// -------------------------------------------------------------------------

void
Cyg_Scheduler_Implementation::timeslice(void)
{
//deleted
//deleted
//deleted

//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted

    if( --timeslice_count[CYG_KERNEL_CPU_THIS()] == 0 )
        timeslice_cpu();
    
//deleted

//deleted
//deleted
//deleted
}

// -------------------------------------------------------------------------

void
Cyg_Scheduler_Implementation::timeslice_cpu(void)
{
//deleted
//deleted
//deleted

    Cyg_Thread *thread = get_current_thread();
    HAL_SMP_CPU_TYPE cpu_this = CYG_KERNEL_CPU_THIS();
    
    CYG_ASSERT( queue_map != 0, "Run queue empty");
    CYG_ASSERT( queue_map & (1<<CYG_THREAD_MIN_PRIORITY), "Idle thread vanished!!!");

//deleted
//deleted
//deleted
//deleted
    if( timeslice_count[cpu_this] == 0 )
//deleted
    {
        CYG_INSTRUMENT_SCHED(TIMESLICE,0,0);
//deleted
//deleted
//deleted

        CYG_ASSERT( get_sched_lock() > 0 , "Timeslice called with zero sched_lock");

        // Only try to rotate the run queue if the current thread is running.
        // Otherwise we are going to reschedule anyway.
        if( thread->get_state() == Cyg_Thread::RUNNING )
        {
            Cyg_Scheduler *sched = &Cyg_Scheduler::scheduler;

            CYG_INSTRUMENT_MLQ( TIMESLICE, thread, 0);
                
            CYG_ASSERTCLASS( thread, "Bad current thread");
            CYG_ASSERTCLASS( sched, "Bad scheduler");
    
            cyg_priority pri    = thread->priority;
            Cyg_RunQueue *queue = &sched->run_queue[pri];

//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
            queue->rotate();
//deleted
            
            if( queue->get_head() != thread )
                sched->set_need_reschedule();

            timeslice_count[cpu_this] = CYGNUM_KERNEL_SCHED_TIMESLICE_TICKS;
        }
    }

    
    CYG_ASSERT( queue_map & (1<<CYG_THREAD_MIN_PRIORITY), "Idle thread vanished!!!");
    CYG_ASSERT( !run_queue[CYG_THREAD_MIN_PRIORITY].empty(), "Idle thread vanished!!!");
//deleted
//deleted
//deleted
}

// -------------------------------------------------------------------------

__externC void cyg_scheduler_timeslice_cpu(void)
{
    Cyg_Scheduler::scheduler.timeslice_cpu();
}

//deleted

//==========================================================================
// Cyg_SchedThread_Implementation class members

Cyg_SchedThread_Implementation::Cyg_SchedThread_Implementation
(
    CYG_ADDRWORD sched_info
)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("sched_info=%08x", sched_info);
        
    // Set priority to the supplied value.
    priority = (cyg_priority)sched_info;

//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Yield the processor to another thread

void
Cyg_SchedThread_Implementation::yield(void)
{
    CYG_REPORT_FUNCTION();
        
    // Prevent preemption
    Cyg_Scheduler::lock();

    Cyg_Thread *thread  = CYG_CLASSFROMBASE(Cyg_Thread,
                                            Cyg_SchedThread_Implementation,
                                            this);

    // Only do this if this thread is running. If it is not, there
    // is no point.
    
    if( thread->get_state() == Cyg_Thread::RUNNING )
    {
        // To yield we simply rotate the appropriate
        // run queue to the next thread and reschedule.

        CYG_INSTRUMENT_MLQ( YIELD, thread, 0);
        
        CYG_ASSERTCLASS( thread, "Bad current thread");
    
        Cyg_Scheduler *sched = &Cyg_Scheduler::scheduler;

        CYG_ASSERTCLASS( sched, "Bad scheduler");
    
        cyg_priority pri    = thread->priority;
        Cyg_RunQueue *queue = &sched->run_queue[pri];

//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
            queue->rotate();
//deleted
        
        if( queue->get_head() != thread )
            sched->set_need_reschedule();
        else
        {
            // Reset the timeslice counter so that this thread gets a
            // full quantum as a reward for yielding when it is
            // eventually rescheduled.
            thread->timeslice_reset();
        }

    }
    
    // Unlock the scheduler and switch threads
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
    Cyg_Scheduler::unlock_reschedule();

    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Rotate the run queue at a specified priority.
// (pri is the decider, not this, so the routine is static)

void
Cyg_SchedThread_Implementation::rotate_queue( cyg_priority pri )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("priority=%d", pri);
        
    // Prevent preemption
    Cyg_Scheduler::lock();

    Cyg_Scheduler *sched = &Cyg_Scheduler::scheduler;

    CYG_ASSERTCLASS( sched, "Bad scheduler");
    
    Cyg_RunQueue *queue = &sched->run_queue[pri];

    if ( !queue->empty() ) {
        queue->rotate();
        sched->set_need_reschedule();
    }

    // Unlock the scheduler and switch threads
    Cyg_Scheduler::unlock();

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Move this thread to the head of its queue
// (not necessarily a scheduler queue)

void
Cyg_SchedThread_Implementation::to_queue_head( void )
{
    CYG_REPORT_FUNCTION();
        
    // Prevent preemption
    Cyg_Scheduler::lock();

    Cyg_Thread *thread  = CYG_CLASSFROMBASE(Cyg_Thread,
                                            Cyg_SchedThread_Implementation,
                                            this);

    CYG_ASSERTCLASS( thread, "Bad current thread");
    
    Cyg_ThreadQueue *q = thread->get_current_queue();
    if( q != NULL )
        q->to_head( thread );
    else if( thread->in_list() )
    {
        // If the queue pointer is NULL then it is on a run
        // queue. Move the thread to the head of it's priority list
        // and force a reschedule.
        
        Cyg_Scheduler *sched = &Cyg_Scheduler::scheduler;
        sched->run_queue[thread->priority].to_head( thread );
        sched->set_need_reschedule( thread );
    }

    // Unlock the scheduler and switch threads
    Cyg_Scheduler::unlock();

    CYG_REPORT_RETURN();
}

//==========================================================================
// Cyg_ThreadQueue_Implementation class members

// -------------------------------------------------------------------------        

void
Cyg_ThreadQueue_Implementation::enqueue(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("thread=%08x", thread);

    CYG_INSTRUMENT_MLQ( ENQUEUE, this, thread );
    
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
//deleted
    // Just add the thread to the tail of the list
    add_tail( thread );
//deleted
    
    thread->queue = CYG_CLASSFROMBASE(Cyg_ThreadQueue,
                                      Cyg_ThreadQueue_Implementation,
                                      this);
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------

Cyg_Thread *
Cyg_ThreadQueue_Implementation::dequeue(void)
{
    CYG_REPORT_FUNCTYPE("returning thread %08x");
        
    Cyg_Thread *thread = rem_head();

    CYG_INSTRUMENT_MLQ( DEQUEUE, this, thread );
    
    if( thread != NULL )
        thread->queue = NULL;

    CYG_REPORT_RETVAL(thread);
    return thread;
}

// -------------------------------------------------------------------------

void
Cyg_ThreadQueue_Implementation::remove( Cyg_Thread *thread )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("thread=%08x", thread);

    CYG_INSTRUMENT_MLQ( REMOVE, this, thread );
    
    thread->queue = NULL;

    Cyg_CList_T<Cyg_Thread>::remove( thread );

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------

Cyg_Thread *
Cyg_ThreadQueue_Implementation::highpri(void)
{
    CYG_REPORT_FUNCTYPE("returning thread %08x");
    CYG_REPORT_RETVAL(get_head());
    return get_head();
}

// -------------------------------------------------------------------------

inline void
Cyg_ThreadQueue_Implementation::set_thread_queue(Cyg_Thread *thread,
                                                 Cyg_ThreadQueue *tq )

{
    thread->queue = tq;
}

// -------------------------------------------------------------------------

//deleted

// -------------------------------------------------------------------------
// EOF sched/mlqueue.cxx
