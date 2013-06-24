#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <mutex>
#include <time.h>
#include <poll.h>
#include <vector>
#include <utility>
#include <map>
#include <iostream>

void write_str (int  fd, char *str, int size) {
    int i = 0;
    while (i < size) {
        int write_res = write(fd, str + i, size - i);
        if (  write_res > 0) {
            i += write_res;
        }
        if (write_res == -1) {
            perror("Write error\n");
            exit(1);
        }
    }
}


template <class T>
void checkNull(T buf) {
	if (buf == NULL) {
		perror("Allocate error\n");
		exit(2);
	}
}


std::pair<int, char *> getnext(char * buf) {
	std::string str(buf);
	int pos = str.find(" ");

	char * ans = (char *) malloc(pos + 1);
	checkNull(ans);
	
	memcpy(ans, buf, pos);
	return std::make_pair(pos + 1, ans);
}


int main(int argc, char ** argv) {	
	const int buffersize = 4096;
	int len = 0;
	char * buffer = (char *) malloc(buffersize);
	checkNull(buffer);
	
	if (argc < 2) {	
		perror("Wrong number of arguments\n");
		exit(3);
	}


	int numneighbours = argc - 2;
	int numafds = 0;
	int activefds[numneighbours];	
	std::map<int, std::vector<std::string>> log;	
	

	char ** neighbours = (char **) malloc(numneighbours * sizeof(char *));
	checkNull(neighbours);
	char * myport = argv[1];
	

	std::vector<struct pollfd> pollfds(2);	

	pollfds[0].fd = 0;
	pollfds[0].events = POLLIN | POLLERR;

	for (int i = 2; i < argc; i++) {
		neighbours[i - 2] = argv[i];
	}
	
	addrinfo hints;
	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
        hints.ai_protocol = 0;
	hints.ai_canonname = nullptr;
        hints.ai_addr = nullptr;
        hints.ai_next = nullptr;
        struct addrinfo * res;


        addrinfo *result; 
	for (int i = 0; i < numneighbours; i++) {
		if (getaddrinfo(nullptr, neighbours[i], &hints, &result) != 0)
		{
		    std::exit(EXIT_FAILURE);
		 
		}
		if (result == nullptr)
		{
		     std::exit(EXIT_FAILURE);
		}
		int socketfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (connect(socketfd, result->ai_addr, result->ai_addrlen) != -1) {
			activefds[numafds] = socketfd;
			numafds++;
		}	
	}

	memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
        hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
        hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
        hints.ai_protocol = 0;          /* Any protocol */
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;
        int s = getaddrinfo(NULL, myport , &hints, &res);
        if (s != 0) {
            perror("error getaddrinfo");
            return 1;
        }
    
        int sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	
	if (sfd == -1) {
            perror("error socket");
            return 2;
        }

	pollfds[1].fd = sfd;
	pollfds[1].events = POLLIN | POLLERR;
	for (int i = 0; i < numafds; i++) {
		struct pollfd tmp;
		tmp.fd = activefds[i];
		tmp.events = POLLIN | POLLOUT | POLLERR;
		pollfds.push_back(tmp);
	}		


        
        int optval = 1;
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    
        if ( (bind(sfd, res->ai_addr, res->ai_addrlen)) == -1) {
            perror("error bind");
            return 3;
        } 
        if (listen(sfd, 5) == -1) {
            perror("error listen");
            return 4;
        }
	
	while (1) {
		int polldata = poll(pollfds.data(), pollfds.size(), -1);
		if (polldata < 0) {
			printf("It cant be\n");
			exit(7);
		}
		if ((pollfds[0].revents & POLLERR) || (pollfds[1].revents & POLLERR)) {
			printf("Im dead\n");
			exit(4);
		}
		if (pollfds[0].revents & POLLIN) {
		//	while (1) {	
				int r = read(0, buffer + len, buffersize - len);
				if (r < 0) {
					exit(7);
				}
				if (r == 0) {
					printf("Im 0\n");
					break;
				}
				len += r;
		//	}
			printf("Im read %s\n", buffer);
			len++;
			std::pair<int, char *> keystring = getnext(buffer); 
			int key = keystring.first;
			if (buffer[0] == 'p') {
				printf("Im printing\n");;
				for (size_t i = 0; i < log[key].size(); i++) {
					std::cout << log[key][i] << std::endl ;
				}
			} else
			if (buffer[0] == 'a') {
				std::pair<int, char *> next = getnext(buffer + keystring.first);
				std::string dest(next.second);
				if (log.count(key) == 0) {
					for (size_t i = 2; i < pollfds.size(); i++) {	
						if (pollfds[i].revents & POLLOUT) {
							write(pollfds[i].fd, buffer, len);
						}
					}
				}
				log[key].push_back(dest);
				std::vector<std::string> a;
			} else 
			if (buffer[0] == 'c'){
				std::pair<int, char *> next = getnext(buffer + keystring.first);
				std::pair<int, char *> nnext = getnext(buffer + next.first);
				std::string dest(next.second);
				std::string src(nnext.second);
				if (log[key].back() == dest) {
					log[key].back() = src;
					for (size_t i = 2; i < pollfds.size(); i++) {	
						if (pollfds[i].revents & POLLOUT) {
							write(pollfds[i].fd, buffer, len);
						}
					}
 				} else {
					for (size_t i = 2; i < pollfds.size(); i++) {	
						if (pollfds[i].revents & POLLOUT) {
							write(pollfds[i].fd, "G", 1);
						}
					}
				}
			} else {
				printf("Wrong command");
			}
			len = 0;
		}
		if (pollfds[1].revents & POLLIN) {
			printf("Connecting to new client...\n");
			int cfd = accept(sfd, NULL, NULL);
			if (cfd == -1) {
				exit(5);
			}
			struct pollfd tmp;
			tmp.fd = cfd;
			tmp.events = POLLIN | POLLOUT | POLLERR;
			pollfds.push_back(tmp);
			printf("Connected to new client\n");
		}
		for (size_t i = 2; i < pollfds.size(); i++) {
			if (pollfds[i].revents & POLLIN) {
				printf("pollin\n");
			//	while (1) {	
				int r = read(pollfds[i].fd, buffer, buffersize);
				if (r < 0) {
					exit(7);
				}
				if (r == 0) {
					printf("Im 0\n");
					break;
				}
				len += r;
				len++;
			//	}
				len = 0;
			}		
		}
	}
		
}
