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
#include <netinet/in.h>
#include <arpa/inet.h>


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

struct multihead_queue {
    std::vector<char> buffer;
    std::vector<size_t> pos;
    std::vectot<int> cnt_ref;
    multihead_queue() {
        pos.push_back(0);
    }
    void add(std::string s) {
        for (size_t i = 0; i < s.size(); i++) {
            buffer.push_back(s[i]);
            cnt.push_back(pos.size());
        }
    }

};


int pid;
void handler(int) {
    if (pid) {
        kill(pid, SIGINT);
    }
}
int main (int argc, char ** argv) {
    if ((pid = fork())) {
         signal(SIGINT, &handler);
         wait(NULL);
         return 0;  
    } else {
        setsid();
        int status, sd;
        struct addrinfo hints, *res;
        struct sockaddr_storage sock_stor;
        if (argc != 2) {
            perror("Wrong number of argument");
            return 3;
        }
        char * myport = argv[1];
        const size_t buffersize = 4096;
        char * buffer = (char *)malloc(buffersize);
        check_null(buffer);

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;    
        hints.ai_socktype = SOCK_STREAM;    
        hints.ai_flags = AI_PASSIVE;  
 
        if ((status = getaddrinfo(NULL, myport, &hints, &res)) != 0) {
            perror("error getaddrfinfo");
            return 1;
        }
        if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
            perror("error socket");
            return 2;
        }
 
        status = 1;
        if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &status, sizeof(int)) == -1) {
            close(sd);
            perror("setsockopt fail");
            return 3;
        }
 
        if (bind(sd, res->ai_addr, res->ai_addrlen) == -1) {
                close(sd);
                perror("bind fail");
                return 4;
        }
       
        if (listen(sd, 5) == -1) {
           close(sd);
           perror("listen fail");
           return 5;
        }

        std::vector<struct pollfd> pollfds(1);
        pollfds[0].fd = sd;
        pollfds[0].events = POLLIN | POLLERR;
        std::vector<std::vector< char > > bufread;
        std::vector<char> tmp;
        bufread.push_back(tmp);
        multihead_queue mqueue;
        while (1) {
            for (size_t i = 1; i < pollfds.size(); i++) {
                pollfds[i].events = POLLERR | POLLHUP | POLLIN ;
                if (mqueue.pos[i] < mqueue.buffer.size()) {
                    pollfds[i].events |= POLLOUT;
                    pollfds[i].revents = 0;
                }
            }
            std::cout << "Before poll" << std::endl;
            int polldata = poll(pollfds.data(), pollfds.size(), -1);
            std::cout << "After poll" << std::endl;

            if (polldata < 0) {
                printf("It cant be\n");
                exit(7);
            }
            if ((pollfds[0].revents & POLLERR) ) {
                printf("I dead POLLERR\n");
                exit(7);
            }

            if (pollfds[0].revents & POLLIN) {
                printf("Connecting to new client...\n");

                sockaddr_in client;
                client.sin_family = AF_INET;
                socklen_t addr_size = sizeof(sock_stor);
                int cfd = accept(sd, (struct sockaddr *) &client, &addr_size);
                if (cfd == -1) {
                    exit(9);
                }

                
                std::cout << "Connected to: " << client.sin_addr.s_addr << std::endl;
                std::cout << "IP: " << (client.sin_port) << std::endl;
                struct pollfd tmp;
                tmp.fd = cfd;
                tmp.events = POLLIN | POLLOUT | POLLERR | POLLHUP;
                pollfds.push_back(tmp);
                std::vector<char> tm;
                bufread.push_back(tm);
                mqueue.pos.push_back(0);
                printf("Connected to new client\n");
            }
            for (size_t i = 1; i < pollfds.size(); i++) {
                if ((pollfds[i].revents & POLLOUT) && (mqueue.pos[i] < mqueue.buffer.size())) {
                    std::cout << "I will write to : " << pollfds[i].fd << std::endl;
                    size_t len = buffersize;
                    if ( ( mqueue.buffer.size() - mqueue.pos[i]) < len) {
                        len = mqueue.buffer.size() - mqueue.pos[i];
                    }
                    int w = write(pollfds[i].fd, mqueue.buffer.data() + mqueue.pos[i] , len);
                    check_write(w);
                    mqueue.pos[i] += w;
                    // check if we was min
                    /*std::vector<char> tmp;   
                    for (size_t j = w; j < bufwrite[i].size(); j++) {
                        tmp.push_back(bufwrite[i][j]);
                    }
                    bufwrite[i] = tmp; */
                }
            }
            for (size_t i = 1; i < pollfds.size(); i++) {
                if (pollfds[i].revents & POLLIN) {
                    int r = read(pollfds[i].fd, buffer, buffersize);
                    check_read(r);
                    for (int j = 0; j < r; j++) {
                        bufread[i].push_back(buffer[j]);
                    }
                    while (1) {
                        int posn = -1;
                        for (size_t j = 0; j < bufread[i].size(); j++) {
                            if (bufread[i][j] == ' ') {
                                posn = j;
                                break;
                            }
                        }
                        if (posn == -1) {
                            break;
                        }
                        std::string strsize;
                        std::string message;
                        size_t curpos = 0;
                        size_t pos = get_next(bufread[i], curpos);
                        for (size_t j = curpos; j < pos; j++) {
                            strsize.push_back(bufread[i][j]);
                        }
                        curpos = pos + 1;

                        int max_len = bufread[i].size() - posn - 1;
                        int size = stoi(strsize);
                        if (max_len >= size) {
                            for (size_t j = curpos; j < curpos + size; j++) {
                                message.push_back(bufread[i][j]);
                            }
                            curpos += size;
                            mqueue.add(message);
                        } else {
                            break;
                        }
                        
                        std::vector<char> tmp;
                        for(size_t j = curpos ; j < bufread[i].size(); j++) {
                            tmp.push_back(bufread[i][j]);
                        } 
                        bufread[i] = tmp;
                    }
                }// if
            }//for 
        }//while
    } //demon
}
