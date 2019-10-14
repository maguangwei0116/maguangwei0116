#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <execinfo.h>
#include <signal.h>
#if 1
#include <unwind.h>
#define UNW_LOCAL_ONLY
#include <libunwind.h>

#define DLADDR_ENABLE

#ifdef DLADDR_ENABLE
typedef struct {
   const char *dli_fname;  /* Pathname of shared object that
                              contains address */
   void       *dli_fbase;  /* Base address at which shared
                              object is loaded */
   const char *dli_sname;  /* Name of symbol whose definition
                              overlaps addr */
   void       *dli_saddr;  /* Exact address of symbol named
                              in dli_sname */
} Dl_info;
#endif

struct _Unwind_Context {
	unw_cursor_t cursor;
	char symbol[256];
	int offset;
	int end_of_stack;     /* set to 1 if the end of stack was reached */
};

typedef int (*backtrac_log_func)(const char *fmt, ...);

static backtrac_log_func g_backtrac_log_func = printf;

#define MAX_THREAD_CNT		20

typedef struct THREAD_INFO {
	unsigned long 	pid;
	const char *	name;
} thread_info_t;

typedef struct ALL_THREAD_INFO {
	int32_t 		cnt;
	thread_info_t 	thread[MAX_THREAD_CNT];
} all_thread_info_t;

static all_thread_info_t g_all_thread_info;

int32_t thread_info_record(unsigned long pid, const char *name)
{
	g_all_thread_info.thread[g_all_thread_info.cnt].pid = pid;
	g_all_thread_info.thread[g_all_thread_info.cnt].name = name;
	g_all_thread_info.cnt++;

	return 0;	
}

static int32_t thread_info_dump(void)
{
	int32_t i;
	
	g_backtrac_log_func("All system threads:\r\n");
	for (i = 0; i < g_all_thread_info.cnt; i++) {
		g_backtrac_log_func("# %02d %p: %s\r\n", i+1, g_all_thread_info.thread[i].pid, g_all_thread_info.thread[i].name);
	}

	return 0;
}

static int32_t thread_info_check(void)
{
	int32_t i;
	unsigned long cur_pid = pthread_self();
	
	for (i = 0; i < g_all_thread_info.cnt; i++) {
		if (cur_pid == g_all_thread_info.thread[i].pid) {
			g_backtrac_log_func("crush thread ===> # %02d %p: %s\r\n", i+1, g_all_thread_info.thread[i].pid, g_all_thread_info.thread[i].name);
		}
	}

	return 0;	
}

static unsigned long _Unwind_GetIP0 (struct _Unwind_Context *context)
{
	unw_word_t val;

	unw_get_reg (&context->cursor, UNW_REG_IP, &val);
  
	return val;
}

#define _Unwind_InitContext(context, uc)                                     \
  ((context)->end_of_stack = 0,                                              \
   ((unw_getcontext (uc) < 0 || unw_init_local (&(context)->cursor, uc) < 0) \
    ? -1 : 0))

static _Unwind_Reason_Code _Unwind_Backtrace(void)
{
	struct _Unwind_Context context;
	unw_context_t uc;
	int ret;
	int i = 0;
	Dl_info info;  
	void * const nearest;
	const void *addr;

	if (_Unwind_InitContext (&context, &uc) < 0) {
		return _URC_FATAL_PHASE1_ERROR;
	}

	g_backtrac_log_func("Stack frames:\r\n");
	while (1) {
		if ((ret = unw_step (&context.cursor)) <= 0) {
			//g_backtrac_log_func("unw step ret=%d\r\n", ret);
			if (ret == 0) {
				ret = _URC_END_OF_STACK;
			} else {
				ret = _URC_FATAL_PHASE1_ERROR;
			}
			goto exit_entry;
        }
		
		memset(context.symbol, 0, sizeof(context.symbol));
		if (unw_get_proc_name(&context.cursor, context.symbol, sizeof(context.symbol), &context.offset)) {
			g_backtrac_log_func(" -- error: unable to obtain symbol name for this frame\n");
			continue;
		}
		
		addr = (const void *)_Unwind_GetIP0(&context);
		memset(&info, 0, sizeof(Dl_info));
		ret = dladdr((void *)addr, &info);		
		g_backtrac_log_func("# %02d  %08x  %-25s  %-25s  [%p]\r\n", i+1, addr, info.dli_fname, context.symbol, context.offset);			

		i++;
    }
	
exit_entry:

	if (ret != 0 && ret != _URC_END_OF_STACK) {
		g_backtrac_log_func("error ret=%d\r\n", ret);
	} else {
		g_backtrac_log_func("\r\n");
	}
	
	return ret;
}

