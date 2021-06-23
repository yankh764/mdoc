/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains necessary functions for ex- |
| ecuting new programs and replacing the current image  |
| with a new one.                                       |
---------------------------------------------------------
*/

#include <stdlib.h>
#include "informative.h"
#include "exec.h"


/*
 * Fork a new child and execute the executable with the given argv,
 * using execvp().
 */
int execvp_process(const char *executable, char *const *argv) {
    int retval = -1;
    int wstatus;
    pid_t pid;

    pid = fork_inf();

    /* Child Process */
    if(pid == 0) { 
        if(execvp_inf(executable, argv))
        /* The exit status of 127 is the value set by the shell 
           when a command is not found and the recommended exit 
           status by POSIX in these situations.                 */
            _Exit(127);
    }
    
    /* Parent Process */
    else if(pid > 0)
        /* Wait for child with process id of pid to get terminated 
           or change its state. Then continue executing the parent
           process.                                                */
        if(waitpid_inf(pid, &wstatus, 0) != -1)
            /* If child exited normally and it's exit 
               status isn't 127. See comment on line 30 */
            if(WIFEXITED(wstatus) && WEXITSTATUS(wstatus) != 127)
                retval = 0;
    
    return retval; 
}

