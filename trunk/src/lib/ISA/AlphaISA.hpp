// $Id$
// -*-C++-*-
// * BeginRiceCopyright *****************************************************
// 
// Copyright ((c)) 2002, Rice University 
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
//***************************************************************************
//                 Copyright (c) 1990-1999 Rice University
//                          All Rights Reserved
//***************************************************************************

//***************************************************************************
//
// File:
//    AlphaISA.h
//
// Purpose:
//    [The purpose of this file]
//
// Description:
//    [The set of functions, macros, etc. defined in the file]
//
//***************************************************************************

#ifndef AlphaISA_H 
#define AlphaISA_H

//************************* System Include Files ****************************

//*************************** User Include Files ****************************

#include <include/general.h>

#include "ISA.hpp"

//*************************** Forward Declarations ***************************

//***************************************************************************
// AlphaISA
//***************************************************************************

// 'AlphaISA': Implements the Alpha Instruction Set Architecture.
// See 'ISA.h' for comments on the interface

class AlphaISA : public ISA {
public:
  AlphaISA() { }
  virtual ~AlphaISA() { }
  
  // --------------------------------------------------------
  // Instructions:
  // --------------------------------------------------------  
  
  virtual ushort GetInstSize(MachInst* mi) { return MINST_SIZE; } 
  
  virtual ushort GetInstNumOps(MachInst* mi) { return 1; }

  virtual InstDesc GetInstDesc(MachInst* mi, ushort opIndex, ushort sz = 0);

  virtual Addr GetInstTargetAddr(MachInst* mi, Addr pc, ushort opIndex,
				 ushort sz = 0);
  
  virtual ushort GetInstNumDelaySlots(MachInst* mi, ushort opIndex,
				      ushort sz = 0)
  { return 0; /* The Alpha has no instruction-specified delay slots. */ }

  virtual bool IsParallelWithSuccessor(MachInst* mi1, ushort opIndex1,
				       ushort sz1,
				       MachInst* mi2, ushort opIndex2,
				       ushort sz2) const
  { return false; }

private: 
  // Should not be used
  AlphaISA(const AlphaISA& i) { }
  AlphaISA& operator=(const AlphaISA& i) { return *this; }

protected:
private:
  static const ushort MINST_SIZE = 4; // machine instruction size in bytes
};

//****************************************************************************

#endif 
