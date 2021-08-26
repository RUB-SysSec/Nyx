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
#include <signal.h>
#include <execinfo.h>

#include "kafl_user.h"

#define BT_BUF_SIZE 100

void *buffer[BT_BUF_SIZE];

void fault_handler(int signo, siginfo_t *info, void *extra) {
    char* log_file_path = NULL;
    char* log_content = malloc(0x1000);
            ucontext_t *context = (ucontext_t *)extra;
    memset(log_content, 0x00, 0x1000);

    asprintf(&log_file_path, "/tmp/error.log");

    /*
    void *buffer[5];

    hprintf("PRE BACKTRACE!\n");
    int nptrs = backtrace(buffer, 1);
    hprintf("POST BACKTRACE! %d\n", nptrs);
	*/

    //hprintf("LOADING %s\n" , log_file_path);

    //fflush(stdout);
    //system("ls /tmp/");
    //sleep(10);
    /*
    FILE* f = fopen(log_file_path, "r");

    if(f){
	int bytes = sprintf(log_content, "--> HYPERCALL_KAFL_PANIC_EXTENDED: Signal: %d at 0x%lx\n", info->si_signo, context->uc_mcontext.mc_rip);
        fread(log_content+bytes, 0x1000-1-bytes, 1, f);
        fclose(f);
        printf("%s\n", log_content);

        kAFL_hypercall(HYPERCALL_KAFL_PANIC_EXTENDED, (uint64_t)log_content);
    }
    else{
    */
    	int pos = 0;
    	if (info->si_signo == 11){
        	pos = sprintf(log_content, "HYPERCALL_KAFL_PANIC_EXTENDED: Signal: %d at 0x%lx (%p)\n", info->si_signo, context->uc_mcontext.mc_rip, info->si_addr);
	}
	else{
	        pos = sprintf(log_content, "HYPERCALL_KAFL_PANIC_EXTENDED: Signal: %d at 0x%lx\n", info->si_signo, context->uc_mcontext.mc_rip);
	}

	//for(int i = 0; i < nptrs; i++){
	//	pos += sprintf(log_content+pos, "-> %p\n", buffer[i]);
	//}
        kAFL_hypercall(HYPERCALL_KAFL_PANIC_EXTENDED, (uint64_t)log_content);
    //}
}

void __assert(const char *func, const char *file, int line, const char *failedexpr){
	char* log_content = malloc(0x1000);
    	memset(log_content, 0x00, 0x1000);
	//int nptrs = backtrace(buffer, BT_BUF_SIZE);
	sprintf(log_content, "HYPERCALL_KAFL_PANIC_EXTENDED: assert: %s %s %d: %s\n", func, file, line, failedexpr);

	kAFL_hypercall(HYPERCALL_KAFL_PANIC_EXTENDED, (uint64_t)log_content);	
}

void __abort(void){
        char* log_content = malloc(0x1000);
        memset(log_content, 0x00, 0x1000);
        //int nptrs = backtrace(buffer, BT_BUF_SIZE);
        sprintf(log_content, "HYPERCALL_KAFL_PANIC_EXTENDED: abort called: %p\n", __builtin_return_address(0));

        kAFL_hypercall(HYPERCALL_KAFL_PANIC_EXTENDED, (uint64_t)log_content);

}

void abort(void){
	char* log_content = malloc(0x1000);
        memset(log_content, 0x00, 0x1000);
        //int nptrs = backtrace(buffer, BT_BUF_SIZE);
	//sprintf(log_content, "HYPERCALL_KAFL_PANIC_EXTENDED: abort called: %d\n", nptrs);
	sprintf(log_content, "HYPERCALL_KAFL_PANIC_EXTENDED: abort called: %p\n", __builtin_return_address(0));

        kAFL_hypercall(HYPERCALL_KAFL_PANIC_EXTENDED, (uint64_t)log_content);

	while(1){}

}

static void setHandler(void (*handler)(int,siginfo_t *,void *)){
    hprintf("%s\n", __func__);
    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = handler;

    int (*new_sigaction)(int signum, const struct sigaction *act, struct sigaction *oldact);
    new_sigaction = dlsym(RTLD_NEXT, "sigaction");

    
    if (new_sigaction(SIGFPE, &action, NULL) == -1) {
        hprintf("sigfpe: sigaction");
        _exit(1);
    }
    if (new_sigaction(SIGILL, &action, NULL) == -1) {
        hprintf("sigill: sigaction");
        _exit(1);
    }
    if (new_sigaction(SIGSEGV, &action, NULL) == -1) {
        hprintf("sigsegv: sigaction");
        _exit(1);
    }
    if (new_sigaction(SIGBUS, &action, NULL) == -1) {
        hprintf("sigbus: sigaction");
        _exit(1);
    }
    if (new_sigaction(SIGABRT, &action, NULL) == -1) {
        hprintf("sigabrt: sigaction");
        _exit(1);
    }
    if (new_sigaction(SIGIOT, &action, NULL) == -1) {
        hprintf("sigiot: sigaction");
        _exit(1);
    }
    if (new_sigaction(SIGTRAP, &action, NULL) == -1) {
        hprintf("sigiot: sigaction");
        _exit(1);
    }
    if (new_sigaction(SIGSYS, &action, NULL) == -1) {
        hprintf("sigsys: sigaction");
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

int atexit(void (*function)(void)) {
  int (*original_atexit)(void (*)(void)) = dlsym(RTLD_NEXT, "atexit");
  setHandler(fault_handler);
  return original_atexit(function);
}
