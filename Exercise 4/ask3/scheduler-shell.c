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
#define SHELL_EXECUTABLE_NAME "shell" /* executable for shell */

typedef struct process_info 
{
	pid_t PID;
	int id;
	char name[TASK_NAME_SZ];
	char priority;
	struct process_info * next;
} proc;

static int nproc; 
static proc * extra_proc;
static proc * current_proc;

/* Insert a process to the process list. */
static void 
ins_proc(int id, pid_t p, char * name)	
{
    proc * extra_proc = (struct process_info *) malloc(sizeof(struct process_info));
	
	extra_proc->id = id;	
	extra_proc->PID = p;
	strcpy(extra_proc->name,name);
	extra_proc->priority = 'l';
	
	extra_proc->next = current_proc->next;
	current_proc->next = extra_proc;	
	current_proc = extra_proc;
}

/* Delete a process from the process list. */
static void 
del_proc(int id)
{	
	extra_proc = current_proc;

	while (extra_proc->next->id != id) extra_proc = extra_proc->next;

	extra_proc->next = extra_proc->next->next;
}

static void
child(char * executable)
{
	char * newargv[] = {executable, NULL, NULL, NULL};
	char * newenviron[] = {NULL};

	raise(SIGSTOP);         //don't start unless said so
	
	execve(executable, newargv, newenviron);
	
	/* execve() only returns on error */
	perror("execve");
	exit(1);
}

/* Print a list of all tasks currently being scheduled. */
static void
sched_print_tasks(void)
{
	extra_proc = current_proc;
	
	char * prior="";
	if(extra_proc->priority == 'h') prior = "high";
	else prior = "low";
	
	printf("		Current process: PID: %d, id: %d, name: %s, priority: %s\n", extra_proc->PID, extra_proc->id, extra_proc->name, prior);
	
	while(extra_proc->next != current_proc){
		extra_proc = extra_proc->next;
		if(extra_proc->priority =='h') prior = "high";
		else prior = "low";
		
		printf("		Process: PID: %d, id: %d, name: %s, priority: %s\n", extra_proc->PID, extra_proc->id, extra_proc->name, prior);
	}
}

/* Send SIGKILL to a task determined by the value of its
 * scheduler-specific id.
 */
static int
sched_kill_task_by_id(int id)
{
	extra_proc = current_proc;

	while (extra_proc->id != id){	
		extra_proc = extra_proc->next;

		if(extra_proc == current_proc) return -1;
	}

	printf("		The child process %d with id=%d was terminated.\n", extra_proc->PID, id);
	
	kill(extra_proc->PID, SIGKILL);
	return id;
}

/* Create a new task. */
static void
sched_create_task(char *executable)
{
	nproc++;

	pid_t p = fork();	
	if (p < 0) {perror("fork"); exit(1);}
	else if (p == 0){
		child(executable);
	}
	else{
		printf("        The child process %d with id=%d is created.\n", p, nproc-1);
		ins_proc(nproc-1, p, executable);
	}
}

static int
sched_high_priority(int id)
{
	extra_proc = current_proc;

	while (extra_proc->id != id){
		extra_proc = extra_proc->next;
		if(extra_proc == current_proc) return -1;
	}
	
	extra_proc->priority = 'h';
	printf("        The child process %d with id=%d now has high priority.\n", extra_proc->PID, extra_proc->id);
	
	return(extra_proc->id);
}

static int
sched_low_priority(int id)
{
	extra_proc = current_proc;	
	while (extra_proc->id != id){
		extra_proc = extra_proc->next;
		if(extra_proc == current_proc) return -1;
	}
	
	extra_proc->priority = 'l';
	printf("        The child process %d with id=%d now has low priority.\n", extra_proc->PID, extra_proc->id);
	
	return(extra_proc->id);
} 

/* Process requests by the shell. */
static int
process_request(struct request_struct *rq)
{
	switch (rq->request_no) {
		case REQ_PRINT_TASKS:
			sched_print_tasks();
			return 0;

		case REQ_KILL_TASK:
			return sched_kill_task_by_id(rq->task_arg);

		case REQ_EXEC_TASK:
			sched_create_task(rq->exec_task_arg);
			return 0;

		case REQ_HIGH_TASK:
			return sched_high_priority(rq->task_arg);

		case REQ_LOW_TASK:
			return sched_low_priority(rq->task_arg); 

		default:
			return -ENOSYS;
	}
}

/* 
 * SIGALRM handler
 */
static void
sigalrm_handler(int signum)
{
	kill(current_proc->PID, SIGSTOP);	   //your time is up, stop
}

/* 
 * SIGCHLD handler
 */
