all : lsyslog.cpp client.cpp  
	g++  -std=c++11 -c -Wall -Wextra -Werror -fpic lsyslog.cpp 
	g++  -shared -std=c++11  -o libsyslog.so -Wall -Wextra -Werror lsyslog.o
	g++ -std=c++11 -Wall -Wextra -Werror -o server server.cpp
	g++ -std=c++11 -o test client.cpp -L. -lsyslog
