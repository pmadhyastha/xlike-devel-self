/////////////////////////////////////////////////////////////////
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
///             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////
//
// v0.01 - simple implementation of the map, in the next iteration I will use
// multimap to store sentences and instance
//
//
//
//
#include "Extract_ER.h"
#include <list>
#include <stack>
#include <map>
#include<sstream>

using namespace std;

Extract_ER::Extract_ER(){}
Extract_ER::~Extract_ER(){}

entity Entity;
relation Relation;


int compare=0;
int sent_id=0;
map<wstring, list<wstring> > instances;
map<wstring, list<wstring> >::iterator iterate;
void Extract_ER::extract(const list<sentence> &ls,
    list<entity> &le, 
    list<relation> &lr) const {
  lr.clear();
  le.clear();
  for(list<sentence>::const_iterator s = ls.begin(); 
      s != ls.end(); s++){

    int word_id=1;
    map<wstring, int> entities;
    map<int, wstring> reverse_entities;
    map<wstring, int> relations;
    stack<int> id;
    map<wstring, int>::iterator it;
    for(sentence::const_iterator w = s->begin(); w != s->end(); w++){

      if(util::wstring2string(util::lowercase(w->get_tag().substr(0, 1)))=="n"){
      
        wstringstream concat;
        concat << sent_id <<"." <<word_id;
        wstring pairs = concat.str();
        entities.insert(make_pair(w->get_lemma(),word_id));
        reverse_entities.insert(make_pair(word_id, w->get_lemma()));
        if(instances.find(w->get_lemma()) != instances.end()){

          list<wstring> cp_list;

          cp_list = instances.find(w->get_lemma())->second;
          cp_list.push_back(pairs);
          instances[w->get_lemma()]= cp_list;
        }
        else{
          list<wstring> dummy_list;
          dummy_list.push_back(pairs);
          instances.insert(make_pair(w->get_lemma(),dummy_list));
        }



      }

      if(util::wstring2string(util::lowercase(w->get_tag().substr(0, 1)))=="v"){

        relations.insert(make_pair(w->get_lemma(), word_id));

      }

      if(entities.size()>1){

        for(it = entities.begin();it != entities.end();it++){

          id.push((*it).second);

        }


        for(it = relations.begin();it != relations.end();it++){
          compare = 0;
          if(id.size()>1){

            compare = id.top();
            id.pop();

            if ((*it).second > compare && (*it).second < id.top()){

                 wstringstream join_rel_id;
                 join_rel_id<<sent_id<<"."<<(*it).second;
                 Relation.name=(*it).first;
                 Relation.id = join_rel_id.str();

                 wstringstream join_subj_id;
                 join_subj_id <<sent_id<<"."<<compare;
                 Relation.subject = reverse_entities.find(compare)->second;
                 Relation.subject_id = join_subj_id.str();

                 wstringstream join_obj_id;
                 join_obj_id << sent_id<<"."<<id.top();
                 Relation.object = reverse_entities.find(id.top())->second;
                 Relation.object_id = join_obj_id.str();
                 
                 lr.push_back(Relation);


            }

            relations.erase(it);

          }
        }
      }
      word_id++;

    }

    sent_id++;
  }

  for(iterate = instances.begin(); iterate != instances.end(); iterate++){

    Entity.name = (*iterate).first;
    Entity.instances = (*iterate).second;

    le.push_back(Entity);
  }
  return;
}

