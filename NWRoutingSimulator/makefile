CC=g++
CFLAGS=-g -std=c++0x -c -Wall
LDFLAGS=
CSOURCES=Constants.h Message.h Channel.h Controller.h Constants.cpp Message.cpp Channel.cpp Controller.cpp
COBJECTS=$(CSOURCES:.cpp=.o)
CEXECUTABLE=controller

NSOURCES=Constants.h Message.h Channel.h LinkStateRouting.h MulticastRouting.h Node.h Channel.cpp Constants.cpp LinkStateRouting.cpp Message.cpp MulticastRouting.cpp Node.cpp 
NOBJECTS=$(NSOURCES:.cpp=.o)
NEXECUTABLE=node

all: mkcontroller mknode

# ------ CONTROLLER START -----

mkcontroller: $(CSOURCES) $(CEXECUTABLE)
	
$(CEXECUTABLE): $(COBJECTS)
	$(CC) $(LDFLAGS) $(COBJECTS) -o $@

# ----- CONTROLLER END ------

#------ NODE START ---------

mknode: $(NSOURCES) $(NEXECUTABLE)

$(NEXECUTABLE): $(NOBJECTS)
	$(CC) $(LDFLAGS) $(NOBJECTS) -o $@

#------ NODE END ------

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm -rf *o $(CEXECUTABLE) $(NEXECUTABLE) ./messages/* 

