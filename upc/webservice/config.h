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

#ifndef _CONFIG
#define _CONFIG

#include <fstream>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

#define MOD_TRACENAME L"CONFIG_OPTIONS"
#define MOD_TRACECODE OPTIONS_TRACE

#define DefaultConfigFile "analyzer.cfg" // default ConfigFile

// codes for ForceSelect
#define FORCE_NONE   0
#define FORCE_TAGGER 1
#define FORCE_RETOK  2

////////////////////////////////////////////////////////////////
///  Class config implements a set of specific options
/// for the NLP analyzer, providing a C++ wrapper to 
/// libcfg+ library.
////////////////////////////////////////////////////////////////

class config {

 public:
  /// Language of text to process
  std::wstring Lang;
  /// Locale of text to process
  std::wstring Locale;

  /// Tokenizer options
  std::wstring TOK_TokenizerFile;
  /// Splitter options
  std::wstring SPLIT_SplitterFile;

  /// Morphological analyzer options
  bool MACO_MultiwordsDetection, 
    MACO_NumbersDetection, MACO_PunctuationDetection, 
    MACO_DatesDetection, MACO_QuantitiesDetection, 
    MACO_DictionarySearch, MACO_ProbabilityAssignment,
    MACO_NERecognition;

  /// Morphological analyzer options
  std::wstring MACO_Decimal, MACO_Thousand;

  /// Language identifier options
  std::wstring IDENT_identFile;

  /// Morphological analyzer options
  std::wstring MACO_LocutionsFile, MACO_QuantitiesFile,
    MACO_AffixFile, MACO_ProbabilityFile, MACO_DictionaryFile, 
    MACO_NPDataFile, MACO_PunctuationFile;
  	 
  double MACO_ProbabilityThreshold;

  // NEC options
  bool NEC_NEClassification;
  std::wstring NEC_NECFile;

  // Sense annotator options
  int SENSE_WSD_which;
  std::wstring SENSE_ConfigFile;
  std::wstring UKB_ConfigFile;

  /// Tagger options
  std::wstring TAGGER_HMMFile;
  bool TAGGER_Retokenize;
  int TAGGER_ForceSelect;

  /// Parser options
  std::wstring PARSER_GrammarFile;

  /// Dependency options
  std::wstring DEP_TxalaFile;    

  /// constructor
  config(const std::string &ConfigFile) {

    // load config file
    std::wifstream fcfg;
    util::open_utf8_file(fcfg,util::string2wstring(ConfigFile));
    if (fcfg.fail()) {
      std::cerr<<"Can not open config file '"+ConfigFile+"'."<<std::endl;
      exit(1);
    }

    std::wstring line;
    while (getline(fcfg,line)) {
      // delete whitespaces and quotes
      size_t p=line.find_first_of(L" \"",0);
      while (p!=std::wstring::npos) {
        line.replace(p,1,L"");
        p=line.find_first_of(L" \"",p);
      }
      // skip comments and empty lines
      if (line.empty() or line[0]==L'#') continue;

      // split at "="
      std::vector<std::wstring> f=util::wstring2vector(line,L"=");

      if (f[0]==L"Lang") Lang=f[1];
      else if (f[0]==L"Locale") Locale=f[1];
      else if (f[0]==L"LangIdentFile") IDENT_identFile=f[1];
      else if (f[0]==L"TokenizerFile") TOK_TokenizerFile=f[1];
      else if (f[0]==L"SplitterFile") SPLIT_SplitterFile=f[1];
      else if (f[0]==L"DecimalPoint") MACO_Decimal=f[1];
      else if (f[0]==L"ThousandPoint") MACO_Thousand=f[1];
      else if (f[0]==L"LocutionsFile") MACO_LocutionsFile=f[1];
      else if (f[0]==L"QuantitiesFile") MACO_QuantitiesFile=f[1];
      else if (f[0]==L"AffixFile") MACO_AffixFile=f[1];
      else if (f[0]==L"ProbabilityFile") MACO_ProbabilityFile=f[1];
      else if (f[0]==L"DictionaryFile") MACO_DictionaryFile=f[1];
      else if (f[0]==L"PunctuationFile") MACO_PunctuationFile=f[1];
      else if (f[0]==L"NPDataFile") MACO_NPDataFile=f[1];
      else if (f[0]==L"NECFile") NEC_NECFile=f[1];
      else if (f[0]==L"TaggerHMMFile") TAGGER_HMMFile=f[1];
      else if (f[0]==L"GrammarFile") PARSER_GrammarFile=f[1];
      else if (f[0]==L"DepTxalaFile") DEP_TxalaFile=f[1];
      else if (f[0]==L"ProbabilityThreshold") 
        MACO_ProbabilityThreshold=util::wstring2double(f[1]);
      else if (f[0]==L"TaggerRetokenize") 
        TAGGER_Retokenize=(f[1]==L"yes" or f[1]==L"true" or f[1]==L"on");
      else if (f[0]==L"TaggerForceSelect") {
        if (f[1]==L"none" or f[1]==L"no") TAGGER_ForceSelect = FORCE_NONE;
        else if (f[1]==L"tagger") TAGGER_ForceSelect = FORCE_TAGGER;
        else TAGGER_ForceSelect = FORCE_RETOK;
      }
      else 
        std::cerr<<"Unexpected option "<<util::wstring2string(f[0])<<" in config file "<<ConfigFile<<std::endl;
    }

    // check options involving Filenames for environment vars expansion.
    ExpandFileName(IDENT_identFile);
    ExpandFileName(TOK_TokenizerFile);
    ExpandFileName(SPLIT_SplitterFile);
    ExpandFileName(MACO_LocutionsFile);
    ExpandFileName(MACO_QuantitiesFile);
    ExpandFileName(MACO_AffixFile);
    ExpandFileName(MACO_ProbabilityFile);
    ExpandFileName(MACO_DictionaryFile); 
    ExpandFileName(MACO_NPDataFile);
    ExpandFileName(MACO_PunctuationFile);
    ExpandFileName(NEC_NECFile); 
    ExpandFileName(SENSE_ConfigFile); 
    ExpandFileName(UKB_ConfigFile); 
    ExpandFileName(TAGGER_HMMFile);
    ExpandFileName(PARSER_GrammarFile); 
    ExpandFileName(DEP_TxalaFile);
  }

 private:

  void ExpandFileName(std::wstring &ws) {
    if (ws.empty()) return;
    std::string s=util::wstring2string(ws);      
    std::string name = s;
    size_t n=name.find_first_of("$"); 
    if (n!=std::string::npos) {
      size_t i=name.find_first_of("/\\",n+1);
      if (i==std::string::npos) i=name.size();
      char* exp=getenv(name.substr(n+1,i-n-1).c_str());
      if (exp==NULL){
        WARNING(L"Undefined variable "+util::string2wstring(name.substr(n+1,i-n-1))+L" in configuration file expanded as empty string.");
        name = name.substr(0,n) + name.substr(i);
      }
      else {
        name = name.substr(0,n) + std::string(exp) + name.substr(i);
      }   
      s=name;
    }

    ws=util::string2wstring(s);
  }
};


#endif

