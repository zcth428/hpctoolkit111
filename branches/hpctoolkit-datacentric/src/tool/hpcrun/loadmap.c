// -*-Mode: C++;-*- // technically C99

// * BeginRiceCopyright *****************************************************
//
// $HeadURL$
// $Id$
//
// --------------------------------------------------------------------------
// Part of HPCToolkit (hpctoolkit.org)
//
// Information about sources of support for research and development of
// HPCToolkit is at 'hpctoolkit.org' and in 'README.Acknowledgments'.
// --------------------------------------------------------------------------
//
// Copyright ((c)) 2002-2013, Rice University
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

#include <sys/time.h>
#include "cct.h"
#include "loadmap.h"
#include "fnbounds_interface.h"
#include "fnbounds_file_header.h"
#include "hpcrun_stats.h"
#include "sample_event.h"
#include "epoch.h"
#include <unwind/common/ui_tree.h>

#include <messages/messages.h>

#include <lib/prof-lean/hpcfmt.h>
#include <lib/prof-lean/spinlock.h>

#include <lib/prof-lean/spinlock.h>
#include <lib/prof-lean/splay-macros.h>

static hpcrun_loadmap_t  s_loadmap;
static hpcrun_loadmap_t* s_loadmap_ptr = NULL;

static dso_info_t* s_dso_free_list = NULL;


/* locking functions to ensure that loadmaps are consistent */
static spinlock_t loadmap_lock = SPINLOCK_UNLOCKED;



//***************************************************************************
// 
//***************************************************************************

dso_info_t*
hpcrun_dso_new()
{
  dso_info_t* x = NULL;

  if (s_dso_free_list) {
    x = s_dso_free_list;
    s_dso_free_list = s_dso_free_list->next;
    if (s_dso_free_list) {
      s_dso_free_list->prev = NULL;
    }
    x->next = NULL; // prev should already be NULL
  }
  else {
    TMSG(DSO, " hpcrun_dso_new");
    x = (dso_info_t*) hpcrun_malloc(sizeof(dso_info_t));
  }

  return x;
}


dso_info_t*
hpcrun_dso_make(const char* name, void** table,
		struct fnbounds_file_header* fh,
		void* startaddr, void* endaddr, unsigned long map_size)
{
  dso_info_t* x = hpcrun_dso_new();
  
  TMSG(DSO," hpcrun_dso_make for module %s", name);

  int namelen = strlen(name) + 1;
  x->name = (char*) hpcrun_malloc(namelen);
  strcpy(x->name, name);

  x->table = table;
  x->map_size = map_size;
  x->nsymbols = 0;
  x->start_to_ref_dist = 0;
  x->start_addr = startaddr;
  x->end_addr = endaddr;
  x->data_root = NULL;

  if (fh) {
    x->nsymbols = (unsigned long)fh->num_entries;
    x->is_relocatable = fh->is_relocatable;

    // Cf. hpcrun_normalize_ip(): Given ip, compute lm_ip:
    //   lm_ip = (ip - lm_mapped_start) + lm_ip_ref
    //         = ip - (lm_mapped_start - lm_ip_ref)
    //         = ip - start_to_ref_dist
    if (fh->is_relocatable) {
      x->start_to_ref_dist = (uintptr_t)startaddr - fh->reference_offset;
    }
  }
  x->next = NULL;
  x->prev = NULL;

  TMSG(DSO, "new dso: start = %p, end = %p, name = %s",
       startaddr, endaddr, name);

  return x;
}


//***************************************************************************

void
hpcrun_dsoList_dump(dso_info_t* dl_list)
{
  for (dso_info_t* x = dl_list; (x); x = x->next) {
    hpcrun_dso_dump(x);
  }
}


void
hpcrun_dso_dump(dso_info_t* x)
{
  printf("%p-%p %s [dso_info_t *%p, table=%p, nsymbols=%ld, relocatable=%d]\n",
	 x->start_addr, x->end_addr, x->name, 
         x, x->table, x->nsymbols, x->is_relocatable);
}


//***************************************************************************
// 
//***************************************************************************

load_module_t*
hpcrun_loadModule_new(const char* name)
{
  load_module_t* x = (load_module_t*) hpcrun_malloc(sizeof(load_module_t));

  //memset(x, 0, sizeof(*x));

  x->id = ++(s_loadmap_ptr->size); // largest id = size

  int namelen = strlen(name) + 1;
  x->name = (char*) hpcrun_malloc(namelen);
  strcpy(x->name, name);

  x->dso_info = NULL;
  x->next = NULL;
  x->prev = NULL;

  return x;
}


//***************************************************************************
// 
//***************************************************************************

