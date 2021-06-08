/*--------------------------------------------------------
 UNIX Shell Project
 function prototypes, macros and type declarations for job_control module

 Sistemas Operativos
 Grados I. Informatica, Computadores & Software
 Dept. Arquitectura de Computadores - UMA

 Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.
 --------------------------------------------------------*/

#ifndef _JOB_CONTROL_H
#define _JOB_CONTROL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

enum status {
	SUSPENDED, SIGNALED, EXITED, CONTINUED
};
enum job_state {
	FOREGROUND, BACKGROUND, STOPPED
};
static char *status_strings[] =
		{ "Suspended", "Signaled", "Exited", "Continued" };
static char *state_strings[] = { "Foreground", "Background", "Stopped" };

typedef struct job_ {
	pid_t pgid;
	char *command;
	enum job_state state;
	struct job_ *next;
} job;

typedef job *job_iterator;

void get_command(char inputBuffer[], int size, char *args[], int *background);

job* new_job(pid_t pid, const char *command, enum job_state state);

void add_job(job *list, job *item);

int delete_job(job *list, job *item);

job* get_item_bypid(job *list, pid_t pid);

job* get_item_bypos(job *list, int n);

enum status analyze_status(int status, int *info);

void print_item(job *item);

void print_list(job *list, void (*print)(job*));

void terminal_signals(void (*func)(int));

void block_signal(int signal, int block);

#define list_size(list)    list->pgid
#define empty_list(list)   !(list->pgid)

#define new_list(name)     new_job(0,name,FOREGROUND)

#define get_iterator(list)   list->next
#define has_next(iterator)   iterator
#define next(iterator)       ({job_iterator old = iterator; iterator = iterator->next; old;})

#define print_job_list(list)   print_list(list, print_item)

#define restore_terminal_signals()   terminal_signals(SIG_DFL)
#define ignore_terminal_signals() terminal_signals(SIG_IGN)

#define set_terminal(pid)        tcsetpgrp (STDIN_FILENO,pid)
#define new_process_group(pid)   setpgid (pid, pid)

#define block_SIGCHLD()   	 block_signal(SIGCHLD, 1)
#define unblock_SIGCHLD() 	 block_signal(SIGCHLD, 0)

#define debug(x,fmt) fprintf(stderr,"\"%s\":%u:%s(): --> %s= " #fmt " (%s)\n", __FILE__, __LINE__, __FUNCTION__, #x, x, #fmt)

#endif

