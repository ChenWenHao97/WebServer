CC = g++
DEBUG = -g
RELEASE = -O2 #优化
LIBRARY = -lpthread
FILE = webserver

all: 
	${CC} main.cpp -o ${FILE} ${DEBUG} ${LIBRARY}
release:
	${CC} main.cpp -o ${FILE} ${RELEASE} ${LIBRARY}


.PHONY:clean
clean:
	-rm ${FILE}