void
hpcrun_loadmap_lock() 
{
  spinlock_lock(&loadmap_lock);
}


void
hpcrun_loadmap_unlock()
{
  spinlock_unlock(&loadmap_lock);
}


int
hpcrun_loadmap_isLocked()
{
  return spinlock_is_locked(&loadmap_lock);
}


//***************************************************************************

hpcrun_loadmap_t*
hpcrun_loadmap_new()
{
  TMSG(LOADMAP, " --NEW");
  hpcrun_loadmap_t* x = hpcrun_malloc(sizeof(hpcrun_loadmap_t));
  if (x == NULL) {
    EMSG("New loadmap requested, but allocation failed!!");
    return NULL;
  }

  hpcrun_loadmap_init(x);
  
  return x;
}


void
hpcrun_loadmap_init(hpcrun_loadmap_t* x)
{
  TMSG(LOADMAP, "init");
  memset(x, 0, sizeof(*x));
  
  x->lm_head = NULL;
  x->lm_end = NULL;
  x->size = 0;
}


//***************************************************************************

load_module_t*
hpcrun_loadmap_findByAddr(void* begin, void* end)
{
  TMSG(LOADMAP, "find by address %p -- %p", begin, end);
  for (load_module_t* x = s_loadmap_ptr->lm_head; (x); x = x->next) {
    if (x->dso_info
	&& x->dso_info->start_addr <= begin && end <= x->dso_info->end_addr) {
      TMSG(LOADMAP, "       --->%s", x->name);
      return x;
    }
  }
  TMSG(LOADMAP, "       --->(NOT FOUND)");
  return NULL;
}


load_module_t*
hpcrun_loadmap_findByName(const char* name)
{
  TMSG(LOADMAP, "find by name: %s", name);
  for (load_module_t* x = s_loadmap_ptr->lm_head; (x); x = x->next) {
    if (strcmp(x->name, name) == 0) {
      TMSG(LOADMAP, "       --->FOUND", x->name);
      return x;
    }
  }
  TMSG(LOADMAP, "       --->(NOT FOUND)");
  return NULL;
}

load_module_t*
hpcrun_loadmap_findById(uint16_t id)
{
  TMSG(LOADMAP, "find by id %d", id);
  for (load_module_t* x = s_loadmap_ptr->lm_head; (x); x = x->next) {
    if (x->id == id) {
      TMSG(LOADMAP, "       --->%s", x->name);
      return x;
    }
  }
  TMSG(LOADMAP, "       --->(NOT FOUND)");
  return NULL;
}

const char*
hpcrun_loadmap_findLoadName(const char* name)
{
  TMSG(LOADMAP, "find load name: %s", name);
  for (load_module_t* x = s_loadmap_ptr->lm_head; (x); x = x->next) {
    if (strstr(x->name, name)) {
      TMSG(LOADMAP, "       --->%s", x->name);
      return x->name;
    }
  }
  TMSG(LOADMAP, "       --->(NOT FOUND)");
  return NULL;
}


//***************************************************************************

static void
hpcrun_loadmap_pushFront(load_module_t* lm)
{
  TMSG(LOADMAP, "push front: %s", lm->name);
  // link 'm' at the head of the list of loaded modules
  if (s_loadmap_ptr->lm_head) {
    TMSG(LOADMAP, "previous front = %s", s_loadmap_ptr->lm_head->name);
    s_loadmap_ptr->lm_head->prev = lm;
    lm->next = s_loadmap_ptr->lm_head;
    lm->prev = NULL;
    s_loadmap_ptr->lm_head = lm;
  }
  else {
    TMSG(LOADMAP, " ->First entry");
    s_loadmap_ptr->lm_head = lm;
    s_loadmap_ptr->lm_end = lm;
    lm->next = NULL;
    lm->prev = NULL;
  }
}


#if 0
// Pushes 'lm' to the end of the current loadmap. Should only occur
// when lm's dso_info field has become invalidated, thus creating a
// sub-list of invalid load modules at the end of the loadmap.
static void
hpcrun_loadmap_moveToBack(load_module_t* lm)
{
  // short-circuit if lm is already at end of list
  if (lm == s_loadmap_ptr->lm_end) {
    return;
  }

  // -------------------------------------------------------
  // INVARIANT: lm is not at the end of the list
  // -------------------------------------------------------

  if (lm->prev) {
    lm->prev->next = lm->next;
  }
  else { // if lm->prev == NULL => lm == s_loadmap_ptr->lm_head
    s_loadmap_ptr->lm_head = lm->next;
  }
  
  if (lm->next) {
    lm->next->prev = lm->prev;
  }
  
  lm->prev = s_loadmap_ptr->lm_end;
  lm->prev->next = lm;
  lm->next = NULL;
  s_loadmap_ptr->lm_end = lm;
}
#endif


