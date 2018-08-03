CC = g++
DEBUG = -g
RELEASE = -O2 #优化
LIBRARY = -lpthread
FILE = webserver

all: 
	${CC} main.cpp ../protocol/http_parser.c ../FastCGI/fcgi.c -o ${FILE} ${DEBUG} ${LIBRARY}
release:
	${CC} main.cpp ../protocol/http_parser.c ../FastCGI/fcgi.c -o ${FILE} ${RELEASE} ${LIBRARY}


.PHONY:clean
clean:
	-rm ${FILE}
