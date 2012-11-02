// -*-Mode: C++;-*- // technically C99

// * BeginRiceCopyright *****************************************************
//
// $HeadURL: https://outreach.scidac.gov/svn/hpctoolkit/branches/hpctoolkit-gpu-blame-shift-proto/src/tool/hpcrun/stream_data.c $
// $Id: stream_data.c 3672 2012-02-21 00:39:22Z mfagan $
//
// --------------------------------------------------------------------------
// Part of HPCToolkit (hpctoolkit.org)
//
// Information about sources of support for research and development of
// HPCToolkit is at 'hpctoolkit.org' and in 'README.Acknowledgments'.
// --------------------------------------------------------------------------
//
// Copyright ((c)) 2002-2011, Rice University
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

//
//

//************************* System Include Files ****************************

//************************ libmonitor Include Files *************************

//*************************** User Include Files ****************************

#include "stream.h"

//***************************************************************************

core_profile_trace_data_t *hpcrun_stream_data_alloc_init(int id)
{
	core_profile_trace_data_t *st = hpcrun_mmap_anon(sizeof(core_profile_trace_data_t));
  // FIXME: revisit to perform this memstore operation appropriately.
  //memstore = td->memstore;
  memset(st, 0xfe, sizeof(core_profile_trace_data_t));
  //td->memstore = memstore;
  //hpcrun_make_memstore(&td->memstore, is_child);
  st->id = id;
  st->epoch = hpcrun_malloc(sizeof(epoch_t));
  st->epoch->csdata_ctxt = copy_thr_ctxt(TD_GET(core_profile_trace_data.epoch)->csdata.ctxt); //copy_thr_ctxt(thr_ctxt);
  hpcrun_cct_bundle_init(&(st->epoch->csdata), (st->epoch->csdata).ctxt);
  st->epoch->loadmap = hpcrun_getLoadmap();
  st->epoch->next  = NULL;
  hpcrun_cct2metrics_init(&(st->cct2metrics_map)); //this just does st->map = NULL;


  st->trace_min_time_us = 0;
  st->trace_max_time_us = 0;
  st->hpcrun_file  = NULL;

  return st;	
}

cct_node_t *stream_duplicate_cpu_node(core_profile_trace_data_t *st, ucontext_t *context, cct_node_t *node) {
	cct_bundle_t* cct= &(st->epoch->csdata);
        cct_node_t * tmp_root = cct->tree_root;
        cct_node_t* n = NULL;
        hpcrun_walk_path(node, l_insert_path, (cct_op_arg_t) &(tmp_root));
	return tmp_root;
}

void 
hpcrun_stream_finalize(core_profile_trace_data_t *st) {
	hpcrun_write_profile_data(st);
}















