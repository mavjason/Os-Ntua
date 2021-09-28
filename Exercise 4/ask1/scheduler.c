#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include <sys/wait.h>
#include <sys/types.h>

#include "proc-common.h"
#include "request.h"

/* Compile-time parameters. */
#define SCHED_TQ_SEC 2                /* time quantum */
#define TASK_NAME_SZ 60               /* maximum size for a task's name */

int * process;
int * active;
int nproc, current_proc;


/*
 * SIGALRM handler
 */
static void
sigalrm_handler(int signum)
{
	kill(process[current_proc], SIGSTOP);       //your time is up, stop
}

/* 
 * SIGCHLD handler
 */
static void
sigchld_handler(int signum)
{
    pid_t p;
	int i, status;

	for(;;) {

  		p = waitpid(-1, &status, WUNTRACED | WNOHANG);      //wait for the process that was lastly signaled or stopped

    	if(p == 0) break;   //if you can't find any, break

		process[current_proc] = (int) p;

		if (WIFEXITED(status) || WIFSIGNALED(status)) {                 //termination OR unhandled signal
			printf("        The child process %d is terminated.\n", process[current_proc]);
			active[current_proc] = 0;
		}

		if (WIFSTOPPED(status)) printf("        The child process %d is stopped.\n", process[current_proc]);        //SIGSTOP


		for (i=0; i<nproc; i++) if (active[i] == 1) break;      //if you find active process stop
		if (i == nproc) {
			printf("        All child processes were terminated.\n");
			exit(0);
		}	
	
		do {
			current_proc = ((current_proc + 1) % nproc);     //find next active process
		} while (active[current_proc] == 0);
		
		printf("        The child process %d is continuing.\n", process[current_proc]);
		
		alarm(SCHED_TQ_SEC);
		kill(process[current_proc], SIGCONT); 
	}
}

/* Install two signal handlers.
 * One for SIGCHLD, one for SIGALRM.
 * Make sure both signals are masked when one of them is running.
 */
static void
install_signal_handlers(void)
{
	sigset_t sigset;
	struct sigaction sa;

	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGCHLD);
	sigaddset(&sigset, SIGALRM);
	sa.sa_mask = sigset;
	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		perror("sigaction: sigchld");
		exit(1);
	}

	sa.sa_handler = sigalrm_handler;
	if (sigaction(SIGALRM, &sa, NULL) < 0) {
		perror("sigaction: sigalrm");
		exit(1);
	}

	/*
	 * Ignore SIGPIPE, so that write()s to pipes
	 * with no reader do not result in us being killed,
	 * and write() returns EPIPE instead.
	 */
	if (signal(SIGPIPE, SIG_IGN) < 0) {
		perror("signal: sigpipe");
		exit(1);
	}
}

int child(char *executable)
{
	char *newargv[] = { executable, NULL, NULL, NULL };
	char *newenviron[] = { NULL };

    raise(SIGSTOP);         //don't start unless said so

	execve(executable, newargv, newenviron);

	/* execve() only returns on error */
	perror("execve");
	exit(1);
}

int main(int argc, char *argv[])
{
	int i;
    pid_t p;    

	/*
	 * For each of argv[1] to argv[argc - 1],
	 * create a new child process, add it to the process list.
	 */

	nproc = argc-1;         /* number of proccesses goes here */
    current_proc = 0;       //random proc_no until we have a process running
    process = malloc(sizeof(int)*nproc);
    active = malloc(sizeof(int)*nproc);
    //memset(active, 1, nproc);

    for (i=0; i<nproc; i++) {
        p = fork();
        if (p < 0) {perror("fork"); exit(1);}
	    else if (p == 0) {
		    child(argv[i+1]);
            exit(1);                    //will never be executed, just for safety reasons ;)
	    }
        else {
            process[i] = (int) p;       //add to process list
            active[i] = 1;              //the process is active
        }
    }

	/* Wait for all children to raise SIGSTOP before exec()ing. */
	wait_for_ready_children(nproc);

	/* Install SIGALRM and SIGCHLD handlers. */
	install_signal_handlers();

    if (nproc == 0) {
		fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
		exit(1);
	}

    alarm(SCHED_TQ_SEC);	
	kill(process[0], SIGCONT);      //start with process 0.

	/* loop forever  until we exit from inside a signal handler. */
	while (pause())
		;

	/* Unreachable */
	fprintf(stderr, "Internal error: Reached unreachable point\n");
	return 1;
}
