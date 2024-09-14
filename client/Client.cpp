/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: minsepar <minsepar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/08 17:11:14 by inghwang          #+#    #+#             */
/*   Updated: 2024/09/12 15:08:17 by minsepar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "ServerConfigData.hpp"
#include "LocationConfigData.hpp"
#include "UtilTemplate.hpp"

LocationConfigData *Client::recurFindLocation(string url,
    LocationConfigData *locationConfigData)
{
    LOG(cout << "url: " << url << endl);

    LocationConfigData *configData = NULL;

    vector<string> &suffixMatch = locationConfigData->getSuffixMatch();
    for (vector<string>::iterator it = suffixMatch.begin();
        it != suffixMatch.end(); it++)
    {
        if (endsWith(url, *it))
        {
            configData = locationConfigData->getLocationConfigData(*it, 1);
            return (recurFindLocation(url, configData));
        }
    }
 
    // LOG(cout << locationConfigData->getPath() << endl);
    Trie &prefixTrie = locationConfigData->getPrefixTrie();
    string urlFound = prefixTrie.find(url);
    LOG(cout << "temp: " << urlFound << endl);
    LOG(cout << "location: " << request.location << endl);
    if (urlFound == "")
        return (locationConfigData);
    configData
        = locationConfigData->getLocationConfigData(urlFound, 0);
    size_t i = url.find(configData->getPath());
    string passURL = url.substr(i + configData->getPath().size());
    passURL = "/" + passURL;
    return (recurFindLocation(passURL, configData));
}

Client::Client() : connect(true), connection(false), fd(0), port(0), socketReadSize(1), msgSize(0), index(0), responseAmount(0), standardTime(75), startLine(0), headerLine(0), contentLine(0)
{
    request.port = port;
    request.fin = false;
    request.status = 0;
	request.clientFd = fd;
}

Client::Client(int fd, int port, string pathEnv_) : connect(true), connection(false), fd(fd), port(port), socketReadSize(1), msgSize(0), index(0), responseAmount(0), standardTime(75), startLine(port), headerLine(port), contentLine(port), pathEnv(pathEnv_)
{
    request.port = port;
    request.fin = false;
    request.status = 0;
	request.clientFd = fd;
}

Client::Client(const Client& src) : connect(src.getConnect()), connection(src.getConnection()), fd(src.getFd()), port(src.getPort()), socketReadSize(src.socketReadSize), msgSize(src.getMsgSize()), index(src.getIndex()), responseAmount(src.getResponseAmount()), standardTime(src.getStandardTime()), msg(src.getMsg()), request(src.getRequest()), startLine(src.getStartLine()), headerLine(src.getHeaderline()), contentLine(src.getContentLine()), response(src.getResponse()), pathEnv(src.pathEnv)
{}

Client& Client::operator=(const Client& src)
{
    connect = src.getConnect();
    connection = src.getConnection();
    fd = src.getFd();
    port = src.getPort();
    socketReadSize = src.socketReadSize;
    msgSize = src.getMsgSize();
    index = src.getIndex();
    responseAmount = src.getResponseAmount();
    standardTime = src.getStandardTime();
    msg = src.getMsg();
    request = src.getRequest();
    startLine = src.getStartLine();
    headerLine = src.getHeaderline();
    contentLine = src.getContentLine();
    response = src.getResponse(); // 디폴트로 불리면서 포트 설정 안됨
	pathEnv = src.pathEnv;
    return (*this);
}

Client::~Client()
{
    LOG(std::cout<<fd<<" client close"<<std::endl);
}

bool    Client::getConnect() const
{
    return (connect);
}

bool    Client::getConnection() const
{
    return (connection);
}

int Client::getFd(void) const
{
    return (fd);
}

int Client::getPort(void) const
{
    return (port);
}

int Client::getSocketReadSize(void) const
{
    return (socketReadSize);
}

size_t  Client::getMsgSize(void) const
{
    return (msgSize);
}

size_t  Client::getIndex() const
{
    return (index);
}

size_t  Client::getResponseAmount() const
{
    return (responseAmount);
}

ssize_t Client::getStandardTime() const
{
    return (standardTime);
}

std::string Client::getMsg() const
{
    return (msg.c_str() + index);
}

Request Client::getRequest() const
{
    return (request);
}

StartLine   Client::getStartLine() const
{
    return (startLine);
}

HeaderLine  Client::getHeaderline() const
{
    return (headerLine);
}

ContentLine    Client::getContentLine() const
{
    return (contentLine);
}

Response    &Client::getResponse()
{
    return (response);
}

Response    Client::getResponse() const
{
    return (response);
}

bool    Client::getRequestFin() const
{
    return (request.fin);
}

int Client::getRequestStatus() const
{
    return (request.status);
}

int Client::getResponseStatus() const
{
    return (response.getRequestStatus());
}

void    Client::setConnection(bool ycdi)
{
    this->connection = ycdi;
}

void    Client::setFd(uintptr_t fd)
{
    this->fd = fd;
}

void    Client::setRequestStatus(int temp)
{
    request.status = temp;
}

void    Client::setRequestFin(bool fin)
{
    request.fin = fin;
}

void    Client::setCgiGetEntity(size_t &cgiContentLength, string &content)
{
    response.setCgiGetHeader(cgiContentLength);
    response.setCgiGetContent(content);
    responseAmount = response.getStartHeaderLength() + cgiContentLength;
    index = 0;
    msg = response.getEntity();
}

void	Client::setCgiResponseEntity(size_t &cgiContentLength, string &content, size_t &status)
{
	size_t  pos;

    LOG(std::cout<<"cgiContentLength: "<<cgiContentLength<<std::endl;)
    pos = response.setCgiContent(content, status);
    if (status >= 400)
    {
        return ;
    }
    LOG(std::cout<<"cgi pos: "<<pos<<std::endl);
    response.setCgiContentLength(cgiContentLength - pos);
    responseAmount = response.getStartHeaderLength() + cgiContentLength - pos;
    index = 0;
    LOG(std::cout<<"responseAmount: "<<response.getStartHeaderLength() + cgiContentLength - pos<<std::endl<<endl;)
    msg = response.getEntity();
    LOG(std::cout<<"msg: "<<msg<<std::endl);
}

void    Client::plusSocketReadSize()
{
    socketReadSize++;
}

bool    Client::getResponseCgi()
{
    return (response.getCgiFlag());
}

void    Client::deleteContent(void)
{
    if (request.method == GET)
        return ;
    // std::ifstream file(request.contentFileName.c_str());

    // if (file.is_open())
    // {
    //     cout << "[Client::deleteContent] - " << file.get
    //     file.close();
    // }
    unlink(getContentLine().getFileName().c_str()); // 잘 지워주세요 To.Ingyu
}

void	Client::clientIP(struct sockaddr_in  clntAdr)
{
	char	clientIp[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &clntAdr.sin_addr, clientIp, INET_ADDRSTRLEN);
	request.clientIp = clientIp;
	// LOG(std::cout<<"client ip: "<<clientIp<<std::endl);
}

const char* Client::respondMsgIndex()
{
    return (msg.c_str() + index);
}

int Client::setStart()
{
    size_t      flag;

    if (startLine.getCompletion() || request.fin || request.status)
        return (0);
    LOG(std::cout<<"...startline parsing...\n");
    flag = msg.find("\r\n");
    if (flag != std::string::npos)
    {
        //keepa-alive
        // standardTime = Server::serverConfig->getDefaultServer(port)->getKeepaliveTimeout();  //여기서 keep-alive setting
        standardTime = Server::serverConfig->getDefaultServer(port)->getHeaderTimeout();  //여기서 keep-alive setting
        LOG(std::cout<<"standardTime: " <<standardTime<<std::endl);
        if ((request.status = startLine.check(msg.substr(0, flag))))  //ingu test
            return (1);
        msgSize -= (flag + 2);
        msg = msg.substr(flag + 2);
        request.method = startLine.getMethod();
        request.url = startLine.getUrl();
        request.location = startLine.getLocation();
        request.version = startLine.getVersion();
        request.query = startLine.getQuery();
    }
    else
    {
        if (msg.size() > 8192)
        {
            standardTime = Server::serverConfig->getDefaultServer(port)->getKeepaliveTimeout();  //여기서 keep-alive setting
            request.status = 414;
            return (2);
        }
    }
    return (0);
}

int Client::setHeader()
{
    size_t                                                    flag;
    std::string                                               str;

    if (!startLine.getCompletion() || headerLine.getCompletion() || request.fin || request.status)
        return (0);
    LOG(std::cout<<"...headerline parsing..."<<std::endl);
    while (1)
    {
        flag = msg.find("\r\n");
        if (flag != std::string::npos)
        {
            msgSize -= (flag + 2);
            if (flag == 0)
            {
                request.header = headerLine.getHeader();
                msg = msg.substr(flag + 2);
                //keep-alive
                standardTime = Server::serverConfig->getDefaultServer(port)->getKeepaliveTimeout();  //여기서 keep-alive setting
                LOG(std::cout<<"standardTime: " <<standardTime<<std::endl);
                LOG(std::cout<<"server name: "<<Server::serverConfig->getDefaultServer(port)->getServerName()[0]<<endl;)
                LOG(cout << "response in location: " << &response << endl);
                if (request.header["host"].empty())
                {
                    request.status = 400;
                    return (1);
                }
                if (setMatchingLocation(request.url))
                {
                    request.status = 404;
                    return (2);
                }
                if ((request.status = headerLine.headerError()) > 0)
                {
                    LOG(std::cout<<"default error"<<std::endl);
                    return (2);
                }
                // if (request.status > 0)
                // {
                //     LOG(std::cout<<"default error"<<std::endl);
                //     std::cout<<"default error"<<std::endl;
                //     return (2);
                // }
                request.header = headerLine.getHeader();
                contentLine.initContentLine(headerLine.getContentLength(), headerLine.getContentType());
                connect = headerLine.getConnect();
                break ;
            }
            str = msg.substr(0, flag);
            if ((request.status = headerLine.makeHeader(str)) > 0)
            {
                LOG(std::cout<<str<<std::endl);
                return (1);  //400
            }
            msg = msg.substr(flag + 2);
            if (headerLine.getHeader().size() > 24576)
            {
                request.status = 400;
                LOG(std::cout<<"header size error"<<std::endl);
                return (2);  //400
            }
        }
        else
        {
            if (msg.size() > 8192)
            {
                request.status = 414;
                return (3);  //414
            }
            break ;
        }
    }
    if (headerLine.getCompletion())
    {
        //아직 다 들어오지 않은 데이터가 있을 수도 있잔녀 이건 우선 생각하지 않음
        //데이터가 후에 들어온다고 가정한다면 그때 가서 처리를 해주면 됨 하지만 들어오지 않고 eof가 들어오면 맞는 데이터임에도 error로 처리하기 때문에 여기서 이렇게 처리하는 것이 맡다. 
        LOG(cout<<"checking..."<<endl);
        if (request.method == GET)
            request.fin = true;
        else if (msg.empty() && headerLine.getContentType() == ENOT)
            request.fin = true;
        else if (!msg.empty() && headerLine.getContentType() == ENOT)
        {
            request.status = 400;
            return (1);
        }
        else if (headerLine.getContentType() == CONTENT && headerLine.getContentLength() == 0)
        {
            //빼야 할 수도
            request.fin = true;
        }
    }
    return (0);
}

int Client::setContent()
{
    if (!headerLine.getCompletion() || contentLine.getCompletion() || request.fin || request.status)
        return (0);
    LOG(std::cout<<"...setBodyLine parsing...\n");
    // LOG(std::cout<<"msgSize: "<<msgSize<<endl);
    // LOG(std::cout<<"msg: "<<msg<<endl);
    if (contentLine.makeContentLine(msg, msgSize, request.status, fd) < 0)
        return (1);
    request.contentFileName = contentLine.getFileName();
    if (contentLine.getCompletion())
    {
        if (headerLine.getTe() == NOT)
        {
            if (!msg.empty())
            {
                LOG(cout << "getTeError: " << msg << endl);
                request.status = 400;
                return (2);
            }
            else
                request.fin = true;
        }
    }
    return (0);
}

int Client::setTrailer(void)
{
    size_t      flag;
    std::string str;

    if (!contentLine.getCompletion() || headerLine.getTe() != YES || request.fin || request.status > 0)
        return (0);
    LOG(std::cout<<"...setTrailer parsing...\n");
    while (1)
    {
        if (msg.empty())
            break ;
        flag = msg.find("\r\n");
        if (flag != std::string::npos)
        {
            str = msg.substr(0, flag);
            msg = msg.substr(flag + 2);
            if ((request.status = headerLine.parseTrailer(str)) > 0)
                return (1);
            else
            {
                request.header = headerLine.getHeader();
                if (request.header["trailer"].empty())
                {
                    headerLine.setTrailer(NOT);
                    request.fin = true;
                    break ;
                }
            }
        }
        else
        {
            if (msg.size() > 8192)
            {
                request.status = 414;
                return (2);  //414
            }
            break ;
        }
    }
    return (0);
}

void    Client::resetClient()
{
    LOG(cout<<"reset fd: "<<fd<<endl);
    request.fin = false;
    request.status = 0;
    // request.header.clear();
    // request.query.clear();
    connect = true;
    socketReadSize = 1;
    msgSize = 0;
    index = 0;
    responseAmount = 0;
    msg.clear();
    // 여기서 뭔가 ServerConfigData, LocationConfigData를 초기화해주는 기분이 듦
    startLine = StartLine(port);
    headerLine = HeaderLine(port);
    contentLine = ContentLine(port);
    response = Response(port, pathEnv);
}

void    Client::setMessage(const char* msgRequest, int &readSize)
{
    msgSize += readSize;
    msg.append(msgRequest, readSize);
    if (setStart())  //max size literal
    {
        request.header["host"].push_back(Server::serverConfig->getDefaultServer(port)->getServerName()[0]);
        request.fin = true;
        LOG(std::cout<<fd<<" "<<request.status<<" ");
        LOG(std::cout<<"Startline Error\n");
        return ;
    }
    if (setHeader())  //max size literal, 헤더 파싱
    {
        if (request.status == 100)
            return ;
        request.fin = true;
        while (!request.header["host"].empty())
            request.header["host"].pop_back();
        request.header["host"].push_back(Server::serverConfig->getDefaultServer(port)->getServerName()[0]);
        LOG(std::cout<<"Header Error\n");
        return ;
    }
    if (setContent()) // 바디 파싱
    {
        request.fin = true;
        LOG(std::cout<<"Body Error\n");
        return ;
    }
    if (setTrailer()) // 바디 마지막, 트레일러 파싱
    {
        request.fin = true;
        LOG(std::cout<<"Trailer Error\n");
        return ; 
    }
}

void    Client::setResponseMessage()
{
    msg.clear();
    index = 0;
    response.setPort(port);
	response.setPathEnv(pathEnv);
    response.initRequest(request);
    response.responseMake();
    if (!response.getCgiFlag())
    {
        LOG(std::cout<<"NOtCGI"<<std::endl);
        msg = response.getEntity();
        responseAmount = response.getStartHeaderLength() + response.getContentLength();
        return ;
    }
    LOG(std::cout<<"YES CGI"<<std::endl);
}

size_t  Client::responseIndex()
{
    if (responseAmount <= index)
        responseAmount = index;
    return (responseAmount - index);
}

void    Client::plusIndex(size_t plus)
{
    index += plus;
}

bool    Client::setMatchingLocation(string url)
{
    LOG(cout << "url " << url << endl);

    string host = request.header["host"].front();
    ServerConfigData *serverConfigData;
    try {
        serverConfigData = Server::serverConfig->getServerData(host, port);
    } catch (exception &e) {
        if (Server::serverConfig->getDefaultServer(port) == NULL)
            return (true);
        else
            serverConfigData = Server::serverConfig->getDefaultServer(port);
    }

    LocationConfigData *location = NULL;

    vector<string> &suffixMatch = serverConfigData->getSuffixMatch();
    for (vector<string>::iterator it = suffixMatch.begin();
        it != suffixMatch.end(); it++)
    {
        if (endsWith(url, *it))
        {
            location = serverConfigData->getLocationConfigData(*it, 1);
            return (recurFindLocation(url, location));
        }
    }
    Trie &prefixTrie = serverConfigData->getPrefixTrie();
    request.location = prefixTrie.find(url);
    LOG(cout << "location: " << request.location << endl);
    if (request.location == "")
    {
        response.setLocationConfigData(NULL);
        return (true);
    }
    location
        = serverConfigData->getLocationConfigData(request.location, 0);
    size_t i = url.find(location->getPath());
    string temp = url.substr(i + location->getPath().size());
    temp = "/" + temp;
    response.setLocationConfigData(recurFindLocation(temp, location));
    LOG(cout << "path: " << response.getLocationConfigData()->getPath() << endl);
    LOG(cout << "location: " << location << endl);
    LOG(cout << "lower location: " << request.location << endl);
    LOG(cout << "location here: " << location << endl);
    return (false);
}

//temp(must delete)
void    Client::showMessage(void)
{
    std::deque<std::string>::iterator  itd;

    time_t now = time(0);
    // time_t 형식을 문자열로 변환합니다.
    char* dt = ctime(&now);

    (void) dt;
    LOG(std::cout<<"time : "<<dt);
    //request 출력
    LOG(std::cout<<"=====strat line=====\n");
    LOG(std::cout<<"fd : "<<fd<<std::endl);
    LOG(std::cout<<request.method<<" "<<request.version<<" "<<request.url<<std::endl);
    for (std::map<std::string, std::string>::iterator it = request.query.begin(); it != request.query.end(); it++)
        LOG(std::cout<<it->first<<"="<<it->second<<std::endl);
    LOG(std::cout<<"=====header line=====\n");
    for (std::map<std::string, std::deque<std::string> >::iterator it = request.header.begin(); it != request.header.end(); it++)
    {
        LOG(std::cout<<it->first<<": ");
        for (itd = request.header[it->first].begin(); itd != request.header[it->first].end(); itd++)
            LOG(std::cout<<*itd<<"  ");
        LOG(std::cout<<"\n");
    }
    LOG(std::cout<<"\n\n");
}
