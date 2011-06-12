OBJS =	ImapLibrary/tcpUtilities.o \
			ImapLibrary/imap.o \
			ImapLibrary/basicNetwork.o \
			ImapLibrary/Folder.o \
			ImapLibrary/Message.o \

SSLFLAGS = -lssl

all : ${OBJS} 
	g++ -o imapFuse imapFuse.cpp -lfuse -D_FILE_OFFSET_BITS=64 ${OBJS} ${SSLFLAGS} -g

basicNetwork.o : ImapLibrary/tcpUtilities.h ImapLibrary/basicNetwork.h ImapLibrary/basicNetwork.cpp ImapLibrary/connection.h
	g++ -Wall -g -ftemplate-depth-30 -c ImapLibrary/basicNetwork.cpp

imap.o : ImapLibrary/basicNetwork.h ImapLibrary/imap.h ImapLibrary/imap.cpp ImapLibrary/imapLiterals.h ImapLibrary/errors.h ImapLibrary/connection.h
	g++ -Wall -g -ftemplate-depth-30 -c imap.cpp

tcpUtilities.o : ImapLibrary/tcpUtilities.h ImapLibrary/tcpUtilities.cpp ImapLibrary/connection.h
	g++ -Wall -g -ftemplate-depth-30 -c ImapLibrary/tcpUtilities.cpp

Folder.o : ImapLibrary/Folder.h ImapLibrary/Folder.cpp ImapLibrary/Message.h
	g++ -Wall -g -ftemplate-depth-30 -c ImapLibrary/Folder.cpp

Message.o : Message.h Message.cpp connection.h
	g++ -Wall -g -ftemplate-depth-30 -c ImapLibrary/Message.cpp

clean :
	rm -f ImapLibrary/imapLibrary
	rm -f ImapLibrary/*.o
	rm -f ImapLibrary/*~
	rm -f imapFuse
