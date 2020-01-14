#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <execinfo.h>
#include <signal.h>
#include <errno.h>
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

static all_thread_info_t g_all_thread_info = {0};

int32_t thread_info_record(unsigned long pid, const char *name)
{	
	int32_t i = 0;
	
	for (i = 0; i < MAX_THREAD_CNT; i++) {
		if (g_all_thread_info.thread[i].pid == 0) {			
			g_backtrac_log_func("+++>>>>>>>>>>>>>>>>Thread create: %p, %s (%d)\r\n", (void *)pid, name, g_all_thread_info.cnt + 1);
			g_all_thread_info.thread[i].pid = pid;
			g_all_thread_info.thread[i].name = name;
			g_all_thread_info.cnt++;
			break;
		}
	}

	return 0;	
}

int32_t thread_info_clear(unsigned long pid)
{
	int32_t i = 0;
	const char *name;
	
	for (i = 0; i < MAX_THREAD_CNT; i++) {
		if (g_all_thread_info.thread[i].pid == pid) {
			name = g_all_thread_info.thread[i].name;			
			g_backtrac_log_func("--->>>>>>>>>>>>>>>>Thread exit: %p, %s (%d)\r\n", (void *)pid, name, g_all_thread_info.cnt - 1);
			g_all_thread_info.thread[i].pid = 0;
			g_all_thread_info.thread[i].name = NULL;
			g_all_thread_info.cnt--;
			break;
		}
	}

	return 0;	
}

static int32_t thread_info_dump(void)
{
	int32_t i;
	int32_t get_pid = 0;
	unsigned long cur_pid = pthread_self();
	
	g_backtrac_log_func("All user threads:\r\n");
	for (i = 0; i < MAX_THREAD_CNT; i++) {
		if (!get_pid && cur_pid == g_all_thread_info.thread[i].pid) {
			g_backtrac_log_func("# %02d [ * ] %p: %s\r\n", i+1, g_all_thread_info.thread[i].pid, g_all_thread_info.thread[i].name);
			get_pid = 1;
		} else if (0 != g_all_thread_info.thread[i].pid) {
			g_backtrac_log_func("# %02d [   ] %p: %s\r\n", i+1, g_all_thread_info.thread[i].pid, g_all_thread_info.thread[i].name);
		}
	}
	
	if (!get_pid) {
		g_backtrac_log_func("# %02d [ * ] %p: %s\r\n", i+1, cur_pid, "unknow");
	}
	
	g_backtrac_log_func("\r\n");

	return 0;
}

static unsigned long _Unwind_GetIP0 (struct _Unwind_Context *context)
{
	unw_word_t val;

	unw_get_reg (&context->cursor, UNW_REG_IP, &val);
  
	return val;
}

#define BACKTRACE_SIZE   16
 
static void backtrace_dump(void)
{
	int j, nptrs;
	void *buffer[BACKTRACE_SIZE];
	char **strings;
	
	nptrs = backtrace(buffer, BACKTRACE_SIZE);
	
	g_backtrac_log_func("Backtrace() returned %d addresses\n", nptrs);
 
	strings = backtrace_symbols(buffer, nptrs);
	if (strings == NULL) {
		g_backtrac_log_func("backtrace_symbols error\r\n");
	} else { 
		for (j = 0; j < nptrs; j++) {
			g_backtrac_log_func("# %02d  %s\n", j+1, strings[j]);
		}	 
		free(strings);
	}
	g_backtrac_log_func("\r\n");
}

#define _Unwind_InitContext(context, uc)                                     \
  ((context)->end_of_stack = 0,                                              \
   ((unw_getcontext (uc) < 0 || unw_init_local (&(context)->cursor, uc) < 0) \
    ? -1 : 0))
	
