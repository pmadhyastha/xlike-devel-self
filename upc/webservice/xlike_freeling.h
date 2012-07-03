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

#ifndef FREELING_WS_H
#define FREELING_WS_H

#include <ServiceSkeleton.h>
#include "freeling.h"
#include "IO_handler.h"
#include "Extract_ER.h"

class xlike_freeling_LANG: public wso2wsf::ServiceSkeleton {

 private:
  // analyzers
  lang_ident *iden;
  tokenizer *tk;
  splitter *sp;
  // morpho modules
  numbers *numb;
  punts *punt;
  dates *date;
  dictionary *dico;
  locutions *loc;
  ner *npm;
  quantities *quant;
  probabilities *prob;
  // tagger, parsers, and others
  hmm_tagger *tagger;
  nec *neclass; 
  chart_parser *parser;
  dep_txala *dep;

  // I/O formatter
  IO_handler *fmt;

  //Entity and Relations extracter
  Extract_ER *ext;

  // ---- private auxiliary class to handle client request options ---
  class invoke_options {
    private: 
      bool text2bool(const std::string &t, bool def);
    public:
      // default values for optional parameters when processing a request
      bool conll, numbers, punct, dates, multiw, ner, quant, nec;
      std::string input, output;
      // constructor
      invoke_options(wso2wsf::OMElement *op);
  }; 
  // ---------------------------------------------------------------

  std::string urlDecode(const std::string &textURL);
  void analyze(const std::wstring &text_news,
               const invoke_options &opt, 
               std::list<sentence> &ls);
 public:
  WSF_EXTERN WSF_CALL xlike_freeling_LANG();  
  WSF_EXTERN WSF_CALL ~xlike_freeling_LANG();  
  void WSF_CALL init();
  wso2wsf::OMElement* WSF_CALL invoke(wso2wsf::OMElement *message, 
                                      wso2wsf::MessageContext *msgCtx);
  wso2wsf::OMElement* WSF_CALL onFault(wso2wsf::OMElement *message);
};


#endif // FREELINGWS_H
