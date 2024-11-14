#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>
#include <limits.h>

const int PIPE_READ = 0;
const int PIPE_WRITE = 1;

int main(int argc, char **argv) {
    printf("argc: %d\n", argc);
    printf("argv:\n");

    for (int arg_idx = 0; arg_idx < argc; arg_idx++) {
        printf("%s\n", argv[arg_idx]);
    }

    if (argc < 2) {
        fprintf(stderr, "at least 1 argument is needed\n");
        exit(EXIT_FAILURE);
    }

    int in_pipefd[2];
    if (pipe(in_pipefd)) {
        perror("input pipe");
        exit(EXIT_FAILURE);
    }

    int out_pipefd[2];
    if (pipe(out_pipefd)) {
        perror("output pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) { // ERROR
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // CHILD
        setvbuf(stdout, NULL, _IONBF, 0);
        
        if (close(in_pipefd[PIPE_WRITE])) {
            perror("close input pipe write");
            exit(EXIT_FAILURE);
        }

        if (close(out_pipefd[PIPE_READ])) {
            perror("close output pipe read");
            exit(EXIT_FAILURE);
        }
        
        if (dup2(in_pipefd[PIPE_READ], STDIN_FILENO) == -1) {
            perror("dup2 child stdin");
            exit(EXIT_FAILURE);
        }

        if (dup2(out_pipefd[PIPE_WRITE], STDOUT_FILENO) == -1) {
            perror("dup2 child stdout");
            exit(EXIT_FAILURE);
        }
        
        execvp(argv[1], &argv[1]);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else { // PARENT
        if (close(in_pipefd[PIPE_READ])) {
            perror("close input pipe write");
            exit(EXIT_FAILURE);
        }
        
        if (close(out_pipefd[PIPE_WRITE])) {
            perror("close output pipe read");
            exit(EXIT_FAILURE);
        }

        struct pollfd pfds[] = {
            {
                .fd = STDIN_FILENO,
                .events = POLLIN,
                .revents = 0,
            },
            {
                .fd = out_pipefd[PIPE_READ],
                .events = POLLIN,
                .revents = 0,
            },
        };

        // pipe is open or there is still data in it
        while (!(pfds[1].revents & POLLHUP) || pfds[1].revents & POLLIN) {
            const ssize_t BUFFER_SIZE = PIPE_BUF;
            char buffer[BUFFER_SIZE];

            if (poll(pfds, 2, -1) == -1) {
                perror("poll");
                exit(EXIT_FAILURE);
            }

            if (pfds[0].revents & POLLERR) {
                    perror("pipe error");
                    exit(EXIT_FAILURE);
            }

            // data can be written to the child's input
            if (pfds[0].revents & POLLIN) {
                ssize_t read_count = read(STDIN_FILENO, buffer, BUFFER_SIZE);
                if (read_count == -1) {
                    perror("read to stdin");
                    exit(EXIT_FAILURE);
                }

                ssize_t write_count = write(in_pipefd[PIPE_WRITE], buffer, read_count);
                if (write_count == -1) {
                    perror("write to child stdin");
                    exit(EXIT_FAILURE);
                }
            }

            // data can be read from the child's output
            if (pfds[1].revents & POLLIN) {
                ssize_t read_count = read(out_pipefd[PIPE_READ], buffer, BUFFER_SIZE);
                if (read_count == -1) {
                    perror("read child stdout");
                    exit(EXIT_FAILURE);
                }

                ssize_t write_count = write(STDOUT_FILENO, buffer, read_count);
                if (write_count == -1) {
                    perror("write to stdout");
                    exit(EXIT_FAILURE);
                }
            }
        }
        
        int wstatus;
        
        waitpid(pid, &wstatus, 0);
        printf("process exited with status: %d\n", wstatus);
    }
    
    return 0;
}