load_module_t*
hpcrun_loadmap_map(dso_info_t* dso)
{
  const char* msg = "";

  TMSG(LOADMAP, "map in dso %s", dso->name);
  // -------------------------------------------------------
  // Find or create a load_module_t: if a load module exists
  // with same name, reuse it; otherwise create a new entry
  // -------------------------------------------------------
  load_module_t* lm = hpcrun_loadmap_findByName(dso->name);
  if (lm) {
    // sanity check to ensure internal consistency
    if (lm->dso_info != dso) {
      TMSG(LOADMAP, " !! Internal consistency check fires !!");
      hpcrun_loadmap_unmap(lm);
      lm->dso_info = dso;
    }
    else {
      EMSG("hpcrun_loadmap_map(): attempt to both map dso '%s' and place it on the free list!", dso->name);
    }
    msg = "(reuse)";
  }
  else {
    lm = hpcrun_loadModule_new(dso->name);
    lm->dso_info = dso;
    hpcrun_loadmap_pushFront(lm);
  }

  TMSG(LOADMAP, "hpcrun_loadmap_map: '%s' size=%d %s",
       dso->name, s_loadmap_ptr->size, msg);

  return lm;
}


void
hpcrun_loadmap_unmap(load_module_t* lm)
{
  TMSG(LOADMAP,"hpcrun_loadmap_unmap: '%s'", lm->name);

  dso_info_t* old_dso = lm->dso_info;
  lm->dso_info = NULL;

  // tallent: For now, do not move the loadmap to the back of the
  //   list.  If we want to enable, this, we could have
  //   hpcrun_loadmap_findByName() begin its search from the end
  //   of the list.
  //hpcrun_loadmap_moveToBack(lm);

  // add old_dso to the head of the s_dso_free_list
  if (old_dso) {
    old_dso->next = s_dso_free_list;
    old_dso->prev = NULL;
    if (s_dso_free_list) {
      s_dso_free_list->prev = old_dso;
    }
    s_dso_free_list = old_dso;
    TMSG(LOADMAP, "Deleting unw intervals");
    hpcrun_delete_ui_range(old_dso->start_addr, old_dso->end_addr+1);
  }
}


//***************************************************************************
// 
//***************************************************************************

void
hpcrun_initLoadmap()
{
  s_loadmap_ptr = &s_loadmap;
  hpcrun_loadmap_init(s_loadmap_ptr);

  s_dso_free_list = NULL;
}


hpcrun_loadmap_t*
hpcrun_getLoadmap()
{
  return s_loadmap_ptr;
}

// splay operation

static spinlock_t datatree_lock = SPINLOCK_UNLOCKED;

static struct static_data_t **
static_data_interval_splay(struct static_data_t **root, void *key)
{
  INTERVAL_SPLAY_TREE(static_data_t, *root, key, start, end, left, right);
  return root;
}

static void
static_data_interval_splay_insert(struct static_data_t **root, struct static_data_t *node)
{
  void *start = node->start;

  node->left = node->right = NULL;

  spinlock_lock(&datatree_lock);
  if (*root != NULL) {
    root = static_data_interval_splay(root, start);

    if (start < (*root)->start) {
      node->left = (*root)->left;
      node->right = *root;
      (*root)->left = NULL;
    } else if (start > (*root)->start) {
      node->left = *root;
      node->right = (*root)->right;
      (*root)->right = NULL;
    } else {
      assert(0);
    }
  }
  *root = node;
  spinlock_unlock(&datatree_lock);
}

// splay tree query
void *
static_data_interval_splay_lookup(static_data_t **root, void *key, void **start, void **end)
{
  if(!*root) {
    return NULL;
  }

  struct static_data_t *info;
  spinlock_lock(&datatree_lock);
  root = static_data_interval_splay(root, key);
  info = *root;
  if(!info) {
    spinlock_unlock(&datatree_lock);
    return NULL;
  }
  if((info->start <= key) && (info->end > key)) {
    *start = info->start;
    *end = info->end;
    spinlock_unlock(&datatree_lock);
    return info->start;
  }
  spinlock_unlock(&datatree_lock);
  return NULL;
}

void 
insert_var_table(dso_info_t *dso, void **var_table, unsigned long num)
{
  if(!var_table) return;
  int i;
  for (i = 0; i < num; i+=2) {
    // create splay node
    static_data_t *node = (static_data_t *)hpcrun_malloc(sizeof(static_data_t));
    node->start = var_table[i];
    node->end = var_table[i]+(long)var_table[i+1];
    node->left = node->right = NULL;
    static_data_interval_splay_insert(&(dso->data_root), node);
  }
}