static void handle_sigsegv(int sig, siginfo_t *info, void *ucontext)
{
	static const char *si_codes[3] = {"", "SEGV_MAPERR", "SEGV_ACCERR"};
	long ip = 0;
	ucontext_t *uc = (ucontext_t *)ucontext;
	
#if defined(__linux__)
#ifdef UNW_TARGET_X86
	ip = uc->uc_mcontext.gregs[REG_EIP];
#elif defined(UNW_TARGET_X86_64)
	ip = uc->uc_mcontext.gregs[REG_RIP];
#elif defined(UNW_TARGET_ARM)
	ip = uc->uc_mcontext.arm_pc;
	char *head_cpu = NULL;
	ucontext_t *t = uc;
	asprintf(&head_cpu, "r0 %08lx  r1 %08lx  r2 %08lx  r3 %08lx\r\n"
                 "r4 %08lx  r5 %08lx  r6 %08lx  r7 %08lx\r\n"
                 "r8 %08lx  r9 %08lx  sl %08lx  fp %08lx\r\n"
                 "ip %08lx  sp %08lx  lr %08lx  pc %08lx  cpsr %08lx\r\n",
         t->uc_mcontext.arm_r0, t->uc_mcontext.arm_r1, t->uc_mcontext.arm_r2,
         t->uc_mcontext.arm_r3, t->uc_mcontext.arm_r4, t->uc_mcontext.arm_r5,
         t->uc_mcontext.arm_r6, t->uc_mcontext.arm_r7, t->uc_mcontext.arm_r8,
         t->uc_mcontext.arm_r9, t->uc_mcontext.arm_r10, t->uc_mcontext.arm_fp,
         t->uc_mcontext.arm_ip, t->uc_mcontext.arm_sp, t->uc_mcontext.arm_lr,
         t->uc_mcontext.arm_pc, t->uc_mcontext.arm_cpsr);	
#endif
#elif defined(__FreeBSD__)
#ifdef __i386__
	ip = uc->uc_mcontext.mc_eip;
#elif defined(__amd64__)
	ip = uc->uc_mcontext.mc_rip;
#else
#error Port me
#endif
#else
#error Port me
#endif

	g_backtrac_log_func("\r\n=======================================================\r\n");
	g_backtrac_log_func("Signal Info:\r\nrecv signal:%d  si_errno:%d, si_code:%d(%s), address:0x%lx  ip:0x%08lx\r\n\r\n",
			sig,
			info->si_errno, 
			info->si_code,
			si_codes[info->si_code],
			/* this is void*, but using %p would print "(null)"
			 * even for ptrs which are not exactly 0, but, say, 0x123:
			 */
			(long)info->si_addr,
			ip);
			
	void * gcc_crush_addr = __builtin_return_address (0);
	g_backtrac_log_func("gcc_crush_addr: %p\r\n\r\n\r\n", gcc_crush_addr);
			
	g_backtrac_log_func("Head CPU Registers:\r\n%s\r\n", head_cpu);
	if (head_cpu) {
		free(head_cpu);
		head_cpu = NULL;
	}
	
	_Unwind_Backtrace();
	
	thread_info_dump();
	
	thread_info_check();
	
	g_backtrac_log_func("=======================================================\r\n");
		
	exit(-1);
}

int32_t init_backtrace(void *arg)
{
	struct sigaction handler;
  
	memset(&handler,0,sizeof(handler));
	handler.sa_sigaction 	= handle_sigsegv;
	handler.sa_flags 		= SA_SIGINFO;
	sigaction(SIGSEGV, &handler, NULL);
	
	g_backtrac_log_func = (backtrac_log_func)arg;
	
	return 0;
}

#endif