// -*-Mode: C++;-*- // technically C99
// $Id$

//***************************************************************************
//
// File: 
//   $Source$
//
// Purpose:
//   LUSH: Logical Unwind Support for HPCToolkit
//
// Description:
//   [The set of functions, macros, etc. defined in the file]
//
// Author:
//   Nathan Tallent, Rice University.
//
//***************************************************************************

#ifndef lush_lush_backtrace_h
#define lush_lush_backtrace_h

//************************* System Include Files ****************************

#include <stdlib.h>
#include <ucontext.h>

//*************************** User Include Files ****************************

#include "lush.h"

#include <state.h>

//*************************** Forward Declarations **************************

#if defined(__cplusplus)
extern "C" {
#endif

//***************************************************************************
// LUSH Agents
//***************************************************************************

// LUSH support
extern lush_agent_pool_t* lush_agents;


//***************************************************************************
// LUSH backtrace
//***************************************************************************

csprof_cct_node_t*
lush_backtrace(csprof_state_t* state, ucontext_t* context,
	       int metric_id, uint64_t metric_incr);


// **************************************************************************

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif /* lush_lush_backtrace_h */
