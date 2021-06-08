/**
 UNIX Shell Project

 Sistemas Operativos
 Grados I. Informatica, Computadores & Software
 Dept. Arquitectura de Computadores - UMA

 Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.

 To compile and run the program:
 $ gcc Shell_project.c job_control.c -o Shell
 $ ./Shell
 (then type ^D to exit program)

**/

#include <string.h>
#include "job_control.h"

#define MAX_LINE 256

job *my_job_list;

void signal_handler(int sig) {
	block_SIGCHLD();

	pid_t pid_wait;
	int status, info;
	enum status status_res;
	job *the_job;
	job_iterator iter;

	iter = get_iterator(my_job_list);
	while (has_next(iter)) {
		the_job = next(iter);

		pid_wait = waitpid(the_job->pgid, &status,
		WNOHANG | WUNTRACED | WCONTINUED);
		if (pid_wait == the_job->pgid) {
			status_res = analyze_status(status, &info);
			if (status_res == SUSPENDED) {
				printf("Background job %s... pid: %d, command: %s\n",
						status_strings[status_res], the_job->pgid,
						the_job->command);

				the_job->state = STOPPED;
			} else if (status_res == SIGNALED || status_res == EXITED) {
				printf("Background job %s... pid: %d, command: %s\n",
						status_strings[status_res], the_job->pgid,
						the_job->command);

				delete_job(my_job_list, the_job);
			} else {
				printf("Background job %s... pid: %d, command: %s\n",
						status_strings[status_res], the_job->pgid,
						the_job->command);

				the_job->state = BACKGROUND;
			}
		}
	}

	unblock_SIGCHLD();
}

// -----------------------------------------------------------------------
//                            MAIN
// -----------------------------------------------------------------------

int main(void) {
	char inputBuffer[MAX_LINE];
	int background;
	char *args[MAX_LINE / 2];

	int pid_fork, pid_wait;
	int status;
	enum status status_res;
	int info;

	ignore_terminal_signals();
	my_job_list = new_list("shell jobs");
	signal(SIGCHLD, signal_handler);

	while (1)
	{
		printf("COMMAND->");
		fflush(stdout);
		get_command(inputBuffer, MAX_LINE, args, &background);

		if (!args[0])
			continue;

		if (!strcmp("cd", args[0])) {
			if (!args[1])
				chdir(getenv("HOME"));
			else
				chdir(args[1]);

			continue;
		}

		if (!strcmp("jobs", args[0])) {
			if (!empty_list(my_job_list)) {
				block_SIGCHLD();
				print_job_list(my_job_list);
				unblock_SIGCHLD();
			} else
				printf("No jobs on the list\n");

			continue;
		}

		if (!strcmp("fg", args[0])) {
			block_SIGCHLD();

			int num = 1;
			job *my_job;

			if (args[1])
				num = atoi(args[1]);

			my_job = get_item_bypos(my_job_list, num);

			if (my_job) {
				set_terminal(my_job->pgid);

				if (my_job->state == STOPPED)
					killpg(my_job->pgid, SIGCONT);

				pid_wait = waitpid(my_job->pgid, &status, WUNTRACED);
				status_res = analyze_status(status, &info);

				if (pid_wait == my_job->pgid) {
					if (status_res == SUSPENDED)
						my_job->state = STOPPED;
					else
						delete_job(my_job_list, my_job);
				} else
					printf("Foreground pid: %d, command: %s, %s, info: %d\n",
							pid_fork, args[0], status_strings[status_res],
							info);

				set_terminal(getpid());
			} else
				printf("Error, job does not exist\n");

			unblock_SIGCHLD();

			continue;
		}

		if (!strcmp("bg", args[0])) {
			block_SIGCHLD();

			int num = 1;
			job *my_job;

			if (args[1])
				num = atoi(args[1]);

			my_job = get_item_bypos(my_job_list, num);

			if (my_job) {

				if (my_job->state == STOPPED) {
					my_job->state = BACKGROUND;
					killpg(my_job->pgid, SIGCONT);
				}
			} else
				printf("Error, job does not exist\n");

			unblock_SIGCHLD();

			continue;
		}

		pid_fork = fork();
		if (pid_fork < 0) {
			printf("Fork error: %s\n", args[0]);
			continue;
		}
		if (pid_fork) {
			new_process_group(pid_fork);

			if (background) {
				block_SIGCHLD();
				printf("Background job running... pid: %d, command: %s\n",
						pid_fork, args[0]);

				add_job(my_job_list,
						new_job(pid_fork, args[0], BACKGROUND));
				unblock_SIGCHLD();
			} else {
				set_terminal(pid_fork);

				pid_wait = waitpid(pid_fork, &status, WUNTRACED);
				status_res = analyze_status(status, &info);

				if (pid_wait == pid_fork) {
					if (status_res == SUSPENDED) {
						block_SIGCHLD();
						add_job(my_job_list,
								new_job(pid_fork, args[0], STOPPED));
						unblock_SIGCHLD();
					} else
						printf(
								"Foreground pid: %d, command: %s, %s, info: %d\n",
								pid_fork, args[0],
								status_strings[status_res], info);
				}

				set_terminal(getpid());
			}
		} else {

			new_process_group(getpid());

			if (!background)
				set_terminal(getpid());

			restore_terminal_signals();
			execvp(args[0], args);
			printf("Error, command not found: %s\n", args[0]);
			exit(-1);
		}

	}
}
