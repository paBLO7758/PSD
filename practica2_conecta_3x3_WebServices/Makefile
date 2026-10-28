#SSL_LIBS=-lssl -lcrypto
#SSL_FLAGS=-DWITH_OPENSSL

SSL_LIBS=
SSL_FLAGS=

all: soapC.c client server

soapC.c: conecta3x3.h
	soapcpp2 -b -c conecta3x3.h

client: soapC.c
	gcc $(SSL_FLAGS) -o client client.c soapC.c soapClient.c utils.c game.c -lgsoap $(SSL_LIBS) -L$(GSOAP_LIB) -I$(GSOAP_INCLUDE)

server: soapC.c
	gcc $(SSL_FLAGS) -o server server.c soapC.c soapServer.c utils.c game.c -lgsoap -lpthread $(SSL_LIBS) -L$(GSOAP_LIB) -I$(GSOAP_INCLUDE)

clean: 	
	rm -f client server *.xml *.nsmap *.wsdl *.xsd soapStub.h soapServerLib.* soapH.h soapServer.* soapClientLib.* soapClient.* soapC.*