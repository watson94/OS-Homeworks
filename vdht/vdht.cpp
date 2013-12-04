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
void check_null(T buf) {
    if (buf == NULL) {
        perror("Allocate error\n");
        exit(2);
    }
}

void check_write(int w) {
    if (w < 0) {
        perror("Write error\n");
        exit(3);
    }
}

void check_read(int r) {
    if (r < 0) {
        perror("Read error\n");
        exit(4);
    }
}



template <class T>
int contains  (std::vector<T> & v, T instance) {
    for (size_t i = 0; i < v.size(); i++) {
        if (v[i] ==  instance) {
            return i;
        }
    }
    return -1;
}



size_t get_next(std::vector<char>  buf, int pos) {
    int i = pos;
    while ((buf[i] != ' ') && (buf[i] != 0) && (buf[i] != '\n')) {
        i++;
    }
    return i;
}

void write_str_to_buffer(std::string s1, std::vector<char> & b) {
    for (size_t i = 0; i < s1.size(); i++) {
        b.push_back(s1[i]);
    }
}


void write_c_to_buffer(std::string s1, std::string s2, std::string s3, std::vector<char> & b) {
    write_str_to_buffer("c ", b);
    write_str_to_buffer(s1, b);
    write_str_to_buffer(" ", b);
    write_str_to_buffer(s2, b);
    write_str_to_buffer(" ", b);
    write_str_to_buffer(s3, b);
    write_str_to_buffer("\n", b);
}

void write_g_to_buffer(std::string s1, std::vector<char> & b) {
    write_str_to_buffer("g ", b);
    write_str_to_buffer(s1, b);
    write_str_to_buffer("\n", b);
}

