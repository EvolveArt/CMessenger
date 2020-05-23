include Makefile.inc

EXE = Server/serveur Client/client

all: ${EXE}

${EXE): ${PSE_LIB}

clean:
	rm -f *.o *~ ${EXE} journal.log

