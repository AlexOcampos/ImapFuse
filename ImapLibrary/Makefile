#Changes to make compile possible on Suns and BSD boxen thanks to Thomas Strömberg


OBJS =	tcpUtilities.o \
			imap.o \
			basicNetwork.o \
			Folder.o \
			Message.o \

SSLFLAGS = -lssl
NOSSLFLAG =


PACKAGE=\"imapLibrary\"
LOCALEDIR=\"/usr/share/locale\"


#mail poll interval in ms
MAIL_POLL_INTERVAL = 60000

CFLAGS = `gtk-config --cflags` -DPACKAGE=${PACKAGE} -DLOCALEDIR=${LOCALEDIR}
DEBUGFLAGS = -Wall -g -ftemplate-depth-30 ${NOSSLFLAG}
LDADD = `gtk-config --libs` ${LDFLAGS} -lpthread

all : ${OBJS} 
	g++ ${DEBUGFLAGS} -o imapLibrary main.cpp ${SSLFLAGS} ${OBJS}

basicNetwork.o : tcpUtilities.h basicNetwork.h basicNetwork.cpp connection.h
	g++ -Wall -g -ftemplate-depth-30 -c basicNetwork.cpp

imap.o : basicNetwork.h imap.h imap.cpp imapLiterals.h errors.h connection.h
	g++ -Wall -g -ftemplate-depth-30 -c imap.cpp

tcpUtilities.o : tcpUtilities.h tcpUtilities.cpp connection.h
	g++ -Wall -g -ftemplate-depth-30 -c tcpUtilities.cpp

Folder.o : Folder.h Folder.cpp Message.h
	g++ -Wall -g -ftemplate-depth-30 -c Folder.cpp

Message.o : Message.h Message.cpp connection.h
	g++ -Wall -g -ftemplate-depth-30 -c Message.cpp

clean :
	rm -f imapLibrary
	rm -f *.o
	rm -f *~
