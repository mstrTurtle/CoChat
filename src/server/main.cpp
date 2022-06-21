/**
 * @file main.cpp
 * @author turtle (qq769711153@hotmail.com)
 * @brief Server Single File Source
 * @version 0.1
 * @date 2022-06-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <string>
#include <deque>
#include <set>
#include <iostream>
#include <memory>
#include <condition_variable>
#include <semaphore>
#include <algorithm>

//#include <nlohmann/json.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "co.hpp"

using namespace std;
//using json = nlohmann::json;
const int BUFFER_SZ = 512;


/*|*****************************│
│1│Socket s{"127.0.0.1","6666"};│
│2│Socket session = s.Accept(); │
│3│string str;                  │
│4│                             │
│5│while(!session.ReadLine(str))│
│6│    cout << str;             │
│*│*****************************/

/**
 * @brief 对底层socket的手工封装
 * 
 */
class Socket{
    public:
    Socket() = delete;
    Socket(string addr, string port){
        socket_ = execConnect(addr, port);
    }
    Socket(int s)
        : socket_(s)
    {}
    Socket(Socket const& sock){
        socket_ = sock.socket_;
    }
    Socket(Socket && sock){
        sock.moved_ = true;
        socket_ = sock.socket_;
    }
    Socket& operator=(Socket const& sock) = delete;
    Socket& operator=(Socket && sock) = delete;

    ~Socket(){
        if(!moved_)
            ::close(socket_);
    }
    void Close(){
        ::close(socket_);
    }
    int ReadLine(string& str, char delim = '\n'){
        str.clear();
        char buff[512];
        for(;;){
            ssize_t n = recv(socket_, buff, 512, 0);
            if(n<=0){
                close(socket_);
                return 1;
            }
            buff[n] = '\0';
            //printf("recv msg from client: %s\n", buff);
            auto it = find(buff, buff + n, '\n');
            if(it==buff+n){
                str.append(buff,buff+n);
            }
            else{
                str.append(buff,it+1);
                return 0;
            }
        }


/**│******************************************************│
│1 │fd_set rset;                                          │
│2 │FD_ZERO(&rset);                                       │
│3 │FD_SET(socket_, &rset);                               │
│4 │for(;;){                                              │
│5 │    select(socket_+1, &rset, NULL, NULL,NULL);        │
│6 │    if(FD_ISSET(socket_, &rset))                      │
│7 │    {                                                 │
│8 │        //cout << "Hey ReadLine\n";                   │
│9 │        int nbytes = read(socket_, buffer, 512);      │
│10│        buffer[511]='\0';                             │
│11│        cout << buffer;                               │
│12│        auto it = find(buffer, buffer + nbytes, '\n');│
│13│        if(it==buffer+nbytes){                        │
│14│            //str.append(buffer,buffer+nbytes);       │
│15│        }                                             │
│16│        else{                                         │
│17│            //str.append(buffer,it);                  │
│18│            cout << "ReadLine: " << str << '\n';      │
│19│            return;                                   │
│20│        }                                             │
│21│    }                                                 │
│22│                                                      │
│23│}                                                     │
***│******************************************************/


        
    }
    void Write(string msg, string name){
        cout << name << "Wrote: " << msg;
        if (write(socket_, msg.c_str(), msg.size()+1) != msg.size()+1) {
            fprintf(stderr, "partial/failed write\n");
            //exit(EXIT_FAILURE);
        }
    }
    void WriteLine(string msg, string name){
        cout << name << "Wrote: " << msg;
        msg.push_back('\n');
        if (write(socket_, msg.c_str(), msg.size()+1) != msg.size()+1) {
            fprintf(stderr, "partial/failed write\n");
            //exit(EXIT_FAILURE);
        }
    }
    Socket Accept(){
        int sfd, cfd;

        sfd = socket_;
        
        /* Now we can accept incoming connections one
            at a time using accept(2). */

        if (listen(sfd, 10) == -1) {
            printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
            exit(0);
        }

        if ((cfd = accept(sfd, (struct sockaddr*)NULL, NULL)) == -1) {
            printf("accept socket error: %s(errno: %d)", strerror(errno),
                   errno);
        }
        
        
        return Socket{cfd};
    }
    bool isOpen(){
        return !term;
    }
    private:
    int execConnect(string addr, string port){
        struct addrinfo hints;
        struct addrinfo *result, *rp;
        int sfd, s;
        size_t len;
        ssize_t nread;
        char buf[BUFFER_SZ];
        /* Obtain address(es) matching host/port. */

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
        hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
        hints.ai_flags = 0;
        hints.ai_protocol = 0;          /* Any protocol */

        s = getaddrinfo(addr.c_str(), port.c_str(), &hints, &result);
        if (s != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            exit(EXIT_FAILURE);
        }

        /* getaddrinfo() returns a list of address structures.
            Try each address until we successfully connect(2).
            If socket(2) (or connect(2)) fails, we (close the socket
            and) try the next address. */

        for (rp = result; rp != NULL; rp = rp->ai_next) {
            sfd = socket(rp->ai_family, rp->ai_socktype,
                    rp->ai_protocol);
            if (sfd == -1)
                continue;

            if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
                break;                  /* Success */

            close(sfd);
        }

        freeaddrinfo(result);           /* No longer needed */

        if (rp == NULL) {               /* No address succeeded */
            fprintf(stderr, "Could not bind\n");
            exit(EXIT_FAILURE);
        }

        printf("binded\n");

        /* Send remaining command-line arguments as separate
        datagrams, and read responses from server. */

        // json obj = {{"type", "hi"}};
        // string dmp = obj.dump();
        // if (write(sfd, dmp.c_str(), dmp.size()+1) != dmp.size()+1) {
        //     fprintf(stderr, "partial/failed write\n");
        //     exit(EXIT_FAILURE);
        // }

        // nread = read(sfd, buf, BUFFER_SZ);
        // if (nread == -1) {
        //     perror("read");
        //     exit(EXIT_FAILURE);
        // }

        // printf("Received %zd bytes: %s\n", nread, buf);

        return sfd;
    }
    int socket_;
    int moved_{false};
    int term{false}; // socket意外断开了
};

