/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains necessary functions for ex- |
| ecuting new programs and replacing the current image  |
| with a new one.                                       |
---------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include "informative.h"
#include "exec.h"

/*
 * Fork a new child and execute pathname with the given argv,
 * using execvp()
 */
int execvp_process(const char *executable, char *const *argv) {
    int retval = -1;
    int wstatus;
    pid_t pid;

    pid = fork_inf();

    /* Child Process */
    if(pid == 0) { 
        if(execvp_inf(executable, argv))
            _Exit(127);
    }
    
    /* Parent Process */
    else if(pid > 0)
        /* Wait for child with process id of pid to finish 
           execution or change its state then continue executing
           the parent process.                                   */
        if(waitpid_inf(pid, &wstatus, 0) != -1)
        /* Note: 
         * I changed my mind and I think that theres no need to check 
         * how the child process got terminated because I don't see any
         * good reason why it should effect the current program. 
         * So for now the parent process will return succes as long as
         * execvp() and waitpid() returns success.                        */
            
            /* If child exited normally */
            if(WIFEXITED(wstatus))
                retval = WIFEXITED(wstatus);
    
    return retval; 
}

