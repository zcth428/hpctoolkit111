/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2007 Intel Corporation 
All rights reserved. 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
/// @file xed-flag-enum.h
/// @author Mark Charney <mark.charney@intel.com>

// This file was automatically generated.
// Do not edit this file.

#if !defined(_XED_FLAG_ENUM_H_)
# define _XED_FLAG_ENUM_H_
#include "xed-common-hdrs.h"
typedef enum {
  XED_FLAG_INVALID,
  XED_FLAG_of, ///<< overflow flag
  XED_FLAG_sf, ///< sign flag
  XED_FLAG_zf, ///< zero flag
  XED_FLAG_af, ///< auxilliary flag
  XED_FLAG_pf, ///< parity flag
  XED_FLAG_cf, ///< carry flag
  XED_FLAG_df, ///< direction flag
  XED_FLAG_vif, ///< virtual interrupt flag
  XED_FLAG_iopl, ///< I/O privilege level
  XED_FLAG_if, ///< interrupt flag
  XED_FLAG_ac, ///< alignment check
  XED_FLAG_vm, ///< virtual-8086 mode
  XED_FLAG_rf, ///< resume flag
  XED_FLAG_nt, ///< nested task
  XED_FLAG_tf, ///< traf flag
  XED_FLAG_id, ///< ID flag
  XED_FLAG_vip, ///< virtual interrupt pending
  XED_FLAG_fc0, ///< x87 FC0 flag
  XED_FLAG_fc1, ///< x87 FC1 flag
  XED_FLAG_fc2, ///< x87 FC2 flag
  XED_FLAG_fc3, ///< x87 FC3 flag
  XED_FLAG_LAST
} xed_flag_enum_t;

XED_DLL_EXPORT xed_flag_enum_t
str2xed_flag_enum_t(const char* s);
XED_DLL_EXPORT const char*
xed_flag_enum_t2str(const xed_flag_enum_t p);

#endif
