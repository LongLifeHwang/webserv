/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Kq.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: minsepar <minsepar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 11:08:58 by inghwang          #+#    #+#             */
/*   Updated: 2024/08/15 02:07:26 by minsepar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Kq.hpp"
#include "HTTPServer.hpp"

extern int logs;

Kq::Kq()
{
    struct sockaddr_in  serverAdr;
    int                 option;
    int                 serverFd;
    //Changing portNum to config port
    map<int, map<string, ServerConfigData *> > &serverConfigData = Server::serverConfig->getServerConfigData();
    map<int, map<string, ServerConfigData *> >::iterator serverConfigIt = serverConfigData.begin();
    //temp
    // int                 portNum[4] = {80, 800, 8000, 8080};

    while ((kq = kqueue()) < 0);
    option = 1;
    //여기서 루프 돌면서 server socket전부다 만들기
    while (serverConfigIt != serverConfigData.end())  //config parser
    {
        int port = serverConfigIt->first;
        while ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) <= 0);
        while (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0);
        memset(&serverAdr, 0, sizeof(serverAdr));
        serverAdr.sin_family = AF_INET;
        serverAdr.sin_addr.s_addr = htonl(INADDR_ANY);  //ip는 무조건 localhost로. config에서 에러 처리
        serverAdr.sin_port = htons(port);   //config parser
        cout << "port: " << port << endl;   
        while (::bind(serverFd, (struct sockaddr *)&serverAdr, sizeof(serverAdr)) < 0)
        {
            if (errno == EADDRINUSE)  //ip 에러를 여기서 처리할 수도...
                std::exit(1);
        }
        while (listen(serverFd, CLIENT_CNT) < 0);
        plusEvent(serverFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
        server[serverFd] = Server(serverFd, port);  //config parser
        serverConfigIt++;
    }
    std::cout<<"\nserver port open.\n";
}

Kq::Kq(const Kq& src) : kq(src.getKq()), fdList(src.getFdList()), server(src.getServer()), findServer(src.getFindServer())
{}

Kq& Kq::operator=(const Kq& src)
{
    kq = src.getKq();
    fdList = src.getFdList();
    server = src.getServer();
    findServer = src.getFindServer();
    return (*this);
}

Kq::~Kq()
{}

int Kq::getKq() const
{
    return (kq);
}

std::vector<struct kevent>  Kq::getFdList() const
{
    return (fdList);
}

std::map<int, Server>   Kq::getServer() const
{
    return (server);
}

std::map<int, int>  Kq::getFindServer() const
{
    return (findServer);
}

void    Kq::plusEvent(uintptr_t fd, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
    struct kevent   temp;

    EV_SET(&temp, fd, filter, flags, fflags, data, udata);
    fdList.push_back(temp);  //temp를 복사해서 저장을 함
}

void    Kq::plusClient(int serverFd)
{
    int clientFd;

    clientFd = server[serverFd].plusClient();
    std::cout<<"plus client"<<std::endl;
    plusEvent(clientFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
    findServer[clientFd] = serverFd;
}

void    Kq::eventRead(struct kevent& store)
{
    int     serverFd;
    EVENT   event;

    if (store.ident == 0)
        return ;
    serverFd = findServer[store.ident]; // client fd (store.ident) 이벤트 발생 fd 를 통해 server fd를 찾음
    if (serverFd == 0)
        return ;
    event = server[serverFd].clientRead(store);
    switch (event)
    {
        case ERROR:
            clientFin(store);
            break ;
        case ING:
            break ;
        case EXPECT:
            plusEvent(store.ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, 0);
            break ;
        case FINISH:
            // std::cout<<"read event delete\n";
            plusEvent(store.ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, 0);
            break ;
    }
}

void    Kq::eventWrite(struct kevent& store)
{
    int     serverFd;
    EVENT   event;

    if (store.ident == 0)
        return ;
    serverFd = findServer[store.ident];
    if (serverFd == 0)
        return ;
    event = server[serverFd].clientWrite(store);
    switch (event)
    {
        case ERROR:
            clientFin(store);
            break ;
        case ING:
            break ;
        case FINISH:
            std::cout<<"write delete\n";
            clientFin(store);
            break ;
        case EXPECT:
            EV_SET(&store, store.ident, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
            break ;
    }
}

void    Kq::mainLoop()
{
    //changing EVENTCNT to WORKER_CONNECTIONS
    const int WORKER_CONNECTIONS = Server::serverConfig->getWorkerConnections();
    struct kevent   store[WORKER_CONNECTIONS];
    int             count;

    //changed EVENTCNT to WORKER_CONNECTIONS
    while ((count = kevent(kq, &fdList[0], fdList.size(), store, WORKER_CONNECTIONS, NULL)) < 0);
    fdList.clear();
    for (int i = 0; i < count; i++)
    {
        if (server.find(static_cast<int>(store[i].ident)) != server.end())
        {
            if (store[i].flags == EV_ERROR)
                serverError(store[i]);  //server에 연결된 모든 client 종료
            else if (store[i].filter == EVFILT_READ) //read 발생시 client 추가
                plusClient(static_cast<int>(store[i].ident));
        }
        else
        {
            if (store[i].flags == EV_ERROR)
                clientFin(store[i]);  //client 종료
            else if (store[i].filter == EVFILT_READ)
                eventRead(store[i]);
            else if (store[i].filter == EVFILT_WRITE)
                eventWrite(store[i]);
        }
    }
}

void    Kq::clientFin(struct kevent& store)
{
    int     serverFd;

    // std::cout<<"error"<<std::endl;
    serverFd = findServer[store.ident];
    server[serverFd].clientFin(store.ident);
}

void    Kq::serverError(struct kevent& store)
{
    //client 모두 닫기
    //server 닫기
    Server  temp = server[store.ident];

    //server file discriptor가 에러가 나왔을 때에 연결된 클라이언트의 모든 것을 에러 처리한다.
    temp.serverError();
}
