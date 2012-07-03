
#include <stdio.h>
#include <ServiceClient.h>
#include <OMElement.h>
#include <iostream>
#include <WSFault.h>
#include <Environment.h>

using namespace std;
using namespace wso2wsf;
 
int main(int argc, char *argv[]) {

    // initialize log
	Environment::initialize("freeling_client.log", AXIS2_LOG_LEVEL_TRACE);

    // connect to servers	
 	ServiceClient *server_ca = new ServiceClient("http://localhost:9090/axis2/services/xlike_freeling_ca");
 	ServiceClient *server_en = new ServiceClient("http://localhost:9090/axis2/services/xlike_freeling_en");
 	ServiceClient *server_es = new ServiceClient("http://localhost:9090/axis2/services/xlike_freeling_es");
 
    cout<<"---------------------------------"<<endl;
    cout<<"Input text?"<<endl;
    string line;
    while (getline(cin,line)) {

      // build a request message for "ident" operation
      OMElement *cmd = new OMElement("ident");
      OMElement *text = new OMElement(cmd,"text");
      text->setText(line);
      
      // send the request and get response
      string lang;
      try {
        OMElement* response = server_en->request(cmd, "");
        if (response) {
          lang = response->getChildElement("language",NULL)->getText();
          cout<<"== LANG IDENT reply received =="<<endl;
          cout<<"     language = "<<lang<<endl;
          cout<<endl;
        }
      }
      catch (AxisFault & e) {
        if (server_en->getLastSOAPFault())
          cout << endl << "Fault: " << server_en->getLastSOAPFault() << endl;
        else
          cout << endl << "Error: " << e << endl;
      }
      
      delete cmd;
      
      // use the right server according to detected language
      ServiceClient *sc=NULL;
      if (lang=="ca") sc=server_ca;
      else if (lang=="en") sc=server_en;
      else if (lang=="es") sc=server_es;
      else
        cout<<"Analyzer not available for language '"<<lang<<"'"<<endl;
      
      if (sc) {
        string outputf;
        cout<<"Expected output (splitted, tagged, parsed) ?"<<endl;
        getline(cin,outputf);

        // build a request message for "analyze" operation
        cmd = new OMElement("analyze");
        OMElement *opt = new OMElement(cmd,"analysisOptions");
        OMElement *k;
        k = new OMElement(opt,"input"); k->setText("text");
        k = new OMElement(opt,"output"); k->setText(outputf);
        k = new OMElement(opt,"conll"); k->setText("true");
        k = new OMElement(opt,"nec"); k->setText("false");
        text = new OMElement(cmd,"text");
        text->setText(line);
        
        // send message and get response
        try {
          OMElement* response = sc->request(cmd, "");
          if (response) {
            cout<<"== ANALYZER reply received =="<<endl;
            cout<<response->getChildElement("analysis",NULL)->getText()<<endl;
            OMElement* conll = response->getChildElement("conll",NULL);
            if (conll) {
              cout<<"<CONLL>"<<endl;
              cout<<conll->getText()<<endl;
              cout<<"</CONLL>"<<endl;
            }
            cout<<endl;
          }
        }
        catch (AxisFault & e) {
          if (sc->getLastSOAPFault())
            cout << endl << "Fault: " << sc->getLastSOAPFault() << endl;
          else
            cout << endl << "Error: " << e << endl;
        }
        delete cmd;
      }

      cout<<"---------------------------------"<<endl;
      cout<<"Input text?"<<endl;
    }
}
