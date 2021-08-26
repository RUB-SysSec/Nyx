/* fuzz */
#define _GNU_SOURCE

#include <sys/mman.h>
#include <dlfcn.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/uio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#include <time.h>
#include <link.h>
#include <stdbool.h>


#include "kafl_user.h"



void fault_handler(int signo, siginfo_t *info, void *extra) {

    char* log_file_path = NULL;
    char* log_content = NULL;

    asprintf(&log_file_path, "/tmp/data.log.%d", getpid());

    FILE* f = fopen(log_file_path, "r");

    if(f){
        log_content = malloc(0x1000);
        memset(log_content, 0x00, 0x1000);
        fread(log_content, 0x1000-1, 1, f);
        fclose(f);
        printf("%s\n", log_content);
            
        kAFL_hypercall(HYPERCALL_KAFL_PANIC_EXTENDED, (uint64_t)log_content);
    }
    else{
        ucontext_t *context = (ucontext_t *)extra;
        uint64_t reason = 0x8000000000000000ULL | context->uc_mcontext.gregs[REG_RIP] | ( (uint64_t)info->si_signo << 47);

        asprintf(&log_content, "HYPERCALL_KAFL_PANIC_EXTENDED: Signal: %d at 0x%llx", info->si_signo, context->uc_mcontext.gregs[REG_RIP]);
        kAFL_hypercall(HYPERCALL_KAFL_PANIC_EXTENDED, (uint64_t)log_content);
    }
}

static void setHandler(void (*handler)(int,siginfo_t *,void *)){
    hprintf("%s\n", __func__);
    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = handler;

    int (*new_sigaction)(int signum, const struct sigaction *act, struct sigaction *oldact);
    new_sigaction = dlsym(RTLD_NEXT, "sigaction");

    if (new_sigaction(SIGABRT, &action, NULL) == -1) {
        hprintf("sigabrt: sigaction");
        _exit(1);
    }
    hprintf("ALL SIGNAL HANDLERS ARE HOOKED!\n");
}


int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact){
  int (*new_sigaction)(int signum, const struct sigaction *act, struct sigaction *oldact);
  new_sigaction = dlsym(RTLD_NEXT, "sigaction");
  setHandler(fault_handler);
    //printf("%s %p\n", __func__, new_sigaction);

    if(!new_sigaction){
        hprintf("dlerror: %s\n", dlerror());
    }

  if(signum != SIGABRT){
    return new_sigaction(signum, act, oldact);
  }
  return 0;
}

int __libc_start_main(int (*main) (int,char **,char **),
              int argc,char **ubp_av,
              void (*init) (void),
              void (*fini)(void),
              void (*rtld_fini)(void),
              void (*stack_end)) {

    int (*original__libc_start_main)(int (*main) (int,char **,char **),
                    int argc,char **ubp_av,
                    void (*init) (void),
                    void (*fini)(void),
                    void (*rtld_fini)(void),
                    void (*stack_end));


    original__libc_start_main = dlsym(RTLD_NEXT,"__libc_start_main");

     setHandler(fault_handler);
  return original__libc_start_main(main,argc,ubp_av, init,fini,rtld_fini,stack_end);
}

