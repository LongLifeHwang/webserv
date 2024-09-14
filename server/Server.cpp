/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: minsepar <minsepar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/08 10:56:52 by inghwang          #+#    #+#             */
/*   Updated: 2024/09/09 15:39:14 by minsepar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>
#include "Server.hpp"
#include "Response.hpp"
#include "UtilTemplate.hpp"

extern int readLog;

HTTPServer *Server::serverConfig = NULL;

Server::Server() : serverFd(0), port(0)
{}

Server::Server(int fd, int num) : serverFd(fd), port(num)
{}

Server::Server(const Server& src) : serverFd(src.getServerFd()), port(src.getPort()), cgiContentLength(src.cgiContentLength), client(src.getClient())
{}

Server&  Server::operator=(const Server& src)
{
    serverFd = src.getServerFd();
    port = src.getPort();
    client = src.getClient();
    cgiContentLength = src.getCgiContentLength();
    return (*this);
}

Server::~Server()
{}

int Server::getServerFd(void) const
{
    return (serverFd);
}

int Server::getPort(void) const
{
    return (port);
}

std::map<int, size_t>   Server::getCgiContentLength(void) const
{
    return (cgiContentLength);
}

std::map<int, Client>  Server::getClient(void) const
{
    return (client);
}

std::map<int, Client>  &Server::getClient(void)
{
    return (client);
}

bool    Server::getResponseCgi(int fd)
{
    return (client[fd].getResponseCgi());
}

ssize_t Server::getStandardTime(int fd)
{
    ssize_t   standardTime = client[fd].getStandardTime();

    if (standardTime < 0)
    {
        cout << "boom" << endl;
        return (75);
    }
    return (standardTime);
}

int setLinger(int sockfd, int linger_time) {
    struct linger linger_opt;
    linger_opt.l_onoff = 1;  // Enable linger option
    linger_opt.l_linger = linger_time;  // Set linger time in seconds

    if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt)) < 0)
    {
        throwIfError(errno, -1);  //pass
        perror("setsockopt(SO_LINGER) failed");
        return (0);
    }
    return (1);
}

int Server::plusClient(string pathEnv)
{
    int                 clntFd;
    struct sockaddr_in  clntAdr;
    socklen_t           adrSize;

    adrSize = sizeof(clntAdr);
    //accept 무한 루프 && server 동기적 실패시 무한 루프 가능성 
    if ((clntFd = accept(serverFd, (struct sockaddr *)&clntAdr, &adrSize)) < 0)
        return (-1);
    if (!setLinger(clntFd, 0))
    {
        close(clntFd);
        return (-1);
    }
    int flags = fcntl(clntFd, F_GETFL, 0);
    if (!throwIfError(errno, fcntl(clntFd, F_SETFL, flags | O_NONBLOCK)))
    {
        close(clntFd);
        return (-1);  //고민
    }
    client[clntFd] = Client(clntFd, port, pathEnv);  //생성자 및 대입 연산자 호출
	client[clntFd].clientIP(clntAdr);
    LOG(std::cout<<"New Client FD : " << clntFd <<std::endl);
    return (clntFd);
}

EVENT   Server::cgiGet(struct kevent& store)
{
	char	    buf[BUFFER_SIZE + 1];  //BUFFER_SIZE의 크기를 65536로 조절하였습니다. 
	int         readSize;

    if (store.flags & EV_ERROR)
    {
        LOG(cout << "cgiGet errno: " << store.data << endl);
        client[Kq::cgiFd[store.ident]].deleteContent();
        return (EXPECT);
    }
    LOG(cerr << "cgiGet" << endl);
    if (cgiContentLength.find(store.ident) == cgiContentLength.end())
    {
        LOG(cout << "clear setting..." << endl);
        cgiContentLength[store.ident] = 0;
        cgiContent[store.ident].clear();
    }
    readSize = read(store.ident, buf, BUFFER_SIZE);
    if (readSize < 0)
    {
        client[Kq::cgiFd[store.ident]].getResponse().setRequestStatus(500);
        client[Kq::cgiFd[store.ident]].getResponse().makeError();
        return (ERROR);
        // client[Kq::cgiFd[store.ident]].deleteContent();
        // Kq::pidPipe.erase(store.ident);
        // return (EXPECT);
    }
    buf[readSize] = '\0';
    cgiContent[store.ident].append(buf, readSize);
    cgiContentLength[store.ident] += readSize;
    LOG(cout << "store.data: " << store.data << " readSize: " << readSize << endl;)
    if (store.data - readSize <= 0)
    {
        LOG(cout << "write message setting start..." << endl);
        client[Kq::cgiFd[store.ident]].setCgiGetEntity(cgiContentLength[store.ident], cgiContent[store.ident]);
        cgiContent[store.ident].clear();
        cgiContentLength[store.ident] = 0;
        cgiContentLength.erase(store.ident);
        return (FINISH);
    }
    return (ING);
}

EVENT   Server::cgiRead(struct kevent& store)
{
	char	    buf[BUFFER_SIZE + 1];  //BUFFER_SIZE의 크기를 65536로 조절하였습니다. 
    // ssize_t     openCheck;
	int         readSize;
    // char        readBuffer[1];

    if (store.flags & EV_ERROR)
    {
        Kq::clientToCgiFd.erase(Kq::cgiFd[store.ident]);
        client[Kq::cgiFd[store.ident]].deleteContent();
        Kq::clientToCgiFd.erase(Kq::cgiFd[store.ident]);
        return (EXPECT);
    }
    LOG(cerr << "cgiRead" << endl);
	LOG(cout << "cgiRead fd: " << store.ident << endl);
    //초기 세팅
    if (cgiContentLength.find(store.ident) == cgiContentLength.end())
    {
        cgiContentLength[store.ident] = 0;
        cgiContent[store.ident].clear();
    }
    // openCheck = recv(store.ident, readBuffer, sizeof(readBuffer), MSG_PEEK);
    // throwIfError(errno, openCheck);
    // if (openCheck == 0)
    // {
    //     LOG(std::cout<<"cgiRead: client close"<<std::endl);
    //     return (ERROR);
    // }
    readSize = read(store.ident, buf, BUFFER_SIZE);
    // if (!throwIfError(errno, readSize))
    if (readSize < 0)
    {
        client[Kq::cgiFd[store.ident]].getResponse().setRequestStatus(500);
        client[Kq::cgiFd[store.ident]].getResponse().makeError();
        Kq::clientToCgiFd.erase(Kq::cgiFd[store.ident]);
        return (ERROR);
    }  //makeError 같이 고민해보기
	LOG(cout << "CGI Read Size : " << readSize << endl);
    if (readSize > 0)
    {
        buf[readSize] = '\0';
        if (static_cast<unsigned long>(store.data) > cgiContent[store.ident].size())
            cgiContent[store.ident].reserve(store.data + 1);
        cgiContent[store.ident].append(buf, readSize);
        cgiContentLength[store.ident] += readSize;
    }
    LOG(cout<<"store.data: "<< store.data<<endl);
    LOG(cout << "pidPipe: " << Kq::pidPipe[store.ident] << endl;)
    LOG(cout << "store.data: " << store.data << " readSize: " << readSize << endl;)
	if (readSize == 0)
	{
        size_t  status = 0;
        pid_t   result;
        int     waitStatus = 0;
        
        result = waitpid(Kq::pidPipe[store.ident], &waitStatus, WNOHANG);
        vector<pid_t>::iterator it = find(Kq::processor.begin(), Kq::processor.end(), Kq::pidPipe[store.ident]);
        LOG(cout << "pidPipe: " << Kq::pidPipe[store.ident] << endl;)
        if (result > 0)
        {
            status = WEXITSTATUS(waitStatus);
            if (!WIFEXITED(waitStatus) || status != 0)
                status = 600;
            LOG(std::cout<<"status: "<< status << std::endl);
            Kq::processor.erase(it);
        }
        LOG(std::cout<<"ERROR Kq::cgiFd[store.ident] : "<<Kq::cgiFd[store.ident]<<std::endl);
        client[Kq::cgiFd[store.ident]].setCgiResponseEntity(cgiContentLength[store.ident], cgiContent[store.ident], status);
        LOG(cout<<"status: "<<status<<endl);
        cgiContent[store.ident].clear();
        cgiContentLength[store.ident] = 0;
        cgiContentLength.erase(store.ident);
        Kq::clientToCgiFd.erase(Kq::cgiFd[store.ident]);
        if (status >= 400)
            return (ERROR);
        LOG(cout << "status 1: " << status << endl);
        LOG(std::cout << "msg: " << client[Kq::cgiFd[store.ident]].getMsg() << endl);
        return (FINISH);
	}
	return (ING);
}

EVENT Server::clientRead(struct kevent& store)
{
    //buffer 문제인지 생각해보기
    // ssize_t openCheck;
    char    buffer[BUFFER_SIZE * client[store.ident].getSocketReadSize() + 1];
    int     readSize;
    // char    readBuffer[1];

    //eof신호를 못 받게 됨
    if (store.flags & EV_ERROR)
    {
        LOG(cout << "clientRead errno: " << errno << endl);
        return (ERROR);
    }
    LOG(cerr << "clientRead" << endl);
    if (store.ident == 0 || client[store.ident].getFd() == 0)
        return (ING);
    // if (client[store.ident].getRequestFin() || client[store.ident].getRequestStatus() > 100)
    //     return (ING);
    // openCheck = recv(store.ident, readBuffer, sizeof(readBuffer), MSG_PEEK);
    // // throwIfError(errno);
    // if (openCheck == 0)
    // {
    //     LOG(std::cout<<"write: client close"<<std::endl);
    //     return (ERROR);
    // }
    readSize = read(store.ident, buffer, BUFFER_SIZE * client[store.ident].getSocketReadSize());
    // throwIfError(errno, readSize);
    if (readSize <= 0) // read가 발생했는데 읽은게 없다면 에러
    {
        LOG(std::cout<<"read error or socket close\n");
        client[store.ident].deleteContent();
        return (ERROR);
    }
    if (readSize == BUFFER_SIZE * client[store.ident].getSocketReadSize())
    {
        client[store.ident].plusSocketReadSize();
        LOG(cout << "buffer read size: " << BUFFER_SIZE * client[store.ident].getSocketReadSize()<<endl);
    }
    LOG(std::cout<<"Client Read " << readSize << std::endl);
    buffer[readSize] = '\0';
    write(readLog, buffer, readSize);
    client[store.ident].setMessage(buffer, readSize);
    client[store.ident].setConnection(true);
    if (client[store.ident].getRequestFin() || client[store.ident].getRequestStatus() > 0)
    {
        client[store.ident].setResponseMessage();
        // if (client[store.ident].getRequestStatus() == 100)
        //     return (EXPECT);
        //debug
        if (client[store.ident].getRequestFin())
            client[store.ident].showMessage();
        return (FINISH);
    }
    return (ING);
}

EVENT   Server::clientWrite(struct kevent& store)
{
//    std::vector<Client*>::iterator   it;
    ssize_t     openCheck;
    size_t      index;
    const char* buffer = client[store.ident].respondMsgIndex();
    char        readBuffer[1];

    if (store.flags & EV_ERROR)
        return (ERROR);
    if (store.ident == 0 || client[store.ident].getFd() == 0)
        return (ING);
    LOG(std::cout<<store.ident<<" "<<client[store.ident].responseIndex()<<std::endl);
    openCheck = recv(store.ident, readBuffer, sizeof(readBuffer), MSG_PEEK);
    if (openCheck == 0)
    {
        LOG(std::cout<<"write: client close"<<std::endl);
        return (ERROR);
    }
    if (openCheck < 0)
    {
        LOG(cout <<"openCheck error: "<< openCheck<<endl;)
        // return (ERROR);
    }
    index = write(store.ident, buffer, client[store.ident].responseIndex());
    if (index < 0 || index > client[store.ident].responseIndex())
        return (ERROR);
    // LOG(cout<<"msg: " <<buffer<<endl;)
    // write(1, buffer, client[store.ident].responseIndex());
    LOG(std::cout<<"write index: " <<index<<endl);
    client[store.ident].plusIndex(index);
    client[store.ident].setConnection(true);
    if (client[store.ident].responseIndex())
        return (ING);
    // if (client[store.ident].getRequestStatus() == 100)
    //     return (EXPECT);
    client[store.ident].deleteContent();
    if (!client[store.ident].getConnect() || client[store.ident].getResponseStatus() >= 400)
    {
        client[store.ident].resetClient();
        LOG(std::cout<<"connection fin"<<std::endl);
        LOG(cout << "Request.status: " << client[store.ident].getResponseStatus() << endl);
        return (ERROR);
    }
    client[store.ident].resetClient();
    LOG(std::cout<<"keep-alive"<<std::endl);
    return (FINISH);
}

EVENT   Server::clientTimer(struct kevent& store)
{
    bool    flag;

    if (store.flags & EV_ERROR)
        return (ERROR);
    if (store.ident == 0 || client[store.ident].getFd() == 0)
        return (ING);
    flag = client[store.ident].getConnection();
    client[store.ident].setConnection(false);
    LOG(cout << "timeout: " << getStandardTime(store.ident) * 1000<< endl);
    if (flag)
        return (ING);
    LOG(std::cout<<"TIMER OUT"<<std::endl);
    return (FINISH);
}

void    Server::clientFin(int clientFd)
{
    client[clientFd].deleteContent();
    Kq::closeFd.push_back(clientFd);
    // throwIfError(errno, close(clientFd));
    client.erase(clientFd);
}

void    Server::serverError()
{
    //이벤트 해지는 하지 않고 socket만 닫기
    std::map<int, Client>::iterator it;

    for (it = client.begin(); it != client.end(); it++)
    {
        if (it->first == 0)
            continue ;
        clientFin(it->first);
    }
    Kq::closeFd.push_back(serverFd);
    // throwIfError(errno, close(serverFd));
}