static _Unwind_Reason_Code _Unwind_Backtrace(void)
{
	struct _Unwind_Context context;
	unw_context_t uc_org;
	int ret;
	int i = 0;
	Dl_info info;  
	void * const nearest;
	const void *addr;
	
	backtrace_dump();

	if (_Unwind_InitContext (&context, &uc_org) < 0) {
		return _URC_FATAL_PHASE1_ERROR;
	}
	
	g_backtrac_log_func("Stack frames:\r\n");
	while (1) {
		if ((ret = unw_step (&context.cursor)) <= 0) {
			if (ret == 0) {
				ret = _URC_END_OF_STACK;
			} else {
				g_backtrac_log_func("unw step ret=%d, err(%d)=%s\r\n", ret, errno, strerror(errno));
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
		if (addr) {
			memset(&info, 0, sizeof(Dl_info));
			ret = dladdr((void *)addr, &info);	
			g_backtrac_log_func("# %02d  %08x\r\n", i+1, addr);	
			g_backtrac_log_func("# %02d  %08x  %-25s  %-25s  [%p]\r\n", i+1, addr, info.dli_fname, context.symbol, context.offset);			
			i++;
		}
    }
	
exit_entry:

	if (ret != 0 && ret != _URC_END_OF_STACK) {
		g_backtrac_log_func("error ret=%d\r\n", ret);
	} else {
		g_backtrac_log_func("\r\n");
	}
	
	return ret;
}

static void show_arm_regs(void)
{/*
	static volatile unsigned long regs[16];
	__asm__ __volatile__(
		"str r0,%0\t\n"
		"str r1,%1\t\n"
		"str r2,%2\t\n"
		"str r3,%3\t\n"
		"str r4,%4\t\n"
		"str r5,%5\t\n"
		"str r6,%6\t\n"
		"str r7,%7\t\n"
		"str r8,%8\t\n"
		"str r9,%9\t\n"
		"str r10,%10\t\n"
		"str r11,%11\t\n"
		"str r12,%12\t\n"
		"str r13,%13\t\n"
		"str r14,%14\t\n"
		"str r15,%15\t\n"
		:"=r"(regs[0]),"=r"(regs[1]),"=r"(regs[2]),"=r"(regs[3]),
		"=r"(regs[4]),"=r"(regs[5]),"=r"(regs[6]),"=r"(regs[7]),
		"=r"(regs[8]),"=r"(regs[9]),"=r"(regs[10]),"=r"(regs[11]),
		"=r"(regs[12]),"=r"(regs[13]),"=r"(regs[14]),"=r"(regs[15]));
	int i = 0;
	for (i = 0; i < 16; i++) {
		g_backtrac_log_func("R%d: %p  \r\n", i, regs[i]);
	}*/
}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) sizeof(a)/sizeof(a[0])
#endif

typedef struct SIGNAL_INFO {
	int32_t 	no;
	const char *name;
} signal_info_t;

#define SIGNAL_ITEM(signal)		{signal, #signal}

static const signal_info_t g_signal_array[] = 
{
	SIGNAL_ITEM(SIGILL), 
	SIGNAL_ITEM(SIGABRT), 
	SIGNAL_ITEM(SIGBUS), 
	SIGNAL_ITEM(SIGFPE), 
	SIGNAL_ITEM(SIGSEGV), 
	SIGNAL_ITEM(SIGSTKFLT), 
	SIGNAL_ITEM(SIGSYS),
};

void handle_sigsegv(int signo, siginfo_t *info, void *ucontext)
{
	const char *si_codes[3] = {"", "SEGV_MAPERR", "SEGV_ACCERR"};
	long ip = 0;
	int i;
	int get_signo = 0;
	ucontext_t *uc = (ucontext_t *)ucontext;
	
	g_backtrac_log_func("\r\n\r\n\r\n");
	g_backtrac_log_func("=======================================================\r\n");
	
	for (i = 0; ARRAY_SIZE(g_signal_array); i++) {
		if (g_signal_array[i].no == signo) {
			g_backtrac_log_func("Recv signal NO: %d [%s]\r\n", signo, g_signal_array[i].name);
			get_signo = 1;
			break;
		}
	}
	
	if (!get_signo) {
		g_backtrac_log_func("Recv signal NO: %d [%s]\r\n", signo, "unknow");
	}
	g_backtrac_log_func("\r\n");
	
	thread_info_dump();
	
#if defined(__linux__)
#ifdef UNW_TARGET_X86
	ip = uc->uc_mcontext.gregs[REG_EIP];
#elif defined(UNW_TARGET_X86_64)
	ip = uc->uc_mcontext.gregs[REG_RIP];
#elif defined(UNW_TARGET_ARM)
	ip = uc->uc_mcontext.arm_pc;
	ucontext_t *t = uc;
	
	g_backtrac_log_func("Head CPU Registers:\r\n");
	g_backtrac_log_func("r0 %08lx  r1 %08lx  r2 %08lx  r3 %08lx\r\n", t->uc_mcontext.arm_r0, t->uc_mcontext.arm_r1, t->uc_mcontext.arm_r2, t->uc_mcontext.arm_r3);
    g_backtrac_log_func("r4 %08lx  r5 %08lx  r6 %08lx  r7 %08lx\r\n", t->uc_mcontext.arm_r4, t->uc_mcontext.arm_r5, t->uc_mcontext.arm_r6, t->uc_mcontext.arm_r7); 
    g_backtrac_log_func("r8 %08lx  r9 %08lx  sl %08lx  fp %08lx\r\n", t->uc_mcontext.arm_r8, t->uc_mcontext.arm_r9, t->uc_mcontext.arm_r10, t->uc_mcontext.arm_fp);
    g_backtrac_log_func("ip %08lx  sp %08lx  lr %08lx  pc %08lx  cpsr %08lx\r\n", t->uc_mcontext.arm_ip, t->uc_mcontext.arm_sp, t->uc_mcontext.arm_lr, t->uc_mcontext.arm_pc, t->uc_mcontext.arm_cpsr);
	//g_backtrac_log_func("t->uc_mcontext.arm_r1=%s\r\n", t->uc_mcontext.arm_r1);  //It may accure second "Segmentation Fault" !!!!!!
	g_backtrac_log_func("\r\n");
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

	g_backtrac_log_func("Signal Info:\r\n");
	g_backtrac_log_func("si_errno:%d  si_code:%d(%s)\r\n", info->si_errno, info->si_code, \
					(info->si_code >= 0 && info->si_code <= 3) ? si_codes[info->si_code] : "NULL");
	/* this is void*, but using %p would print "(null)"
	 * even for ptrs which are not exactly 0, but, say, 0x123:
	 */
	g_backtrac_log_func("si_pid:%p  address:0x%08lx  ip:0x%08lx\r\n",	info->si_pid, (long)info->si_addr, ip);
	g_backtrac_log_func("\r\n");
		
	void * gcc_crush_addr = __builtin_return_address (0);
	g_backtrac_log_func("gcc_crush_addr: %p\r\n", gcc_crush_addr);
	g_backtrac_log_func("\r\n");	
	
	_Unwind_Backtrace();
	
	g_backtrac_log_func("=======================================================\r\n");
		
	exit(-1);
}

static void sys_dump_stack(void)
{
    Dl_info info;
    int status;
    int *fp = 0, *next_fp = 0;
    int cnt = 0;
	
    //only ARM support , x86 is not supoorted !
    __asm__(
        "mov %0, fp\n"
        : "=r"(fp)
    );

	show_arm_regs();
    g_backtrac_log_func("List current stack ...\n");
	g_backtrac_log_func("fp: %p\r\n", fp);
    
    next_fp = (int *)(*(fp - 3));
    while(next_fp != 0) {
        void * pcur = (void *)*(next_fp - 1);
        status = dladdr (pcur, &info);
        if (status && info.dli_fname && info.dli_fname[0] != '\0') {
            g_backtrac_log_func("# %02d  0x%08x 0x%08x %-25s <  %s + %p  >\r\n", \
                cnt + 1, (unsigned int)pcur, (unsigned int)info.dli_saddr,\
                info.dli_fname, info.dli_sname,\
                (void *)(unsigned long)((unsigned int)pcur - (unsigned int)info.dli_saddr));
        } else {
            g_backtrac_log_func( "# %02d  [%p]\r\n", cnt + 1, (void *)*(next_fp - 1));
        }

        //array[cnt++] = (void *)*(next_fp - 1);
        next_fp = (int *)(*(next_fp-3));
        cnt++;
    }

    g_backtrac_log_func("Backstrace (%d deep)\n", cnt);
	
	void * gcc_crush_addr = __builtin_return_address (0);
	g_backtrac_log_func("gcc_crush_addr: %p\r\n\r\n\r\n", gcc_crush_addr);
			
	_Unwind_Backtrace();
	
	thread_info_dump();
}

static void signal_handler(int signo)
{
	g_backtrac_log_func("=======================================================\r\n");
	g_backtrac_log_func("pid: %p, Recv signo: %d, errno(%d):%s\r\n", (unsigned long)pthread_self(), signo, errno, strerror(errno));
    if (signo == SIGSEGV) { 
		g_backtrac_log_func("Segmentation Fault !\n");        
    } else if (signo == SIGPIPE) {// Ignore SIGPIPE.
		g_backtrac_log_func("SIGPIPE !\n");
    } else if (signo == SIGBUS) {
        g_backtrac_log_func("BUS ERROR !\n");
    }
	sys_dump_stack();
	g_backtrac_log_func("=======================================================\r\n");
	exit(-1);
}  

int32_t init_backtrace(void *arg)
{
	struct sigaction act;
	int32_t i;
	int32_t ret;
	int32_t cnt = ARRAY_SIZE(g_signal_array);
  
	if (arg) {
		g_backtrac_log_func = (backtrac_log_func)arg;
	}

	sigemptyset(&act.sa_mask);
	act.sa_sigaction 	= handle_sigsegv;
	act.sa_flags 		= SA_SIGINFO;
	
	for (i = 0; i < cnt; i++) {		
		ret = sigaction(g_signal_array[i].no, &act, NULL);
		g_backtrac_log_func("Install signal %3d [%-10s] handler, ret=%d, err(%d)=%s\r\n", g_signal_array[i].no, g_signal_array[i].name, ret, errno, strerror(errno));
	}
	
	thread_info_record(pthread_self(), "main-thread");
	
	return 0;
}

#endif