int main(int argc, char ** argv) {

    const size_t buffersize = 4096;
    char * buffer = (char *) malloc(buffersize);
    check_null(buffer);

    if (argc < 2) {
        perror("Wrong number of arguments\n");
        exit(3);
    }
    std::map<std::string ,  std::vector<std::string > > log;
    std::vector<std::vector< char > > bufread;
    std::vector<std::vector< char > > bufwrite;
    char * myport = argv[1];

    int numneighbours = argc - 2;
    int activefds[numneighbours];
    int numafds = 0;
    char ** neighbours = (char **) malloc( numneighbours * sizeof(char *));
    check_null(neighbours);
    for (int i = 2; i < argc; i++) {
        neighbours[i - 2] = argv[i];
    }

    std::vector<struct pollfd> pollfds(2);	

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
            return 4;
    }

    pollfds[0].fd = sfd;
    pollfds[0].events = POLLIN | POLLERR;

    pollfds[1].fd = 0;
    pollfds[1].events = POLLIN | POLLERR;
    std::vector<char> t;
    bufread.push_back(t);
    bufwrite.push_back(t);
    bufread.push_back(t);
    bufwrite.push_back(t);

    
    for (int i = 0; i < numafds; i++) {
        struct pollfd tmp;
        tmp.fd = activefds[i];
        tmp.events = POLLIN | POLLOUT | POLLERR| POLLHUP;
        pollfds.push_back(tmp);
        std::vector<char> tm;
        bufread.push_back(tm);
        bufwrite.push_back(tm);
    }


        
    int optval = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    
    if ( (bind(sfd, res->ai_addr, res->ai_addrlen)) == -1) {
        perror("error bind");
        return 5;
    } 
    if (listen(sfd, 5) == -1) {
       perror("error listen");
       return 6;
    }

    while (1) {
        for (size_t i = 2; i < pollfds.size(); i++) {
            pollfds[i].events = POLLERR | POLLHUP | POLLIN;
            if (bufwrite[i].size()> 0) {
                pollfds[i].events |= POLLOUT;
                pollfds[i].revents = 0;
            }
        }
//        std::cout << "Before poll" << std::endl;
        int polldata = poll(pollfds.data(), pollfds.size(), -1);
//        std::cout << "After poll" << std::endl;

        if (polldata < 0) {
            printf("It cant be\n");
            exit(7);
        }
        if ((pollfds[0].revents & POLLERR) || (pollfds[1].revents & POLLERR)) {
            printf("I dead POLLERR\n");
            exit(7);
        }

        if (pollfds[0].revents & POLLIN) {
            printf("Connecting to new client...\n");
            int cfd = accept(sfd, NULL, NULL);
            if (cfd == -1) {
                exit(9);
            }

            struct pollfd tmp;
            tmp.fd = cfd;
            tmp.events = POLLIN | POLLOUT | POLLERR | POLLHUP;
            pollfds.push_back(tmp);
            std::vector<char> tm;
            bufread.push_back(tm);
            bufwrite.push_back(tm);
            printf("Connected to new client\n");
        }
        for (size_t i = 2; i < pollfds.size(); i++) {
            /*if (pollfds[i].revents & POLLOUT) {
                std::cout << "POLLOUT" <<std::endl;
            }
            if(pollfds[i].revents & POLLIN) {
                std::cout << "POLLIN" << std::endl;
            }
            if (pollfds[i].revents & POLLERR) {
                std::cout << "POLLOUT" <<std::endl;
            }
            if (pollfds[i].revents & POLLHUP) {
                std::cout << "POLLHUP" <<std::endl;
            }*/
            if ((pollfds[i].revents & POLLOUT) && (bufread.size() > 0)) {
                std::cout << "I will write to : " << pollfds[i].fd << std::endl;
                std::cout << "buffer : " << bufread[i].size() << std::endl;
                size_t len = buffersize;
                if (bufwrite[i].size() < len) {
                    len = bufwrite[i].size();
                }
                int w = write(pollfds[i].fd, bufwrite[i].data() , len);
                check_write(w);
                std::vector<char> tmp;   
                for (size_t j = w; j < bufwrite[i].size(); j++) {
                    tmp.push_back(bufwrite[i][j]);
                }
                bufwrite[i] = tmp;
            }
        }
        for (size_t i = 1; i < pollfds.size(); i++) {
            if (pollfds[i].revents & POLLIN) {
                int r = read(pollfds[i].fd, buffer, buffersize);
                check_read(r);
                for (int j = 0; j < r; j++) {
                    bufread[i].push_back(buffer[j]);
                }
                int posn = -1;
                for (size_t j = 0; j < bufread[i].size(); j++) {
                    if (bufread[i][j] == '\n') {
                        posn = j;
                        break;
                    }
                }
                if (posn == -1) {
                    continue;
                }
                std::string keystring;
                std::string oldv;
                std::string newv;
                size_t curpos = 2;
                size_t pos = get_next(bufread[i], curpos);
                for (size_t j = curpos; j < pos; j++) {
                    keystring.push_back(bufread[i][j]);
                }
                curpos = pos + 1;
                


                pos = get_next(bufread[i], curpos);
                for (size_t j = curpos; j < pos; j++) {
                    oldv.push_back(bufread[i][j]);
                }
                curpos = pos + 1;

                if (bufread[i][0] == 'c') {
                    pos = get_next(bufread[i], curpos);
                    for (size_t j = curpos; j < pos; j++) {
                        newv.push_back(bufread[i][j]);
                    }
                    int posold = -1;
                    std::cout << "oldv: " << oldv << std::endl;
                    if ( (posold = contains(log[keystring], oldv)) >= 0) {
                        std::cout << "I found old record" << std::endl;
                        std::cout << "posold: " << posold << std::endl;
                        std::cout << "size: " << log[keystring].size() << std::endl;
                        if ((posold == (int) (log[keystring].size() - 2)) && (log[keystring].back() == newv) ) {
                            std::cout << "Recieved. We have this information." << std::endl;
                            //all is ok
                            //no write
                        } else {
                            if (posold == (int) (log[keystring].size() - 1)) {
                                std::cout << "Recieved. Changing log..." << std::endl;
                                log[keystring].push_back(newv);
                                for (size_t k = 2; k < pollfds.size(); k++) {
                                    write_c_to_buffer(keystring, oldv, newv, bufwrite[k]);
                                }
                            } else {
                                std::cout << "Collision occured" << std::endl;
                                std::string tmp;
                                log[keystring].push_back(tmp);
                                for (size_t k = 2; k < pollfds.size(); k++) {
                                    write_g_to_buffer(keystring, bufwrite[k]);
                                }
                            }
                        }
                    } else {
                        std::cout << "I creating new record" << std::endl;
                        log[keystring].push_back(oldv);
                        log[keystring].push_back(newv);
                        for (size_t k = 2; k <pollfds.size(); k++) {
                            write_c_to_buffer(keystring, oldv, newv, bufwrite[k]);
                        }
                    }
                            
                } else 
                if (bufread[i][0] == 'g') {
                    if (!log[keystring].back().empty()) {
                         std::string tmp;
                         log[keystring].push_back(tmp);
                         for (size_t k = 2; k < pollfds.size(); k++) {
                            write_g_to_buffer(keystring,bufwrite[k]);
                         }
                    }
                } else
                if (bufread[i][0] == 'p') {
                    std::cout << keystring << std::endl;
                    for (size_t j = 0; j < log[keystring].size(); j++) {
                        std::cout << log[keystring][j] << std::endl ;
                    }
                } else {
                    printf("Wrong command\n");
                }

                std::vector<char> tmp;
                for(size_t j = posn + 1; j < bufread[i].size(); j++) {
                    tmp.push_back(bufread[i][j]);
                } 
                bufread[i] = tmp;

            }// if
        }//for 
    }//while
}
