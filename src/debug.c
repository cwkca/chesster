#include <dlfcn.h>
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const int HANDLED_SIGNALS[] = {SIGQUIT, SIGILL,  SIGABRT, SIGFPE,
                               SIGSEGV, SIGTERM, 0};

#define MAX_STACK_DEPTH 20
#define SKIP_FRAMES 5

void handle_signal(int sig);

void print_call_stack() {
  void *stack_addrs[MAX_STACK_DEPTH];
  int frame_count = backtrace(stack_addrs, MAX_STACK_DEPTH);

  dprintf(STDOUT_FILENO, "Backtrace:\n");
  backtrace_symbols_fd(stack_addrs + SKIP_FRAMES, frame_count - SKIP_FRAMES,
                       STDOUT_FILENO);
}

int enable_stack_traces() {
  const int *sig;

  if (!dlsym(RTLD_DEFAULT, "main")) {
    printf("No symbol info found\n");
    return 1;
  }

  for (sig = HANDLED_SIGNALS; *sig; sig++)
    signal(*sig, handle_signal);

  return 0;
}

void throw(const char *error) {
  printf("%s\n", error);
  abort();
}

void handle_signal(int sig) {
  print_call_stack();
  signal(sig, SIG_DFL);
  raise(sig);
}
