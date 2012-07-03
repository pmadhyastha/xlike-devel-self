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

#include "IO_handler.h"

using namespace std;

////////////////////////////////////////////////////////////////
//  Auxiliary class to take care of input/output formats
//  for the WebService
////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////
//  Constructor 

IO_handler::IO_handler() {}

////////////////////////////////////////////////////////////////
//  Destructor

IO_handler::~IO_handler() {};


////////////////////////////////////////////////////////////////
//  Format sentences into appropiate XML DTD

string IO_handler::sentences_to_XML(const list<sentence> &ls,
    list<entity> &le,list<relation> &lr,const string &outf) {
  
  stringstream out_st;
  int ns=0;
  for (list<sentence>::const_iterator s=ls.begin(); s!=ls.end(); s++) {
    // print sentence XML tag
    out_st<<"<sentence id=\""<<ns<<"\">"<<endl;

    // shift positions to count from sentence begin instead of document begin.
    int offs=0;
    if (not s->empty()) offs=s->begin()->get_span_start();

    int nt=1;
    for (sentence::const_iterator w=s->begin(); w!=s->end(); w++) {
      // print word form, with PoS and lemma chosen by the tagger
      out_st<<"  <token ";
      out_st<<" id=\""<<ns<<"."<<nt<<"\"";
      out_st<<" start=\""<<w->get_span_start()-offs<<"\" end=\""<<w->get_span_finish()-offs<<"\"";
      out_st<<" lemma=\""<<util::wstring2string(w->get_lemma())<<"\"";
      out_st<<" pos=\""<<util::wstring2string(w->get_tag())<<"\"";
      out_st<<">";
      out_st<<util::wstring2string(w->get_form())<<"</token>"<<endl;
      
      nt++;
    }
    
    // close sentence XML tag
    out_st<<"</sentence>"<<endl;
    ns++;
  }

  out_st<<"<annotations>"<<endl;
  int ne=1;
  if (le.empty()) out_st<<"<annotation id=\""<<"NILL"<<"\" displayName=\""<<"NILL"<<"\"";
  else{
    for (list<entity>::iterator e=le.begin(); e!=le.end(); e++){
      out_st<<"<annotation id=\""<<ne<<"\" displayName=\""<<util::wstring2string(e->name)<<"\""<<endl;
      out_st<<" semantics=\""<<"Pending Implementation"<<"\"";
      out_st<<">"<<endl;
      out_st<<"  <instances>"<<endl;
      int in_id=1;
      for (list<wstring>::iterator inst=e->instances.begin(); inst!=e->instances.end(); inst++){
        out_st<<"    <instance id=\""<<in_id<<"\">"<<endl;
        out_st<<"      <token id=\""<<util::wstring2string(*inst)<<"\"/>"<<endl;
        out_st<<"    </instance>"<<endl;
        in_id++;
      }
      out_st<<"  </instances>"<<endl;
      ne++;
    }
  }
  out_st<<"</annotations>"<<endl;

  out_st<<"<assertions>"<<endl;
  int nr=1;
  for (list<relation>::iterator r=lr.begin(); r!=lr.end(); r++){
    out_st<<"  <assertion id=\""<<nr<<"\">"<<endl;
    out_st<<"    <subject id=\""<<util::wstring2string(r->subject_id)<<"\" subjectName=\""<<util::wstring2string(r->subject)<<"\""<<endl;
    out_st<<"    <verb id=\""<<util::wstring2string(r->id)<<"\" verbName=\""<<util::wstring2string(r->name)<<"\""<<endl;
    out_st<<"    <object id=\""<<util::wstring2string(r->object_id)<<"\" objectName=\""<<util::wstring2string(r->object)<<"\""<<endl;
    out_st<<"  </assertion>"<<endl;
    nr++;
  }
  out_st<<"</assertions>"<<endl;
  return out_st.str();
}

////////////////////////////////////////////////////////////////
//  Format sentences to CoNLL format.
//  

// byte position of the parent word (or -1 if x is the top)
#define depen(x) (x.second->get_parent()==NULL ? -1 : (int)x.second->get_parent()->info.get_word().get_span_start()) 

// byte position of word in the sentence 
#define posicio(x) (x.first.get_span_start()) 

