/* x86-64_backtrace.c -- unwinding the stack on x86-64 w/ frame pointers */

#include <sys/types.h>
#include <ucontext.h>

#include <libunwind.h>

#include "backtrace.h"
#include "state.h"
#include "general.h"
#include "util.h"

extern void _start();

#define USE_LIBUNWIND_TO_START 0

int csprof_sample_callstack_from_frame(csprof_state_t *, int,
				       size_t, unw_cursor_t *);

/* XXX duplication */
void
csprof_record_metric_with_unwind(int metric_id, size_t value, int unwinds)
{
  csprof_state_t *state = csprof_get_state();

  if(state != NULL) {
    unw_context_t ctx;
    unw_cursor_t frame;
    int i;

    /* force insertion from the root */
    state->treenode = NULL;
    state->bufstk = state->bufend;
    state = csprof_check_for_new_epoch(state);

    /* FIXME: error checking */
    unw_getcontext(&ctx);
    unw_init_local(&frame, &ctx);
    for(i = 0; i < unwinds; ++i) {
      unw_step(&frame);		/* step out into our caller(s) */
    }

    csprof_sample_callstack_from_frame(state, metric_id, value, &frame);
  }
}

static int
backtrace_done(unw_cursor_t *frame, unw_word_t *pcp, unw_word_t *spp)
{
    unw_word_t pc = *pcp;
    unw_word_t sp = *spp;

    unw_word_t npc, nsp;

    unw_get_reg(frame, UNW_X86_64_RIP, &npc);
    unw_get_reg(frame, UNW_X86_64_RSP, &nsp);

#if 1
    DBGMSG_PUB(CSPROF_DBG_UNWINDING, "npc %lx | pc %lx | nsp %lx | sp %lx",
               npc, pc, nsp, sp);
#endif
    if((nsp == sp) || npc == 0) {
        DBGMSG_PUB(CSPROF_DBG_UNWINDING, "stop due to ==");
        return 1;
    }
    else {
        *pcp = npc;
        *spp = nsp;

        return 0;
    }
}

