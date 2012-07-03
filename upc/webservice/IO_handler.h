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

#ifndef FLWS_IOHANDLER_H
#define FLWS_IOHANDLER_H

#include "freeling.h"
#include "Extract_ER.h"

////////////////////////////////////////////////////////////////
//  Auxiliary class to take care of input/output formats
//  for the WebService
////////////////////////////////////////////////////////////////

class IO_handler {
  public:
    IO_handler();
    ~IO_handler();
    std::string sentences_to_XML(const std::list<sentence> &ls, 
        std::list<entity> &le, std::list<relation> &lr, 
        const std::string &outf);
    std::string sentences_to_CONLL(std::list<sentence> &ls, 
        const std::string &outf);
    void loadSplitted(const std::wstring &text, 
        std::list<sentence> &ls);
};

#endif
