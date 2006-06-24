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
//    Section.h
//
// Purpose:
//    [The purpose of this file]
//
// Description:
//    [The set of functions, macros, etc. defined in the file]
//
//***************************************************************************

#ifndef Section_H 
#define Section_H

//************************* System Include Files ****************************

#include <deque>
#include <iostream>

//*************************** User Include Files ****************************

#include <include/general.h>
#include <include/gnu_bfd.h>

#include "LoadModule.hpp"
#include <lib/ISA/ISATypes.hpp> 
#include <lib/support/String.hpp>


//*************************** Forward Declarations **************************

class LoadModule;
class Procedure;
class Instruction;

//***************************************************************************
// Section
//***************************************************************************

// 'Section' is a base class for representing file segments/sections
// of a 'LoadModule'.

class Section {
public: 
  enum Type {BSS, Text, Data, Unknown};
  
  Section(LoadModule* _lm, String _name, Type t, Addr _start,
          Addr _end, Addr _sz);
  virtual ~Section();

  LoadModule* GetLoadModule() const { return lm; }

  // Return name and type of section
  String GetName() const { return name; }
  Type  GetType()  const { return type; }
  
  // Return start/ending virtual memory address for section.  The end
  // of a section is equal to the start address of the next section
  // (or the end of the file).  Note that a different convention is
  // used for the end address of a 'Procedure'.
  Addr  GetStart() const { return start; }
  Addr  GetEnd()   const { return end; }

  // Return size of section
  Addr GetSize()  const { return size; }

  // Return true if virtual memory address 'pc' is within the section
  // WARNING: pc must be unrelocated
  bool  IsIn(Addr pc) const { return (start <= pc && pc < end); }

  // Convenient wrappers for the 'LoadModule' versions of the same.
  MachInst*    GetMachInst(Addr pc, ushort &sz) const {
    return lm->GetMachInst(pc, sz);
  }
  Instruction* GetInst(Addr pc, ushort opIndex) const {
    return lm->GetInst(pc, opIndex);
  }
  bool GetSourceFileInfo(Addr pc, ushort opIndex,
			 String &func, String &file, suint &line) const {
    return lm->GetSourceFileInfo(pc, opIndex, func, file, line);
  }
  bool GetSourceFileInfo(Addr startPC, ushort sOpIndex,
			 Addr endPC, ushort eOpIndex,
			 String &func, String &file,
			 suint &startLine, suint &endLine) const {
    return lm->GetSourceFileInfo(startPC, sOpIndex, endPC, eOpIndex,
				 func, file, startLine, endLine);
  }

  // Dump contents for inspection
  virtual void Dump(std::ostream& o = std::cerr, const char* pre = "") const;
  virtual void DDump() const;
  
protected:
  // Should not be used
  Section() { } 
  Section(const Section& s) { }
  Section& operator=(const Section& s) { return *this; }
private:
  
protected:
private:
  LoadModule* lm; // we are not owners

  String name;
  Type   type;
  Addr   start;  // beginning of section 
  Addr   end;    // end of section [equal to the beginning of next section]
  Addr   size;   // size in bytes
};

//***************************************************************************
// TextSection
//***************************************************************************

// 'TextSection' represents a text segment in a 'LoadModule' and
// implements special functionality pertaining to it.

class TextSectionImpl; 

class TextSection : public Section { 
public:
  TextSection(LoadModule* _lm, String _name, Addr _start, Addr _end,
	      suint _size, asymbol **syms, int numSyms, bfd *abfd);
  virtual ~TextSection();

  suint GetNumProcedures() const { return procedures.size(); }

  // Dump contents for inspection
  virtual void Dump(std::ostream& o = std::cerr, const char* pre = "") const;
  virtual void DDump() const;
  
  friend class TextSectionProcedureIterator;

private:
  // Should not be used
  TextSection() { }
  TextSection(const TextSection& s) { }
  TextSection& operator=(const TextSection& s) { return *this; }

  void Create_InitializeProcs();
  void Create_DisassembleProcs();

  // Construction helpers
  String FindProcedureName(bfd *abfd, asymbol *procSym) const;
  Addr FindProcedureEnd(int funcSymIndex) const;
  Instruction* MakeInstruction(bfd *abfd, MachInst* mi, Addr pc,
			       ushort opIndex, ushort sz) const;
  
  // Procedure sequence: 'deque' supports random access iterators (and
  // is thus sortable with std::sort) and constant time insertion/deletion at
  // beginning/end.
  typedef std::deque<Procedure*>           ProcedureSeq;
  typedef std::deque<Procedure*>::iterator ProcedureSeqIt;
  typedef std::deque<Procedure*>::const_iterator ProcedureSeqItC;

protected:
private:
  TextSectionImpl* impl;
  ProcedureSeq procedures;
};

//***************************************************************************
// TextSectionProcedureIterator
//***************************************************************************

// 'TextSectionProcedureIterator': iterate over the 'Procedure' in a
// 'TextSection'.  No order is guaranteed.

class TextSectionProcedureIterator {
public: 
  TextSectionProcedureIterator(const TextSection& _sec);
  ~TextSectionProcedureIterator();

  // Returns the current object or NULL
  Procedure* Current() const {
    if (it != sec.procedures.end()) { return *it; }
    else { return NULL; }
  }
  
  void operator++()    { ++it; } // prefix increment
  void operator++(int) { it++; } // postfix increment

  bool IsValid() const { return it != sec.procedures.end(); } 
  bool IsEmpty() const { return it == sec.procedures.end(); }

  // Reset and prepare for iteration again
  void Reset() { it = sec.procedures.begin(); }

private:
  // Should not be used
  TextSectionProcedureIterator();
  TextSectionProcedureIterator(const TextSectionProcedureIterator& i);
  TextSectionProcedureIterator& operator=(const TextSectionProcedureIterator& i) { return *this; }

protected:
private:
  const TextSection& sec;
  TextSection::ProcedureSeqItC it;
};

//****************************************************************************

#endif 
