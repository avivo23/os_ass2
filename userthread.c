#include "types.h"
#include "stat.h"
#include "user.h"

/* Possible states of a thread; */
#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2

#define STACK_SIZE  4096
#define MAX_UTHREADS 64
#define UTHREAD_QUANTA 5

typedef struct thread thread_t, *thread_p;

struct thread {
  int        sp;                /* curent stack pointer */
  char stack[STACK_SIZE];       /* the thread's stack */
  int        state;             /* running, runnable, waiting */
  int id;
};
static thread_t all_thread[MAX_UTHREADS];
static thread_p  current_thread;
static thread_p  next_thread;
 

int uthread_self()
{
    return current_thread->id;
}


void uthread_schedule()
{

  thread_p t;
  current_thread->state = RUNNABLE;


  /* Find another runnable thread. */
  for (t = all_thread; t < all_thread + MAX_UTHREADS; t++) {
    if (t->state == RUNNABLE && t != current_thread) {
      next_thread = t;
      break;
    }
  }

  if (t >= all_thread + MAX_UTHREADS && current_thread->state == RUNNABLE) {
    /* The current thread is the only runnable thread; run it. */
    next_thread = current_thread;
    printf(2, "keep running thread %d\n", current_thread->id);
  }

  if (next_thread == 0) {
    printf(2, "thread_schedule: no runnable threads; deadlock\n");
    exit();
  }

  
  if (current_thread != next_thread ) {         /* switch threads?  */
      next_thread->state = RUNNING;
    
      printf(2, "current thread: %d, next thread: %d, current thread sp: %d, next thread sp: %d\n", current_thread->id, next_thread->id, current_thread->sp, next_thread->sp);
    
        asm("movl current_thread, %eax\n\t"
        "movl %esp, (%eax)\n\t"
        "movl next_thread, %eax\n\t"
        "movl %eax, current_thread\n\t"
        "movl $0, next_thread\n\t"
        "movl current_thread, %eax\n\t"
        "movl (%eax), %esp\n\t");
            
    
  } else
    next_thread = 0;
  
  signal(14, uthread_schedule);
  alarm(UTHREAD_QUANTA);

}

int uthread_init()
{
  current_thread = &all_thread[0];
  current_thread->state = RUNNING;
  signal(14, uthread_schedule);
  alarm(UTHREAD_QUANTA);

  return 1;
}

void 
uthread_exit()
{
  current_thread->state = FREE;
  uthread_schedule();
}

void wrapStartFunc(void (*start_func)(void *), void*arg)
{
 start_func(arg);
 uthread_exit();
}


int uthread_create(void (*start_func)(void *), void*arg)
{
  thread_p t;
  int index = 0;
  for (t = all_thread; t < all_thread + MAX_UTHREADS; t++) {
    if (t->state == FREE) break;
    index++;
  }
  if(index > 0 && index < MAX_UTHREADS)
  {
      printf(2, "address is: %d\n",t);
    t->sp = (int) (t->stack + STACK_SIZE);   // set sp to the top of the stack
    t->sp -= 4;                
     * (int *) (t->sp) = (int)start_func;            // space for return address
    t->sp -= 4;                
     * (int *) (t->sp)= (int)arg;
    t->sp -= 4;                
    * (int *) (t->sp) = (int)wrapStartFunc;           // push return address on stack
    t->sp -= 32;                             // space for registers that thread_switch will push
    t->state = RUNNABLE;
    t->id = index;
    return index;
  }
  
  return -1;
}




int uthread_join(int tid)
{
  thread_p t;
  for (t = all_thread; t < all_thread + MAX_UTHREADS; t++) {
    if (t->id == tid)
    {
        while(!t->state == FREE) { }
        break;
    }
  }
  
  return 0;
}

int uthread_sleep(int ticks)
{
    int i = ticks;
    while(--i) { }
    return 0;
}


void mythread(void* arg)
{
  int i;
  printf(1, "thread %d: running\n", uthread_self());
  for (i = 0; i < 100; i++) {
    printf(1, "thread %d says hello\n", uthread_self());
  }
  printf(1, "thread %d: exit\n", uthread_self());
  current_thread->state = FREE;
}


int 
main(int argc, char *argv[]) 
{
  int* nothing = 0;
  uthread_init();
  uthread_create(mythread, nothing);
  sleep(5);
  sleep(5);
  sleep(5);
  /*uthread_create(mythread, nothing);
  */
  exit();
  return 1;
}