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

struct multihead_queue {
    std::vector<char> buffer;
    std::vector<size_t> pos;
    std::vector<int> cnt_ref;
    multihead_queue() {
        pos.push_back(0);
    }
    void add(std::string s) {
        for (size_t i = 0; i < s.size(); i++) {
            buffer.push_back(s[i]);
            cnt_ref.push_back(pos.size() - 1);
        }
    }
    void add_port(int port) {
        std::vector<char> tmp;
        while (port > 0) {
            char x = '0' + (port % 10);
            port = port / 10;
            tmp.push_back(x);
        }
        for (int i = tmp.size() - 1; i >= 0; i--) {
            buffer.push_back(tmp[i]);
            cnt_ref.push_back(pos.size() - 1);
        }
        return;
    }
    void add_println() {
        buffer.push_back('\n');
        cnt_ref.push_back(pos.size() - 1);
    }
    void dec_cnt_ref(size_t start, size_t end) {
        for (size_t i = start; i < end; i++) {
            cnt_ref[i]--;
        }
    }
    void update() {
        size_t i = 0;
        while ( (i < buffer.size()) && (cnt_ref[i] == 0)) {
            i++;
        }
        std::vector<char> tmp;
        std::vector<int> new_cnt;
        for (size_t j = i; j < buffer.size(); j++) {
            tmp.push_back(buffer[j]);
            new_cnt.push_back(cnt_ref[j]);
        }
        for (size_t j = 1; j < pos.size(); j++) {
            pos[j] = pos[j] - i;
        }
        cnt_ref = new_cnt;
        buffer = tmp;
    }

};


int finddelim( std::vector<char> & v, char delim) {
    for (size_t i = 0; i < v.size(); i++) {
        if (v[i] == delim) {
            return i;
        }
    }
    return -1;
}


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
//      struct sockaddr_storage sock_stor;
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
        std::vector<int> port;
        port.push_back(0);

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
            int polldata = poll(pollfds.data(), pollfds.size(), -1);
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
                socklen_t addr_size = sizeof(client);
                int cfd = accept(sd, (struct sockaddr *) &client, &addr_size);
                if (cfd == -1) {
                    exit(9);
                }

                
                std::cout << "Connected to: " << client.sin_addr.s_addr << std::endl;
                std::cout << "IP: " << (client.sin_port) << std::endl;
                port.push_back(client.sin_port);
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
                    size_t len = buffersize;
                    if ( ( mqueue.buffer.size() - mqueue.pos[i]) < len) {
                        len = mqueue.buffer.size() - mqueue.pos[i];
                    }
                    int w = write(pollfds[i].fd, mqueue.buffer.data() + mqueue.pos[i] , len);
                    check_write(w);
                    mqueue.dec_cnt_ref(mqueue.pos[i], mqueue.pos[i] + w);
                    mqueue.pos[i] += w;
                    mqueue.update();
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
                        int posn = finddelim(bufread[i], ' ');
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
                            mqueue.add_port(port[i]);

                            mqueue.add(" ");
                            mqueue.add(message);
                            mqueue.add_println();
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
