//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2004   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    General Public License for more details.
//
//    You should have received a copy of the GNU General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////


#ifndef EXTRACT_ER_H
#define EXTRACT_ER_H

#include "freeling.h"

///////////////////////////////////////////////////////////////////////////
// Entity extraction module.
// This specific module performs entity recognition and relation extraction.
//
// Version Log.
// 0.01 - basic entity extraction using pos tags and named entities. 
/////////////////////////////////////////////////////////////////////////////

class entity {
  public:
    /// entity name
    std::wstring name;
    /// entity instances: this consits of the sentence id.word id
    std::list<std::wstring> instances;
};

class relation {
  public:
    /// relation id
    std::wstring id;
    /// relation name
    std::wstring name;
    /// for relation args
    std::wstring subject;
    std::wstring object;
    /// ids for the relation args
    std::wstring subject_id;
    std::wstring object_id;
};


/////////////////////////////////////7
/// Entities and relations extractor

class Extract_ER {
 public:
  Extract_ER();
  ~Extract_ER();
  
  /// Extract entities and relations from given sentences
  void extract(const std::list<sentence> &ls, 
               std::list<entity> &le,
               std::list<relation> &lr) const;
};





#endif
