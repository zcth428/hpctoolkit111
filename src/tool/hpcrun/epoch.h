// -*-Mode: C++;-*- // technically C99

// * BeginRiceCopyright *****************************************************
//
// $HeadURL$
// $Id$
//
// -----------------------------------
// Part of HPCToolkit (hpctoolkit.org)
// -----------------------------------
// 
// Copyright ((c)) 2002-2009, Rice University 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// 
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// 
// * Neither the name of Rice University (RICE) nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
// 
// This software is provided by RICE and contributors "as is" and any
// express or implied warranties, including, but not limited to, the
// implied warranties of merchantability and fitness for a particular
// purpose are disclaimed. In no event shall RICE or contributors be
// liable for any direct, indirect, incidental, special, exemplary, or
// consequential damages (including, but not limited to, procurement of
// substitute goods or services; loss of use, data, or profits; or
// business interruption) however caused and on any theory of liability,
// whether in contract, strict liability, or tort (including negligence
// or otherwise) arising in any way out of the use of this software, even
// if advised of the possibility of such damage. 
// 
// ******************************************************* EndRiceCopyright *

/*
  Copyright ((c)) 2002, Rice University 
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  * Neither the name of Rice University (RICE) nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  This software is provided by RICE and contributors "as is" and any
  express or implied warranties, including, but not limited to, the
  implied warranties of merchantability and fitness for a particular
  purpose are disclaimed. In no event shall RICE or contributors be
  liable for any direct, indirect, incidental, special, exemplary, or
  consequential damages (including, but not limited to, procurement of
  substitute goods or services; loss of use, data, or profits; or
  business interruption) however caused and on any theory of liability,
  whether in contract, strict liability, or tort (including negligence
  or otherwise) arising in any way out of the use of this software, even
  if advised of the possibility of such damage.
*/
#ifndef EPOCH_H
#define EPOCH_H

#include <stdio.h>

/* an "epoch" is an interval of time during which no two dynamic 
   libraries are mapped to the same region of the address space. 
   an epoch can span across dlopen and dlclose operations. an epoch
   ends when a dlopen maps a new load module on top of a region of 
   the address space that has previously been occupied by another
   module earlier during the epoch.
*/

// Local includes

#include <lib/prof-lean/hpcio.h>
#include <lib/prof-lean/hpcfmt.h>
#include <lib/prof-lean/hpcrun-fmt.h>

// FIXME:tallent: I moved the declaration of loadmap_src_t from
// hpcrun-fmt.h.  Most likely, the guts of loadmap_src_t should simply
// be replaced with loadmap_entry_t.

typedef struct loadmap_src_t {
  uint id;
  char* name;
  void* vaddr;
  void* mapaddr;
  size_t size;

  struct loadmap_src_t* next;

} loadmap_src_t;


typedef struct hpcrun_epoch_t
{
  struct hpcrun_epoch_t *next;  /* the next epoch */
  unsigned int id;            /* an identifier for disk writeouts */
  unsigned int num_modules;   /* how many modules are loaded? */
  loadmap_src_t *loaded_modules;
} hpcrun_epoch_t;

hpcrun_epoch_t* hpcrun_static_epoch(void);
hpcrun_epoch_t* hpcrun_epoch_new(void);
void            hpcrun_epoch_init(hpcrun_epoch_t* e);
hpcrun_epoch_t* hpcrun_get_epoch(void);

void hpcrun_loadmap_add_module(const char* module_name, void* vaddr, void* mapaddr, size_t size);

/* avoid weird dynamic loading conflicts */

void hpcrun_epoch_lock();
void hpcrun_epoch_unlock();
int  hpcrun_epoch_is_locked();

void hpcrun_finalize_current_epoch(void);

void hpcrun_write_all_epochs(FILE *);
void hpcrun_write_current_loadmap(FILE *);

#endif // EPOCH_H