static void
sigchld_handler(int signum)
{
	pid_t p;
	int status;
	
	for(;;){
		p = waitpid(-1, &status, WUNTRACED | WNOHANG);
		
		if(p == 0) break;
		
		while(current_proc->PID != p) current_proc = current_proc->next;

		if (WIFEXITED(status)  || WIFSIGNALED(status)) {
			printf("        The child process %d with id=%d is terminated.\n", current_proc->PID, current_proc->id);
			
			if(current_proc->next == current_proc){
				printf("        All child processes were terminated.\n");
				exit(0);			
			}
			
			del_proc(current_proc->id);	
			current_proc=current_proc->next;
		}
		
		if (WIFSTOPPED(status)){
			printf("        The child process %d with id=%d is stopped.\n",  current_proc->PID, current_proc->id);        //SIGSTOP
		}
		
		extra_proc = current_proc->next;
		while(extra_proc->priority != 'h'){
			extra_proc = extra_proc->next;
			if(extra_proc == current_proc){
				if(current_proc->priority == 'h') break;
				else {
					extra_proc = extra_proc->next;
					break;
				}
			}
		}
		
		current_proc = extra_proc;
		
		char * prior;
		if(current_proc->priority == 'l') prior = "low";
		else prior = "high";
		
		printf("        The child process %d with id=%d and priority=%s is continuing.\n", current_proc->PID, current_proc->id, prior);
		
		alarm(SCHED_TQ_SEC);	
		kill(current_proc->PID, SIGCONT);
	}
}

/* Disable delivery of SIGALRM and SIGCHLD. */
static void
signals_disable(void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGALRM);
	sigaddset(&sigset, SIGCHLD);
	if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0) {
		perror("signals_disable: sigprocmask");
		exit(1);
	}
}

/* Enable delivery of SIGALRM and SIGCHLD.  */
static void
signals_enable(void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGALRM);
	sigaddset(&sigset, SIGCHLD);
	if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0) {
		perror("signals_enable: sigprocmask");
		exit(1);
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

static void
do_shell(char *executable, int wfd, int rfd){
	char arg1[10], arg2[10];
	char *newargv[] = { executable, NULL, NULL, NULL };
	char *newenviron[] = { NULL };

	sprintf(arg1, "%05d", wfd);
	sprintf(arg2, "%05d", rfd);
	newargv[1] = arg1;
	newargv[2] = arg2;

	raise(SIGSTOP);
	execve(executable, newargv, newenviron);

	/* execve() only returns on error */
	perror("scheduler: child: execve");
	exit(1);
}

/* Create a new shell task.
 *
 * The shell gets special treatment:
 * two pipes are created for communication and passed
 * as command-line arguments to the executable.
 */
static void
sched_create_shell(char *executable, int *request_fd, int *return_fd)
{
	pid_t p;
	int pfds_rq[2], pfds_ret[2];

	if (pipe(pfds_rq) < 0 || pipe(pfds_ret) < 0) {
		perror("pipe");
		exit(1);
	}

	p = fork();
	if (p < 0) {
		perror("scheduler: fork");
		exit(1);
	}

	if (p == 0) {
		/* Child */
		close(pfds_rq[0]);
		close(pfds_ret[1]);
		do_shell(executable, pfds_rq[1], pfds_ret[0]);
		assert(0);
	}
	/* Parent */
	current_proc = (struct process_info *) malloc(sizeof(struct process_info));
	current_proc->PID = p;	
	strcpy(current_proc->name, SHELL_EXECUTABLE_NAME);
	current_proc->priority = 'l';
	current_proc->next = current_proc;	

	close(pfds_rq[1]);
	close(pfds_ret[0]);
	*request_fd = pfds_rq[0];
	*return_fd = pfds_ret[1];
}

static void
shell_request_loop(int request_fd, int return_fd)
{
	int ret;
	struct request_struct rq;

	/*
	 * Keep receiving requests from the shell.
	 */
	for (;;) {
		if (read(request_fd, &rq, sizeof(rq)) != sizeof(rq)) {
			perror("scheduler: read from shell");
			fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
			break;
		}
		
		signals_disable();
		ret = process_request(&rq);
		signals_enable();
		
		if (write(return_fd, &ret, sizeof(ret)) != sizeof(ret)) {
			perror("scheduler: write to shell");
			fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	/* Two file descriptors for communication with the shell */
	static int request_fd, return_fd;

	/* Create the shell. */
	sched_create_shell(SHELL_EXECUTABLE_NAME, &request_fd, &return_fd);
	/* TODO: add the shell to the scheduler's tasks */

	/*
	 * For each of argv[1] to argv[argc - 1],
	 * create a new child process, add it to the process list.
	 */

	pid_t p;
	int i;
	
	nproc = argc; 
	 
	for (i=1; i < nproc; i++){
		p = fork();
		if (p < 0) {perror("fork"); exit(1);}
		else if (p == 0){
			child(argv[i]);	
		}
		else{
			ins_proc(i, p, argv[i]);
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
	kill(current_proc->PID, SIGCONT);
	
	shell_request_loop(request_fd, return_fd);

	/* Now that the shell is gone, just loop forever
	 * until we exit from inside a signal handler.
	 */
	while (pause())
		;

	/* Unreachable */
	fprintf(stderr, "Internal error: Reached unreachable point\n");
	return 1;
}
