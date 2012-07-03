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


#include <ServiceSkeleton.h>
#include <iostream>
#include <stdio.h>

#include <axutil_env.h>
#include <Environment.h>
#include <OMText.h>

#include <string>
#include <sstream>
#include <stdlib.h>
#include <algorithm>

#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)

#ifndef LANG
#warning "LANG not defined. Using default 'en'."
#define LANG en
#endif


#include "xlike_freeling.h"
#include "freeling/morfo/traces.h"
#include "config.h"
#include "Extract_ER.h"

using namespace wso2wsf;
using namespace std;

/** Load the service into axis2 engine */
WSF_SERVICE_INIT(xlike_freeling_LANG)


////////////////////////////////////////
//  Parse XML options in the client request 
//  into an "invoke_options" object.

xlike_freeling_LANG::invoke_options::invoke_options(OMElement *op) {
  // required, no default values
  input=op->getChildElement("input",NULL)->getText();
  output=op->getChildElement("output",NULL)->getText();

  // default values for optional parameters
  conll=false; nec=false;
  numbers=true; punct=true; dates=true;
  multiw=true;  ner=true;   quant=true;;

  // check for specified optional parameters
  OMElement *k;
  k=op->getChildElement("conll",NULL); if (k) conll=text2bool(k->getText(),conll);
  k=op->getChildElement("nec",NULL); if (k) nec=text2bool(k->getText(),nec);
  k=op->getChildElement("numbers",NULL); if (k) numbers=text2bool(k->getText(),numbers);
  k=op->getChildElement("punctuation",NULL); if (k) punct=text2bool(k->getText(),punct);
  k=op->getChildElement("dates",NULL); if (k) dates=text2bool(k->getText(),dates);
  k=op->getChildElement("multiwords",NULL); if (k) multiw=text2bool(k->getText(),multiw);
  k=op->getChildElement("ner",NULL); if (k) ner=text2bool(k->getText(),ner);
  k=op->getChildElement("quantities",NULL); if (k) quant=text2bool(k->getText(),quant);
}

////////////////////////////////////////
//  Convert option string into a boolean

bool xlike_freeling_LANG::invoke_options::text2bool(const string &t, bool def) {
  bool b=def;
  if (t=="false" or t=="off" or t=="no" or t=="none") b = false;
  else if (t=="true" or t=="on" or t=="yes") b = true;
  else cerr<<"Invalid boolean option '"<<t
           <<"' in request. Using default ("
           <<(def?"true":"false")<<")";

  return b;
}


////////////////////////////////////////
//  Decode hex codes in URL

#define hex2int(c) (isdigit(c) ? c-'0' : tolower(c)-'a'+10)
#define hex2char(s) static_cast<char>(hex2int(s[0])<<4 | hex2int(s[1]))

string xlike_freeling_LANG::urlDecode(const string &textURL) {
  string ret;
  for (unsigned int i=0; i<textURL.length(); i++) {
    if (textURL[i]=='%') {
      ret += hex2char(textURL.substr(i+1,2));
      i=i+2;
    } 
    else 
      ret += textURL[i];
  }
  return (ret);
}

////////////////////////////////////////
/// Constructor

xlike_freeling_LANG::xlike_freeling_LANG() {};

////////////////////////////////////////
/// Destructor

xlike_freeling_LANG::~xlike_freeling_LANG() {
  delete iden;
  delete tk; 
  delete sp; 
  delete numb; 
  delete punt; 
  delete date;
  delete dico;
  delete loc; 
  delete npm; 
  delete quant; 
  delete prob;
  delete neclass; 
  delete tagger; 
  delete parser;
  delete dep;

  delete fmt;

  delete ext;

};


////////////////////////////////////////
/// Init WebService.
/// This creates ALL modules for the given language.
/// When serving each client request, the modules that
/// actually need to be called for that request will 
/// be determined.

void xlike_freeling_LANG::init() {
  // if FreeLing was compiled with --enable-traces, you can activate
  // the required trace verbosity for the desired modules.
  //traces::TraceLevel=4;
  //traces::TraceModule=0xF;

  // get which language we are compiled for
  wstring language=util::string2wstring(STR(LANG));

  /// set locale to an UTF8 compatible locale
  util::init_locale(L"default");
  std::ios_base::sync_with_stdio(false);

  string path="../services/xlike_freeling_"+string(STR(LANG))+"/analyzer.cfg";

  config cfg(path);

  // create I/O formatter 
  fmt = new IO_handler();

  //create Extracter
  ext = new Extract_ER();


  // create analyzers
  iden=NULL; tk=NULL; sp=NULL; 
  numb=NULL; punt=NULL; date=NULL; dico=NULL;
  loc=NULL; npm=NULL; quant=NULL; prob=NULL;
  neclass=NULL; tagger=NULL; parser=NULL; dep=NULL;

  iden = new lang_ident(cfg.IDENT_identFile);
  tk = new tokenizer(cfg.TOK_TokenizerFile);
  sp = new splitter(cfg.SPLIT_SplitterFile);

  numb = new numbers(cfg.Lang, cfg.MACO_Decimal, cfg.MACO_Thousand);

  if (not cfg.MACO_PunctuationFile.empty())
    punt = new punts(cfg.MACO_PunctuationFile);

  date = new dates(cfg.Lang);

  dico = new dictionary(cfg.Lang, 
                        cfg.MACO_DictionaryFile, 
                        true,   // do affix analysis
                        cfg.MACO_AffixFile, 
                        false,  // no inverse dict
                        true);  // split contractions
  
  if (not cfg.MACO_LocutionsFile.empty()) 
    loc = new locutions(cfg.MACO_LocutionsFile);

  if (not cfg.MACO_NPDataFile.empty())
    npm = new ner(cfg.MACO_NPDataFile);
  
  if (not cfg.MACO_QuantitiesFile.empty())
    quant = new quantities(cfg.Lang, cfg.MACO_QuantitiesFile);

  prob = new probabilities(cfg.Lang, 
                           cfg.MACO_ProbabilityFile, 
                           cfg.MACO_ProbabilityThreshold);
  
  // create a hmm tagger (with retokenization ability, and forced 
  // to choose only one tag per word)
  tagger = new hmm_tagger(cfg.Lang, 
                          cfg.TAGGER_HMMFile, 
                          cfg.TAGGER_Retokenize,
                          cfg.TAGGER_ForceSelect);
  // NE classifier
  if (not cfg.NEC_NECFile.empty()) 
    neclass = new nec (cfg.NEC_NECFile);

  // create chunker  
  parser = new chart_parser(cfg.PARSER_GrammarFile);	

  // create dependency parser 
  dep = new dep_txala(cfg.DEP_TxalaFile, 
                      parser->get_start_symbol ());
}; 


