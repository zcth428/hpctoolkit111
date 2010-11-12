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
// Copyright ((c)) 2002-2010, Rice University 
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

#include <string.h>
#include "x86-unwind-interval-fixup.h"
#include "x86-unwind-interval.h"

static int intel11_f90main_signature[] = { 
  0xe3894853, 0x80e48348, 0x8b485555, 0x8948086b, 0x4808246c
};


static int 
adjust_intel11_f90main_intervals(char *ins, int len, interval_status *stat)
{
  int siglen = sizeof(intel11_f90main_signature);

  if (len > siglen && strncmp((char *)intel11_f90main_signature, ins, siglen) == 0) {
    // signature matched 
    unwind_interval *ui = (unwind_interval *) stat->first;

    // this won't fix all of the intervals, but it will fix the one we care about.
    while(ui) {
      if (ui->ra_status == RA_STD_FRAME){
	ui->bp_ra_pos = 8;
	ui->bp_bp_pos = 0;
      }
      ui = (unwind_interval *)(ui->common).next;
    }

    return 1;
  } 
  return 0;
}


static void 
__attribute__ ((constructor))
register_unwind_interval_fixup_function(void)
{
  add_x86_unwind_interval_fixup_function(adjust_intel11_f90main_intervals);
}

