// -*-Mode: C++;-*-
// $Id$
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
//
// File:
//    Procedure.h
//
// Purpose:
//    [The purpose of this file]
//
// Description:
//    [The set of functions, macros, etc. defined in the file]
//
//***************************************************************************

#ifndef Procedure_H 
#define Procedure_H

//************************* System Include Files ****************************

#include <iostream>

//*************************** User Include Files ****************************

#include <include/general.h>

#include "LoadModule.hpp"
#include "Section.hpp"

#include <lib/ISA/ISATypes.hpp>

//*************************** Forward Declarations **************************

class TextSection;

//***************************************************************************
// Procedure
//***************************************************************************

// 'Procedure' represents a procedure in the 'TextSection' of a 'LoadModule'

class Procedure {
public:
  enum Type { Local, Weak, Global, Unknown };
  
  Procedure(TextSection* _sec, String _name, String _linkname, Type t,
	    Addr _begVMA, Addr _endVMA, suint _size);
  virtual ~Procedure();

  TextSection* GetTextSection() const { return sec; }
  LoadModule*  GetLoadModule()  const { return sec->GetLoadModule(); }

  // Returns the name as determined by debugging information; if this
  // is unavailable returns the name found in the symbol table.  (Note
  // that no demangling is performed.)
  String  GetName()     const { return name; }
  String& GetName()           { return name; }

  // Returns the name as found in the symbol table
  String GetLinkName() const { return linkname; }

  // Return type of procedure
  Type  GetType()      const { return type; }

  // Return the begin and end virtual memory address of a procedure.
  // The end address points to the beginning of the last instruction.
  // Note that a different convention is used for the end address of a
  // 'Section'.
  Addr  GetStartAddr() const { return begVMA; }
  Addr  GetEndAddr()   const { return endVMA; }
  void  SetEndAddr(Addr _endVMA) { endVMA = _endVMA; }

  // Return size, which is (end - start) + sizeof(last instruction)
  suint GetSize()      const { return size; }
  void SetSize(suint _size)  { size = _size; }
  
  // Symbolic information: may or may not be available
  String  GetFilename()     const { return filenm; }
  String& GetFilename()           { return filenm; }

  suint   GetBegLine()      const { return begLine; }
  suint&  GetBegLine()            { return begLine; }

  // Return true if virtual memory address 'pc' is within the procedure
  // WARNING: pc must be unrelocated
  bool  IsIn(Addr pc)  const { return (begVMA <= pc && pc <= endVMA); }

  // Return the unique number assigned to this procedure
  suint GetId()        const { return id; }

  // Return the number of instructions in the procedure: FIXME
  suint GetNumInsts()  const { return numInsts; }

  // Return the first and last instruction in the procedure
  Instruction* GetFirstInst() const { return GetInst(begVMA, 0); }
  Instruction* GetLastInst() const;
  
  // Convenient wrappers for the 'LoadModule' versions of the same.
  MachInst*    GetMachInst(Addr pc, ushort &sz) const {
    return sec->GetLoadModule()->GetMachInst(pc, sz);
  }
  Instruction* GetInst(Addr pc, ushort opIndex) const {
    return sec->GetLoadModule()->GetInst(pc, opIndex);
  }
  bool GetSourceFileInfo(Addr pc, ushort opIndex,
			 String &func, String &file, suint &line) const {
    return sec->GetLoadModule()->GetSourceFileInfo(pc, opIndex,
						   func, file, line);
  }
  bool GetSourceFileInfo(Addr startPC, ushort sOpIndex,
			 Addr endPC, ushort eOpIndex,
			 String &func, String &file,
			 suint &startLine, suint &endLine) const {
    return sec->GetLoadModule()->GetSourceFileInfo(startPC, sOpIndex,
						   endPC, eOpIndex, func, file,
						   startLine, endLine);
  }
  
  // Dump contents for inspection
  virtual void Dump(std::ostream& o = std::cerr, const char* pre = "") const;
  virtual void DDump() const;

  friend class ProcedureInstructionIterator;

private:
  // Should not be used
  Procedure() { } 
  Procedure(const Procedure& p) { }
  Procedure& operator=(const Procedure& p) { return *this; }

protected:
private:
  TextSection* sec; // we do not own
  String name;
  String linkname;
  Type   type;
  
  Addr   begVMA; // points to the beginning of the first instruction
  Addr   endVMA; // points to the beginning of the last instruction
  Addr   size;

  // symbolic information: may or may not be known
  String filenm;     // filename and 
  suint  begLine;    //   begin line of definition, if known
  Procedure* parent; // parent routine, if lexically nested

  suint  id;    // a unique identifier
  suint  numInsts;
  
  static suint nextId;
};

//***************************************************************************
// ProcedureInstructionIterator
//***************************************************************************

// 'ProcedureInstructionIterator' enumerates a procedure's
// instructions maintaining relative order.

class ProcedureInstructionIterator {
public:
  ProcedureInstructionIterator(const Procedure& _p);
  ~ProcedureInstructionIterator();

  // Returns the current object or NULL
  Instruction* Current() const {
    if (it != endIt) { return (*it).second; }
    else { return NULL; }    
  }

  // Note: This is the 'operation PC' and may not actually be the true
  // PC!  Use the 'Instruction' for the true PC. 
  Addr CurrentPC() const {
    if (it != endIt) { return (*it).first; }
    else { return 0; } 
  }
  
  void operator++()    { ++it; } // prefix increment
  void operator++(int) { it++; } // postfix increment

  bool IsValid() const { return (it != endIt); } 
  bool IsEmpty() const { return (it == endIt); }

  // Reset and prepare for iteration again
  void Reset();

private:
  // Should not be used
  ProcedureInstructionIterator();
  ProcedureInstructionIterator(const ProcedureInstructionIterator& i);
  ProcedureInstructionIterator& operator=(const ProcedureInstructionIterator& i) { return *this; }

protected:
private:
  const Procedure& p;
  const LoadModule& lm;
  LoadModule::AddrToInstMapItC it;
  LoadModule::AddrToInstMapItC endIt;
};

//***************************************************************************

#endif 
