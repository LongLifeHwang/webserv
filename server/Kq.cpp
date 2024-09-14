/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Kq.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: minsepar <minsepar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 11:08:58 by inghwang          #+#    #+#             */
/*   Updated: 2024/09/09 14:50:34 by minsepar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Kq.hpp"
#include "UtilTemplate.hpp" 
#include <signal.h>

using namespace std;

Kq::Kq()
{}

std::vector<pid_t>  processorInit()
{
    std::vector<pid_t>  v;

    return (v);
}

std::vector<struct kevent>  fdListInit()
{
    std::vector<struct kevent>  m;

    return (m);
}

std::map<int, int>  cgiFdInit()
{
    std::map<int, int>  m;

    return (m);
}

std::map<pid_t, int>    pidPipeInit()
{
    std::map<pid_t, int>    m;

    return (m);
}

std::vector<int>  closeFdInit()
{
    std::vector<int>  v;

    return (v);
}

std::map<int, int>  clientToCgiFdInit()
{
    std::map<int, int>  m;

    return (m);
}

std::vector<pid_t>          Kq::processor = processorInit();
std::vector<struct kevent>  Kq::fdList = fdListInit();
std::map<int, int>          Kq::cgiFd = cgiFdInit();
std::map<pid_t, int>        Kq::pidPipe = pidPipeInit();
std::vector<int>            Kq::closeFd = closeFdInit();
std::map<int, int>          Kq::clientToCgiFd = clientToCgiFdInit();

Kq::Kq(string pathEnv_) : pathEnv(pathEnv_)
{
    struct sockaddr_in  serverAdr;
    int                 option;
    int                 serverFd;
    int                 port;
    //Changing portNum to config port
    map<int, map<string, ServerConfigData *> > &serverConfigData = Server::serverConfig->getServerConfigData();
    map<int, map<string, ServerConfigData *> >::iterator serverConfigIt = serverConfigData.begin();

    while ((kq = kqueue()) < 0);
    option = 1;
    //여기서 루프 돌면서 server socket전부다 만들기
    while (serverConfigIt != serverConfigData.end())  //config parser
    {
        port = serverConfigIt->first;
        while ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) <= 0);
        while (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0);
        memset(&serverAdr, 0, sizeof(serverAdr));
        serverAdr.sin_family = AF_INET;
        serverAdr.sin_addr.s_addr = htonl(INADDR_ANY);  //ip는 무조건 localhost로. config에서 에러 처리
        serverAdr.sin_port = htons(port);   //config parser
        while (::bind(serverFd, (struct sockaddr *)&serverAdr, sizeof(serverAdr)) < 0)
        {
            if (errno == EADDRINUSE)  //port 번호가 같다면....
            {
                LOG(cout<<"port error"<<endl;)
                std::exit(1);
            }
        }
        //fcntl temp
        int flags = fcntl(serverFd, F_GETFL, 0);
        throwIfError(errno, flags);
        throwIfError(errno, fcntl(serverFd, F_SETFL, flags | O_NONBLOCK));  //처리하지 않기
        while (listen(serverFd, SOMAXCONN) < 0);
        plusEvent(serverFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
        server[serverFd] = Server(serverFd, port);  //config parser
        serverConfigIt++;
    }
    LOG(std::cout<<"server port open.\n");
    connectionCnt = Server::serverConfig->getWorkerConnections();
}

Kq::Kq(const Kq& src) : kq(src.getKq()), connectionCnt(Server::serverConfig->getWorkerConnections()), pathEnv(src.pathEnv), server(src.getServer()), findServer(src.getFindServer())
{}

Kq& Kq::operator=(const Kq& src)
{
    kq = src.getKq();
    connectionCnt = Server::serverConfig->getWorkerConnections();
    pathEnv = src.pathEnv;
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

std::map<int, Server>   Kq::getServer() const
{
    return (server);
}

std::map<int, int>  Kq::getFindServer() const
{
    return (findServer);
}

void    Kq::clientFin(struct kevent& store)
{
    int     serverFd;

    LOG(std::cout<<"bye"<<std::endl);
    serverFd = findServer[store.ident];
    plusEvent(store.ident, EVFILT_TIMER, EV_DELETE, 0, 0, 0);
    // plusEvent(store.ident, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
    // plusEvent(store.ident, EVFILT_READ, EV_DELETE, 0, 0, 0);
    if (serverFd == 0)
        return ;
    findServer[store.ident] = 0;
    server[serverFd].clientFin(store.ident);
}

void    Kq::serverError(struct kevent& store)
{
    //client 모두 닫기
    //server 닫기
    Server  temp = server[store.ident];

    temp.serverError();
}

void    Kq::plusEvent(uintptr_t fd, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
    struct kevent   temp;

    if (fd <= 0)
        return ;
    EV_SET(&temp, fd, filter, flags, fflags, data, udata);
    fdList.push_back(temp);  //temp를 복사해서 저장을 함
}

void    Kq::plusClient(int serverFd)
{
    int clientFd;

    clientFd = server[serverFd].plusClient(pathEnv);
    if (clientFd < 0)
        return ;
    LOG(std::cout<<"plus client: "<<clientFd<<std::endl);
    plusEvent(clientFd, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, 75000, 0);  //75초 default값
    plusEvent(clientFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
    findServer[clientFd] = serverFd;
}

void    Kq::eventRead(struct kevent& store)
{
    int     serverFd;
    EVENT   event;
	map<int, int>::iterator iter = cgiFd.begin();


    LOG(cerr << "event read" << endl);
    LOG(cout << "event read" << endl);
    if (store.ident == 0)
        return ;
	//vector에 담기 혹은 다른 방안 생각하기 serverFd가 map일 텐데 이거를 pipe일 경우에 value값을 1로
	while (iter != cgiFd.end())
	{
		if (iter->first == static_cast<int>(store.ident))
			break;
		iter++;
	}
	if (iter != cgiFd.end())
	{
        LOG(std::cout<<"READ EVENT CGI: "<<store.ident<<endl;)
		serverFd = findServer[cgiFd[store.ident]]; // client fd (store.ident) 이벤트 발생 fd 를 통해 server fd를 찾음
		if (serverFd == 0)
        {
            LOG(std::cout<<"No enroll cgi: "<<store.ident << std::endl);
            if (clientToCgiFd.find(cgiFd[iter->first]) != clientToCgiFd.end())
                clientToCgiFd.erase(cgiFd[iter->first]);
            cgiFd.erase(iter->first);
            plusEvent(store.ident, EVFILT_READ, EV_DELETE, 0, 0, 0);
            closeFd.push_back(store.ident);
			return ;
        }
        if (pidPipe[iter->first] == -1)
            event = server[serverFd].cgiGet(store);
        else
            event = server[serverFd].cgiRead(store);
		switch (event)
		{
			case EXPECT:
                plusEvent(store.ident, EVFILT_READ, EV_DELETE, 0, 0, 0);
                plusEvent(cgiFd[store.ident], EVFILT_TIMER, EV_DELETE, 0, 0, 0);
                closeFd.push_back(store.ident);
                closeFd.push_back(cgiFd[store.ident]);
                // clientToCgiFd.erase(cgiFd[store.ident]);
                cgiFd.erase(store.ident);
                pidPipe.erase(store.ident);
                break ;
			case ING:
                LOG(cout << "[Server::eventRead] - (ING, EXPECT) FD: " << iter->first << endl;)
				break ;
			case ERROR:
                LOG(cout<<"ERROR CLOSE"<<endl);
                LOG(cout << "[Server::eventRead] - (ERROR) FD: " << iter->first << endl;)
                LOG(std::cout<<"first CGI Error: "<<iter->first<<std::endl);
                plusEvent(store.ident, EVFILT_READ, EV_DELETE, 0, 0, 0);
                closeFd.push_back(store.ident);
                // clientToCgiFd.erase(cgiFd[store.ident]);  //지우기
                cgiFd.erase(store.ident);
                pidPipe.erase(store.ident);
                break ;
			case FINISH:
                LOG(cout<<"FINISH CLOSE"<<endl);
                LOG(cout << "[Server::eventRead] - (FINISH) FD: " << iter->first << endl;)
                LOG(std::cout<<"CGI Finish: "<<iter->first<<std::endl);
                plusEvent(cgiFd[store.ident], EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, 0);
                // clientToCgiFd.erase(cgiFd[store.ident]);  //지우기
				cgiFd.erase(store.ident);
                plusEvent(store.ident, EVFILT_READ, EV_DELETE, 0, 0, 0);
                closeFd.push_back(store.ident);
                pidPipe.erase(store.ident);
                // throwIfError(errno, close(store.ident));
				break ;
		}
	}
	else
	{
        LOG(std::cout<<"READ EVENT CLIENT "<<store.ident<<endl);
		serverFd = findServer[store.ident]; // client fd (store.ident) 이벤트 발생 fd 를 통해 server fd를 찾음
		if (serverFd == 0)
        {
            LOG(std::cout<<"No enroll read: "<<store.ident << std::endl);
			return ;
        }
		event = server[serverFd].clientRead(store);
		switch (event)
		{
			case ERROR:
                plusEvent(store.ident, EVFILT_READ, EV_DELETE, 0, 0, 0);
				clientFin(store);
				break ;
			case ING:
				break ;
			case EXPECT:
			case FINISH:
                if (!server[serverFd].getResponseCgi(store.ident) && findServer[store.ident] != 0)
                {
                    plusEvent(store.ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, 0);
                }
				plusEvent(store.ident, EVFILT_TIMER, EV_DELETE, 0, 0, 0);
				plusEvent(store.ident, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, server[serverFd].getStandardTime(store.ident) * 1000, 0);  //75초
                plusEvent(store.ident, EVFILT_READ, EV_DELETE, 0, 0, 0);
                LOG(std::cout << "keep-alive: "<<server[serverFd].getStandardTime(store.ident) << endl);
				break ;
		}
	}
}

void    Kq::eventWrite(struct kevent& store)
{
    int     serverFd;
    EVENT   event;

    LOG(cerr << "event write" << endl;)
    LOG(std::cout<<"event write"<<std::endl);
    if (store.ident == 0)
        return ;
    serverFd = findServer[store.ident];
    if (serverFd == 0)
    {
        LOG(std::cout<<"No enroll write: "<<store.ident << std::endl);
        return ;
    }
    event = server[serverFd].clientWrite(store);
    switch (event)
    {
        case ING:
            break ;
        case ERROR:
            plusEvent(store.ident, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
            clientFin(store);
            break ;
        case FINISH:
        case EXPECT:
            plusEvent(store.ident, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
            plusEvent(store.ident, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
            break ;
    }
}

void    Kq::eventTimer(struct kevent& store)
{
    int                             serverFd;
    EVENT                           event;
    std::map<int, int>::iterator    itClient;
    std::map<int, int>::iterator    itPid;

    LOG(cerr << "event timer" << endl;)
    LOG(cout << "event timer" << endl);
    serverFd = findServer[store.ident];
    if (serverFd == 0)
        return ;
    LOG(cout << "timer fd: " << store.ident << endl);
    event = server[serverFd].clientTimer(store);
    switch (event)
    {
        case EXPECT:
        case ING:
            break ;
        case ERROR:
            itClient = clientToCgiFd.find(store.ident);
            if (itClient != clientToCgiFd.end())
            {
                itPid = pidPipe.find(clientToCgiFd[store.ident]);
                if (itPid != pidPipe.end())
                {
                    kill(pidPipe[clientToCgiFd[store.ident]], SIGKILL);
                    // waitpid(pidPipe[clientToCgiFd[store.ident]], NULL, 0);
                    LOG(cout << "kill" << endl);
                    plusEvent(clientToCgiFd[store.ident], EVFILT_READ, EV_DELETE, 0, 0, 0);
                    Kq::processor.push_back(clientToCgiFd[store.ident]);
                    Kq::closeFd.push_back(clientToCgiFd[store.ident]);
                    pidPipe.erase(clientToCgiFd[store.ident]);
                    cgiFd.erase(clientToCgiFd[store.ident]);
                    clientToCgiFd.erase(store.ident);
                }
                cgiFd.erase(clientToCgiFd[store.ident]);
                clientToCgiFd.erase(store.ident);
            }
            clientFin(store);
            break ;
        case FINISH:
            LOG(cout << "timeOut Finish" << endl);
            // Response&                       response = server[serverFd].getClient()[store.ident].getResponse();
            itClient = clientToCgiFd.find(store.ident);
            if (itClient != clientToCgiFd.end())
            {
                LOG(cout << "pidPipe[clientToCgiFd[store.ident]]: " << pidPipe[clientToCgiFd[store.ident]] << endl);
                itPid = pidPipe.find(clientToCgiFd[store.ident]);
                if (itPid != pidPipe.end())
                {
                    kill(pidPipe[clientToCgiFd[store.ident]], SIGKILL);
                    // waitpid(pidPipe[clientToCgiFd[store.ident]], NULL, 0);
                    LOG(cout << "kill" << endl);
                    plusEvent(clientToCgiFd[store.ident], EVFILT_READ, EV_DELETE, 0, 0, 0);
                    LOG(cout << "clientToCgiFd[store.ident]: " << clientToCgiFd[store.ident] << endl);
                    Kq::closeFd.push_back(clientToCgiFd[store.ident]);
                    pidPipe.erase(clientToCgiFd[store.ident]);
                    cgiFd.erase(clientToCgiFd[store.ident]);
                    clientToCgiFd.erase(store.ident);
                    server[serverFd].getClient()[store.ident].getResponse().setRequestStatus(408);
                    LOG(cout << "time out make error" << endl);
                    server[serverFd].getClient()[store.ident].getResponse().makeError();
                    // plusEvent(store.ident, EVFILT_TIMER, EV_DELETE, 0, 0, 0);
                    break ;
                }
                cgiFd.erase(clientToCgiFd[store.ident]);
                clientToCgiFd.erase(store.ident);
            }
            clientFin(store);
            break ;
    }
}

void    Kq::mainLoop()
{
    // std::vector<pid_t>  notFin;
    struct kevent       store[connectionCnt];
    int                 count;
    int                 status;

    //waitpid(complete) 복사 생성자를 없앰 다만 erase를 진행할 때의 오히려 비용이 조금 더 들 수도 있을 수도 있다. 
    LOG(cout << "Kq::processor size: " << Kq::processor.size() << endl;)
    for (std::vector<pid_t>::iterator it = Kq::processor.begin(); it != Kq::processor.end();)
    {
        pid_t pid = waitpid(*it, &status, WNOHANG);
        if (pid <= 0)
            it++;
        else if (pid > 0)
            it = Kq::processor.erase(it);
    }
    LOG(cout << "connectionCnt: " << connectionCnt << endl);
    if ((count = kevent(kq, &fdList[0], fdList.size(), store, connectionCnt, NULL)) <= 0)
    {
        LOG(cout << "ERROR or kevent Zero: " << count << endl;)
        return ;
    }
    fdList.clear();
    LOG(cout << "count: " << count << endl;)
    for (int i = 0; i < count; i++)
    {
        if (server.find(static_cast<int>(store[i].ident)) != server.end())
        {
            LOG(cout << "server: " << store[i].ident << endl;)
            if (store[i].flags == EV_ERROR)
            {
                LOG(cerr << "server EV_ERROR" << endl);
                serverError(store[i]);  //server에 연결된 모든 client 종료
            }
            else if (store[i].filter == EVFILT_READ) //read event(complete)
                plusClient(static_cast<int>(store[i].ident));
        }
        else
        {
            LOG(cout << "client: " << store[i].ident << endl);
            LOG(std::cout<<"store[i].ident: "<<store[i].ident<<std::endl);
            if (store[i].flags & EV_ERROR) // 오류 플래그 확인
            {
                LOG(std::cerr << "Event error: " << strerror(store[i].data) << std::endl);
                LOG(cerr << "store[i].flags: " << store[i].flags << endl);
            }
            if (store[i].filter == EVFILT_READ)
            {
                eventRead(store[i]);
            }
            else if (store[i].filter == EVFILT_WRITE)
            {
                eventWrite(store[i]);
            }
            else if (store[i].filter == EVFILT_TIMER)
            {
                eventTimer(store[i]);
            }
        }
    }
    if (Kq::closeFd.empty())
        return ;
    for (std::vector<int>::iterator it = Kq::closeFd.begin(); it != Kq::closeFd.end(); it++)
        close(*it);
    Kq::closeFd.clear();
}
