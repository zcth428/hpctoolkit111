#include <stdio.h>
#include <setjmp.h>
#include "pmsg.h"
#include "monitor.h"
#include "fname_max.h"
#include "backtrace.h"
#include "files.h"
#include "state.h"
#include "thread_data.h"
#include "hpcrun_return_codes.h"
#include "hpcio.h"
#include "hpcfmt.h"
#include "hpcrun-fmt.h"
#include "hpcrun_write_data.h"

static char *
hpcrun_itos(char *buf, int i)
{
  sprintf(buf, "%d", i);
  return buf;
}

// TODO --- convert to New fmt
//          Be sure to fix corresponding read routines in ../../lib/prof-juicy/CallPath-Profile.cpp
//                Toplevel read = Profile::make

int
hpcrun_write_profile_data(csprof_state_t *state)
{
  FILE* fs;

  int ret, ret1, ret2;

  // FIXME: Does this go here? Why not in fini_thread / fini process?

#if defined(HOST_SYSTEM_IBM_BLUEGENE)
  EMSG("Backtrace for last sample event:\n");
  dump_backtrace(state, state->unwind);
#endif

  /* Generate a filename */
  char fnm[HPCRUN_FNM_SZ];


  int rank = monitor_mpi_comm_rank();
  if (rank < 0) rank = 0;
  files_profile_name(fnm, rank, HPCRUN_FNM_SZ);

  sigjmp_buf_t *it = &(TD_GET(mem_error));

  if (sigsetjmp(it->jb,1)){
    EEMSG("***hpcrun failed to write profile data due to memory allocation failure***");
    return HPCRUN_ERR;
  }

  TMSG(DATA_WRITE, "Filename = %s", fnm);

  /* Open file for writing; fail if the file already exists. */
  fs = hpcio_fopen_w(fnm, /* overwrite */ 0);

//***************************************************************************
//
//        The top level loop
//
//  hpcrun_fmt_hdr_fwrite()
//  hpcrun_le4_fwrite(# epochs)
//  foreach epoch
//     hpcrun_epoch_fwrite()
//
// hpcrun_epoch_fwrite()
//    hpcrun_fmt_epoch_hdr_fwrite(flags, char-rtn-dst, gran, NVPs)
//    hpcrun_fmt_metric_tbl_fwrite()
//    hpcrun_fmt_loadmap_fwrite()
//    /* write cct */
//    hpcrun_le4_fwrite(# cct_nodes)
//    foreach cct-node
//       hpcrun_fmt_cct_node_fwrite(cct_node_t *p)
//
//***************************************************************************

  char _tmp[10];
  hpcfile_csprof_data_t *tmp = csprof_get_metric_data();

  //
  // ==== hdr =====
  //
  TMSG(DATA_WRITE,"writing file header");
  hpcrun_fmt_hdr_fwrite(fs,
                        "program-name", "TBD",
                        "process_id", "TBD",
                        "mpi-rank", hpcrun_itos(_tmp, rank),
                        "target", tmp->target,
                        END_NVPAIRS);

  // FIXME-MWF: for the moment, # epochs = # ccts

  //
  // === # epochs === 
  //

  csprof_state_t *runner = state;

  uint32_t num_ccts = 0;
  uint64_t num_tramp_samps = 0;

  /* count states */
  while(runner != NULL) {
    if(runner->epoch != NULL) {
      num_ccts++;
      num_tramp_samps += runner->trampoline_samples;
    }
    runner = runner->next;
  }
  TMSG(DATA_WRITE, "writing # epochs = %d", num_ccts);
  hpcio_fwrite_le4(&num_ccts, fs);

  //
  // for each epoch ...
  //

  TMSG(DATA_WRITE,"metric data target = %s",tmp->target);

  TMSG(DATA_WRITE,"metric data target = %s",tmp->target);
  TMSG(DATA_WRITE,"metric data num metrics = %d",tmp->num_metrics);
  hpcfile_csprof_metric_t *tmp1 = tmp->metrics;
  for (int i=0;i<tmp->num_metrics;i++,tmp1++){
    TMSG(DATA_WRITE,"--metric %s period = %ld",tmp1->metric_name,tmp1->sample_period);
  }
  ret1 = hpcfile_csprof_write(fs, tmp);

  TMSG(DATA_WRITE, "Done writing metric data");

  if(ret1 != HPCFILE_OK) {
    goto error;
  }

  TMSG(DATA_WRITE, "Preparing to write epochs");
  csprof_write_all_epochs(fs);

  TMSG(DATA_WRITE, "Done writing epochs");

  /* write profile states out to disk */

  runner = state;

  while(runner != NULL) {
    if(runner->epoch != NULL) {
      TMSG(DATA_WRITE, "Writing %ld nodes", runner->csdata.num_nodes);
      ret2 = csprof_csdata__write_bin(fs, runner->epoch->id, 
                                      &runner->csdata, runner->csdata_ctxt);
          
      if(ret2 != HPCRUN_OK) {
        TMSG(DATA_WRITE, "Error writing tree %#lx", &runner->csdata);
        TMSG(DATA_WRITE, "Number of tree nodes lost: %ld", runner->csdata.num_nodes);
        EMSG("could not save profile data to file '%s'", __FILE__, __LINE__, fnm);
        perror("write_profile_data");
        ret = HPCRUN_ERR;
      }
    }
    else {
      TMSG(DATA_WRITE, "Not writing tree %#lx; null epoch", &runner->csdata);
      TMSG(DATA_WRITE, "Number of tree nodes lost: %ld", runner->csdata.num_nodes);
    }

    runner = runner->next;
  }
          
  if(ret1 == HPCFILE_OK && ret2 == HPCRUN_OK) {
    TMSG(DATA_WRITE, "saved profile data to file '%s'", fnm);
  }
  /* if we've gotten this far, there haven't been any fatal errors */
  goto end;

 error:
 end:

  TMSG(DATA_WRITE,"closing file");
  hpcio_close(fs);
  TMSG(DATA_WRITE,"Done!");

//***************************************************************************
//
//  return HPCRUN_OK;
//
//***************************************************************************

  return ret;
}
