#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string>
#include <thread>
#include <algorithm>

// //#include <sys/un.h>
// #include <netdb.h>
// #include <stdio.h>
// #include <stdlib.h>
#include <unistd.h>
// #include <string.h>
#include <iostream>
#include <mutex>
#include <semaphore>

using namespace std;

#define MAXLINE 4096

mutex m;


char sendline[4096];

int reciever(int sfd){
    string str;
    char buff[512];
    for(;;){
        str.clear();
        for(;;){
            ssize_t n = recv(sfd, buff, 512, 0);
            if(n<=0){
                close(sfd);
                cout << "Closed!\n";
                return 1;
            }
            buff[n] = '\0';
            //printf("recv msg from client: %sfd\n", buff);
            auto it = find(buff, buff + n, '\n');
            if(it==buff+n){
                str.append(buff,buff+n);
            }
            else{
                str.append(buff,it+1);
                str.pop_back(); // 不要反斜杠n
                {
                    lock_guard lg(m);
                    cout << "recv> " << str << '\n';
                }
                break;
            }
        }
    }
}

int sender(int sockfd){
    for(;;){
        //cout << "send> ";
        //sem_getline.acquire();
        fgets(sendline, 4096, stdin);
        if(!strncmp("\\q",sendline,2)) break;
        if (send(sockfd, sendline, strlen(sendline), 0) < 0) {
            printf("send msg error: %sfd(errno: %d)\n", strerror(errno), errno);
            exit(0);
        }
    }
    return 0;
}
int main(int argc, char** argv) {
    int sockfd, n;
    char recvline[4096], sendline[4096];
    struct sockaddr_in servaddr;
    if (argc < 2) {
        printf("usage: ./client <ipaddress> [<port>]\n");
        exit(0);
    }
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("create socket error: %sfd(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    if(argc < 3){
        servaddr.sin_port = htons(6666);
    }
    else{
        servaddr.sin_port = htons(atoi(argv[2]));
    }
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        printf("inet_pton error for %sfd\n", argv[1]);
        exit(0);
    }
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        printf("connect error: %sfd(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    printf("Type Nickname:\n");
    
    thread th_r{reciever,sockfd};
    thread th_w{sender,sockfd};
    
    th_r.join();
    th_w.join();

    close(sockfd);
    exit(0);
}