class Participant{
    public:
    virtual ~Participant(){}
    virtual void push(string msg) = 0;
};

typedef std::shared_ptr<Participant> PtrParticipant;

/**
 * @brief 维护一个Room，状态是其中Session的集合。
 * 
 */
class Room{
    public:
    void join(PtrParticipant p){
        participants_.insert(p);
        for(auto msg:latestMessages_){
            p->push(msg);
        }
    }
    void leave(PtrParticipant p){
        participants_.erase(p);
    }
    void broadcast(string const& msg){
        latestMessages_.push_back(msg);
        while(latestMessages_.size()>100){
            latestMessages_.pop_front();
        }
        cout << "In Broadcast. Size of participants is " << participants_.size() << '\n';
        for(auto p:participants_){
            p->push(msg);
        }
    }
    private:
    std::set<PtrParticipant> participants_;
    std::deque<std::string> latestMessages_;
};

/**
 * @brief 维护一个Session。
 * 里面的状态是Session所在Room, 
 * Session对应的TCP Socket,
 * 还有本Session待发送的消息队列。
 * 
 */
class Session 
    : public Participant,
      public std::enable_shared_from_this<Session>
{
    public:
    Session(Socket && s, Room& r)
    : s_(std::forward<Socket>(s)),
      room_(r)
    {
        cout << "new session established\n";
    }
    void start(){
        room_.join(shared_from_this());
        string str;
        cout << "Before Reading Hello\n";
        if(s_.ReadLine(str)){
            cout << "Can't read hello.\n";
            stop();
            return;
        }
        str.pop_back();
        cout << "Hello: " << str << '\n';
        room_.broadcast("[system] \""s + str + "\" joined in"s);
        name_ = str;
        thread([&]{auto r = reader();
        auto w = writer();
        r.get();
        w.get();}).detach();
    }
    void push(string msg){
        pushMessages_.push_back(msg);
        cout << "Pushed back: " << msg << '\n';
        sem_.release();
    }
    private:
    void stop(){
        room_.leave(shared_from_this());
    }

    future<void> reader(){
        std::string str;
        for(;;){
            int ret;
            co_await std::async([&] {
                ret = s_.ReadLine(str);
                if(ret) {stop();return;}
                str.pop_back(); // 把换行符丢掉
                cout << "Read: " << str << '\n';
                //sleep(1);
            });
            if(ret) break;
            room_.broadcast(name_ + ": "s + str);
        }
        co_return;
    }
    future<void> writer(){
        for(;;){
            if(!s_.isOpen()) break;
            co_await std::async([&] {
                sem_.acquire();
                std::string front = pushMessages_.front();
                cout << "Front is: " << front << '\n';
                front.push_back('\n');
                s_.Write(front,name_);
            });
            pushMessages_.pop_front();
        }
        co_return;
    }
    
    Room& room_;
    Socket s_;
    string name_;
    static std::mutex m_;
    static std::condition_variable cv_;
    static bool ready_;
    std::counting_semaphore<10086> sem_{0};
    std::deque<std::string> pushMessages_;

};

std::mutex Session::m_;
std::condition_variable Session::cv_;
bool Session::ready_ = false;
/**
 * @brief Acceptor，对Socket的封装。
 * 听一个TCP的套接字，并返回一个新套接字。
 * 
 */
class Acceptor
    : public std::enable_shared_from_this<Acceptor>
{
    public:
    Acceptor(string port)
        : s_{"127.0.0.1", port}
    {
    }
    auto asyncAccept(){
        return std::async([self=shared_from_this()] {
            return self->s_.Accept();
        });
    }
    private:
    Socket s_;
};

/**
 * @brief 异步地建立Session
 * 
 * @param port 
 * @return future<void> 
 */
future<void> listener(string port = "6666"){
    Room room;

    auto acceptor = make_shared<Acceptor>(port);
    for(;;){
        std::make_shared<Session>(
            co_await acceptor->asyncAccept(),
            room
        )->start();
    }
    co_return;
    
}

int main(int argc, char* argv[]){
    if(argc >= 2 && (!strcmp("-h",argv[1]) || !strcmp("--help",argv[1]))){
        cout << "Usage: " << argv[0] << " [<port>]\n";
        return 0;
    }
    //Socket s{argv[1],argv[2]};

    if(argc < 2)
        listener().get();
    else
        listener(argv[1]).get();

    return 0;
}