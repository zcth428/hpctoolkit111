#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "all_sample_sources.h"
#include "csprof_options.h"
#include "files.h"
#include "process_event_list.h"
#include "csprof_misc_fn_stat.h"
#include "env.h"
#include "pmsg.h"
#include "registered_sample_sources.h"

/* option handling */
/* FIXME: this needs to be split up a little bit for different backends */

int
csprof_options__init(csprof_options_t *x)
{
  NMSG(OPTIONS,"__init");
  memset(x, 0, sizeof(*x));
  return CSPROF_OK;
}

int
csprof_options__fini(csprof_options_t* x)
{
  return CSPROF_OK;
}

/* assumes no private 'heap' memory is available yet */
int
csprof_options__getopts(csprof_options_t* x)
{
  /* Option: CSPROF_OPT_LUSH_AGENTS */
  char *s = getenv(CSPROF_OPT_LUSH_AGENTS);
  if (s) {
    strcpy(x->lush_agent_paths, s);
  }
  else {
    x->lush_agent_paths[0] = '\0';
  }

  // process event list
  
  NMSG(OPTIONS,"--before init of registered sample sources");
  csprof_registered_sources_init();
#if 0 // BG/P temporarily use fixed options here
  csprof_sample_sources_from_eventlist(getenv("CSPROF_OPT_EVENT"));
#else
  csprof_sample_sources_from_eventlist("WALLCLOCK:5000");
#endif
  return CSPROF_OK;
}