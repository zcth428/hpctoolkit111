/* -*-Mode: C;-*- */
/* $Id$ */

/****************************************************************************
//
// File: 
//    $Source$
//
// Purpose:
//    General PAPI support.
//
// Description:
//    [The set of functions, macros, etc. defined in the file]
//
// Author:
//    Written by John Mellor-Crummey and Nathan Tallent, Rice University.
//
*****************************************************************************/

/************************** System Include Files ****************************/

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <papiStdEventDefs.h>
#include <papi.h>

/**************************** User Include Files ****************************/

#include "hpcpapi.h"

/****************************************************************************/

int
hpc_init_papi(int (*is_init)(void), int (*init)(int))
{
  if ((*is_init)() == PAPI_NOT_INITED) {
    return hpc_init_papi_force(init);
  }
  
  return 0;
}


/****************************************************************************/

int
hpc_init_papi_force(int (*init)(int))
{
  /* Initialize PAPI library */
  int papi_version; 
  papi_version = (*init)(PAPI_VER_CURRENT);
  if (papi_version != PAPI_VER_CURRENT) {
    fprintf(stderr, "(pid %d): PAPI library initialization failure - expected version %d, dynamic library was version %d. Aborting.\n", getpid(), PAPI_VER_CURRENT, papi_version);
    return 1;
  }
  
  if (papi_version < 3) {
    fprintf(stderr, "(pid %d): Using PAPI library version %d; expecting version 3 or greater.\n", getpid(), papi_version);
    return 1;
  }
  return 0;
}

/****************************************************************************/

static hpcpapi_flagdesc_t papi_flags[] = {
  { PAPI_PROFIL_POSIX,    "PAPI_PROFIL_POSIX" },
  { PAPI_PROFIL_RANDOM,   "PAPI_PROFIL_RANDOM" },  
  { PAPI_PROFIL_WEIGHTED, "PAPI_PROFIL_WEIGHTED" },  
  { PAPI_PROFIL_COMPRESS, "PAPI_PROFIL_COMPRESS" },  
  { -1,                   NULL }
}; 

const hpcpapi_flagdesc_t *
hpcpapi_flag_by_name(const char *name)
{
  hpcpapi_flagdesc_t *i = papi_flags;
  for (; i->name != NULL; i++) {
    if (strcmp(name, i->name) == 0) return i;
  }
  return NULL;
}

/****************************************************************************/