////////////////////////////////////////
/// Service a request, return result.
/// We expect in the message a set of options specifying which
/// service is to be executed

OMElement* xlike_freeling_LANG::invoke(OMElement *inMsg, MessageContext *msgCtx) {

  if (not inMsg) {
    cerr << " Incorrect inMsg " << endl;
    return NULL;
  }

  else if (inMsg->getLocalname()=="ident") {
    // lang ident requested, get text and identify language
    string text = inMsg->getChildElement("text",NULL)->getText();
    wstring wtext=util::string2wstring(urlDecode(text));

    set<wstring> candidates=set<wstring>(); 
    wstring lg = iden->identify_language(wtext, candidates); 

    OMElement *result = new OMElement("identResponse");
    OMElement *lang = new OMElement(result, "language");
    lang->setText(util::wstring2string(lg));

    return result;
  }

  else if (inMsg->getLocalname()=="analyze") {

    // extract options from XML element
    OMElement *opt_element;
    if (msgCtx->getDoingREST()) opt_element = inMsg;  // REST
    else opt_element = inMsg->getChildElement("analysisOptions",NULL);  // SOAP

    invoke_options opt(opt_element); 

    // get text to process
    string text = inMsg->getChildElement("text",NULL)->getText();

    wstring wtext=util::string2wstring(urlDecode(text));


    // process it 
    list<sentence> ls;
    analyze(wtext,opt,ls);


    //adding support for entity relationship extraction

    list <entity> le;
    list <relation> lr;



    ext->extract(ls,le,lr);

    OMElement *result = new OMElement("analyzeResponse");
    OMElement *analysis = new OMElement(result, "analysis");
    analysis->setText(fmt->sentences_to_XML(ls,le,lr,opt.output));

    if (opt.conll) {
      OMElement *conllout = new OMElement(result, "conll");
      conllout->setText(fmt->sentences_to_CONLL(ls,opt.output));
    }

    return result;  
    le.clear();
    lr.clear();


  }

  else
    cerr<<" Unknown operation '"<<inMsg->getLocalname()<<"'"<<endl;

  return NULL; 
  }

////////////////////////////////////////
/// Return error code

OMElement* xlike_freeling_LANG::onFault(OMElement *ele) {
	OMElement *responseEle = new OMElement("xlike_freelingServiceErrorResponse");
	responseEle->setText("xlike_freeling Service Failed");
	return responseEle;
}


/// --------------  Private methods

////////////////////////////////////////
/// Analyze given text

void xlike_freeling_LANG::analyze(const wstring &text_news, 
                                  const invoke_options &opt, 
                                  list<sentence> &ls) {
  wstring text;
  list<word> lw;

  if (opt.input=="text") {  
    // tokenize input line into a list of words
    lw=tk->tokenize(text_news);    
    // Split list of tokens in to sentences, flush buffers at the end.
    ls=sp->split(lw, true);
  }
  else {
    // Sentences are splitted (and maybe tagged).
    // Parse conll format input and load sentences into ls
    fmt->loadSplitted(text_news,ls);
  }

  // if the requested output was "splitted", we are done.
  if (opt.output=="splitted") return;

  // if the input was text or splitted, run morpho and tagger
  if (opt.input!="tagged") {
    // morphosyntactic analysis. 
    // Use only modules that are requested (and were correctly created)
    if (opt.numbers) numb->analyze(ls);
    if (opt.punct and punt) punt->analyze(ls);
    if (opt.dates) date->analyze(ls);
    dico->analyze(ls);
    if (opt.multiw and loc) loc->analyze(ls);
    if (opt.ner and npm) npm->analyze(ls);
    if (opt.quant and quant) quant->analyze(ls);
    prob->analyze(ls); 
    // PoS tagging
    tagger->analyze(ls);
    if (opt.nec and neclass) neclass->analyze(ls);
  }

  // if the requested output was "tagged", we are done.
  if (opt.output=="tagged") return;

  // output was "parsed", run parsers
  // Chunking
  parser->analyze(ls);
  // Dep. parsing
  dep->analyze(ls);  

}



