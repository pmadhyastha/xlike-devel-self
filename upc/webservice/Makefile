
#### Change these two paths to match your local installation
#FLDIR=/home/padro/FreeLing-proves/FL-3.0
FLDIR=/home/pranava/local/xlike/freeling

#WSODIR=/home/padro/FreeLing-build/xlike/wso2
WSODIR=/home/pranava/local/xlike/wso2


AXISDIR=$(WSODIR)/include/axis2-1.6.0

WSOINCL=-I$(WSODIR)/include -I$(AXISDIR) -I$(AXISDIR)/platforms -L$(WSODIR)/lib 
FLINCL=-I$(FLDIR)/include -L$(FLDIR)/lib
WSOLIBS=-laxutil -laxis2_axiom -laxis2_parser -laxis2_engine -lwso2_wsf -lpthread -laxis2_http_sender -laxis2_http_receiver

COMP=g++
CXXFLAGS=-Wall -fPIC

LANG=ca en es

all: libraries client install

.PHONY: all

libraries: 
	@for lg in $(LANG); do \
	  echo "Compiling libxlike_freeling_$$lg.so..."; \
	  $(COMP) $(CXXFLAGS) -shared -olibxlike_freeling_$$lg.so xlike_freeling.cpp IO_handler.cpp Extract_ER.cpp $(WSOINCL) $(FLINCL) $(WSOLIBS) -lfreeling -DLANG=$$lg; \
	done

client: client.cc
	$(COMP) $(CXXFLAGS) -o client client.cc $(WSOINCL) $(WSOLIBS) -ldl -Wl,--rpath -Wl,$(WSODIR)/lib 

install: 
	@for lg in $(LANG); do \
	  echo "Installing service xlike_freeling_$$lg..."; \
	  mkdir -p $(WSODIR)/services/xlike_freeling_$$lg; \
	  cat services.xml | sed 's/LANG/'$$lg'/g' > $(WSODIR)/services/xlike_freeling_$$lg/services.xml; \
	  cat xlike_freeling.wsdl | sed 's/LANG/'$$lg'/g' > $(WSODIR)/services/xlike_freeling_$$lg/xlike_freeling_$$lg.wsdl; \
	  cp $$lg.cfg $(WSODIR)/services/xlike_freeling_$$lg/analyzer.cfg; \
	  cp libxlike_freeling_$$lg.so $(WSODIR)/services/xlike_freeling_$$lg; \
	done

clean:
	rm -f libxlike_freeling_*.so client