//---------------------------------------------
// Given a dep tree, fill a vector with the word nodes in preorder.
// each element retains information about the tree node it comes from
void Preorder(dep_tree::iterator n, vector<pair<word,dep_tree::iterator> > &v) {
  v.push_back(make_pair(n->info.get_word(),n));
  for (dep_tree::sibling_iterator d=n->sibling_begin(); d!=n->sibling_end(); ++d){    
    Preorder(d,v);
  }
}

//---------------------------------------------
// sort the vector according to word position in the text
bool operator <(const pair<word,dep_tree::iterator>& a, const pair<word,dep_tree::iterator>& b) { 
  return (posicio(a)<posicio(b));
}


//---------------------------------------------
string IO_handler::sentences_to_CONLL(list<sentence> &ls, const string &outf) {

  vector<pair<word,dep_tree::iterator> > v;
  map<int,int> trad;   

  stringstream out_st;
  for (list<sentence>::iterator s=ls.begin(); s!=ls.end(); s++) {

    // if parsing was requested, prepare parsing result fields
    if (outf=="parsed") {

      dep_tree &dp = s->get_dep_tree();
      
      // fill a vector with the node for each word and sort them in the original order
      Preorder(dp.begin(),v);
      sort(v.begin(), v.end());
      
      // map byte position -> word position
      for (size_t i=0; i<v.size(); i++) {
        trad.insert(make_pair(posicio(v[i]),i+1));
      }

    }
    
    int n=0;
    for (sentence::const_iterator w=s->begin(); w!=s->end(); w++) {

      // All fields (except id and form) default to "_".  
      // This is the output if outf="splitter".
      string lemma, shtag, tag, morph, pfun;
      int pdep = -1;
      lemma="_";  shtag="_";  tag="_";  morph="_";  pfun="_";    

      // if tagging or more was requested, add tagging result fields
      if (outf=="tagged" or outf=="parsed") {
        lemma = util::wstring2string(w->get_lemma());
        shtag = util::wstring2string(util::lowercase(w->get_tag().substr(0,1)));
        tag = util::wstring2string(w->get_tag());
       
        morph = "_"; //change this to something extracting morph info from tags.
      }

      if (outf == "parsed") {
        pdep = (depen(v[n])==-1 ? 0 : trad[depen(v[n])] );  // head
        pfun =  util::wstring2string(v[n].second->info.get_label());              // function
      }

      // print CoNLL columns.

      // id and form are always present
      out_st<<n+1<<"\t";
      out_st<<util::wstring2string(w->get_form())<<"\t";

      // tagging output
      out_st<<lemma<<"\t";  // lemma
      out_st<<shtag<<"\t";  // short tag
      out_st<<tag<<"\t";    // full tag
      out_st<<morph<<"\t";  // morphological features

      // parsing output
      out_st<< (pdep==-1? 0 : pdep) <<"\t";   // projective head
      out_st<<pfun<<"\t";   // projective function

      out_st<<endl;
      
      n++;
    }
    
    // sentence end: blank line
    out_st<<endl;
  }

  return out_st.str();
}



////////////////////////////////////////////////////////////////
//  Load sentences form CoNLL format

void IO_handler::loadSplitted(const wstring &text, list<sentence> &ls) {

  wstringstream in_st;
  in_st.str(text);

  ls.clear();
  sentence s;
  int id;
  wstring form,lemma,shtag,tag,morph,pdep,pfun,npdep,npfun;
  wstring line;
  while(getline(in_st,line)) {
    if (not line.empty()) {
      wstringstream sin; sin.str(line);
      sin>>id>>form>>lemma>>shtag>>tag>>morph>>pdep>>pfun>>npdep>>npfun;
      word w(form);
      if (lemma!=L"_") { // morphological information is available, load it
        analysis a(lemma,tag);
        w.add_analysis(a);
      }
      if (pdep!=L"_") {  // parsing information available, load it
        // no so easy!  probably loading it here in a temporary space
        // and build the tree once the sentence is complete
      }
      // add word to sentence
      s.push_back(w);
    }
    else {
      // blank line read: sentence ended, add it to list.
      ls.push_back(s);
      s.clear();
    }
  }  
}