int
csprof_sample_callstack_from_frame(csprof_state_t *state, int metric_id,
				   size_t sample_count, unw_cursor_t *frame)
{
  /* we define this somewhat trivial macro because the places where it
     currently resides used to just DIE.  DIE'ing is somewhat
     inconvenient for the user of a parallel job, so we take a slightly
     less strict route.  Someday we may want to go back to DIE'ing,
     though--hence the macro. */
#define THROW_UNSAFE \
do { \
state->trampoline_samples++; \
ret = CSPROF_ERR; \
goto EXIT; \
} while(0) \
    /* FIXME: the Alpha port uses these to good effect, but they seem
       somewhat less useful on the x86-64. */
    int first_ever_unwind = (state->bufstk == state->bufend);
    void *sp1 = first_ever_unwind ? (void *) -1 : (state->bufstk)->sp;

    unw_word_t ip;
    unw_word_t rbp;
    /* "thread-local" state for determining whether we are done */
    /* eliminate GCC warnings by assigning */
    unw_word_t program_counter = 0;
    unw_word_t stack_pointer = 0;

    csprof_frame_t *unwind = state->btbuf;
    csprof_frame_t *start;
    csprof_frame_t *end;

    int first_time = 1;
    int ret = CSPROF_OK;

    size_t newsz = 0;

    if(unw_get_reg(frame, UNW_TDEP_IP, &ip) < 0) {
	THROW_UNSAFE;
    }

    unw_get_reg(frame, UNW_TDEP_SP, &rbp);

    DBGMSG_PUB(CSPROF_DBG_UNWINDING, "target sp %p", sp1);

    do {
        /* check for overflow */
        if(unwind == state->bufstk) {
            unwind = csprof_state_expand_buffer(state, unwind);
        }

        DBGMSG_PUB(CSPROF_DBG_UNWINDING, "IP %p | RSP %p", (void *)ip, (void *)rbp);

        /* add the frame, but don't advance since we don't know the
           entire story yet */
        unwind->ip = (void *)ip;

	program_counter = ip;
	stack_pointer = rbp;

        /* next iteration */
        if(unw_step(frame) < 0) {
	    THROW_UNSAFE;
        }

        /* unwinding figures out the CFA for the previous frame */
        if(unw_get_reg(frame, UNW_TDEP_SP, &rbp) < 0) {
	    THROW_UNSAFE;
        }
        unwind->sp = (void *)rbp;

        /* record the ip for the next iteration */
        if(unw_get_reg(frame, UNW_TDEP_IP, &ip) < 0) {
	    /* I don't understand why, but this call almost *always* fails */
	    //THROW_UNSAFE;
	}

        DBGMSG_PUB(CSPROF_DBG_UNWINDING, "post-unwind rsp = %p", unwind->sp);

        /* record the CFA for inserting the trampoline */
        if(first_time) {
            state->extra_state = (void *)rbp;
        }

        /* check to see if we reached the top of the cached call stack */
        if((void *)rbp > sp1) {
	  csprof_frame_t *x = state->bufstk - 1;

	  for( ; x != state->bufend; ++x) {
	    DBGMSG_PUB(CSPROF_DBG_UNWINDING, "(stk) ip %#lx | sp %#lx", x->ip, x->sp);
	  }
	  DBGMSG_PUB(CSPROF_DBG_UNWINDING, "last swizzled at %p", state->swizzle_patch);
            DBGMSG_PUB(CSPROF_DBG_UNWINDING, "breaking post-unwind ip = %p", ip);
            break;
        }

        /* can advance the unwind cache now */
        unwind++;

        first_time = 0;
    } while(!backtrace_done(frame, &program_counter, &stack_pointer));

    DBGMSG_PUB(CSPROF_DBG_UNWINDING, "Done unwinding");

    newsz = unwind - state->btbuf;

    if(newsz != 0) {
        csprof_frame_t *recorded_top = unwind - 1;
        csprof_frame_t *cached_top = state->bufend - 1;

	DBGMSG_PUB(CSPROF_DBG_UNWINDING, "Recording %d samples", newsz);
#if 0
        if(recorded_top->sp == cached_top->sp) {
            fflush(stderr);
            DIE("Eliminate all occurrences of smashing!\n"
                "recorded | cached addr: %#lx | %#lx\n"
                "recorded: %#lx | %#lx / cached: %#lx | %#lx",
		__FILE__, __LINE__, recorded_top, cached_top,
                recorded_top->ip, recorded_top->sp,
		cached_top->ip, cached_top->sp);
        }
        else
#endif
            {
            /* shove the collected frames together */
            memmove(state->bufstk-newsz + (1 - first_ever_unwind),
                    state->btbuf, newsz*sizeof(csprof_frame_t));
            state->bufstk = state->bufstk - newsz + (1 - first_ever_unwind);
            start = state->bufstk + newsz - (1 - first_ever_unwind);
            end = state->bufstk;

            /* ugh */
            if(!first_ever_unwind)
                state->treenode = ((csprof_cct_node_t *)state->treenode)->parent;
            }
    }
    else {
      start = end = state->bufstk;
    }
      

    csprof_state_verify_backtrace_invariants(state);

    if(csprof_state_insert_backtrace(state, metric_id, start,
                                     end, sample_count) != CSPROF_OK) {
        ERRMSG("failure recording callstack sample", __FILE__, __LINE__);
        return CSPROF_ERR;
    }

    csprof_state_verify_backtrace_invariants(state);

 EXIT:
    return ret;
}

/* FIXME: some of the checks from libunwind calls are checked for errors.
   I suppose that's a good thing, but there should be some way (CSPROF_PERF?)
   to turn off the checking to avoid branch prediction penalties...

   Then again, branch prediction penalities after you've already made a
   function call and decoded DWARF unwind info and written several words
   of memory...maybe we don't care that much. */
int
csprof_sample_callstack(csprof_state_t *state, int metric_id,
			size_t sample_count, void *context)
{
    int first_ever_unwind = (state->bufstk == state->bufend);
    void *sp1 = first_ever_unwind ? (void *) -1 : state->bufstk->sp;

    unw_context_t ctx;
    unw_cursor_t frame;

    csprof_state_verify_backtrace_invariants(state);

#if USE_LIBUNWIND_TO_START
    /* would be nice if we could just copy in the signal context... */
    if(unw_getcontext(&ctx) < 0) {
        DIE("Could not initialize unwind context!", __FILE__, __LINE__);
    }
#else
    memcpy(&ctx.uc_mcontext, context, sizeof(mcontext_t));
#endif

    if(unw_init_local(&frame, &ctx) < 0) {
        DIE("Could not initialize unwind cursor!", __FILE__, __LINE__);
    }

#if USE_LIBUNWIND_TO_START
    {
        int i;

        /* unwind through libcsprof and the signal handler */
        for(i=0; i<CSPROF_SAMPLE_CALLSTACK_DEPTH; ++i) {
            if(unw_step(&frame) < 0) {
                DIE("An error occurred while unwinding", __FILE__, __LINE__);
            }
        }
    }
#endif

    return csprof_sample_callstack_from_frame(state, metric_id,
					      sample_count, &frame);
}
