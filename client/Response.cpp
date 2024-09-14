/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: inghwang <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/09 16:25:04 by inghwang          #+#    #+#             */
/*   Updated: 2024/08/09 16:25:05 by inghwang         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <map>
#include <string>
#include <vector>
#include "Response.hpp"
#include "Server.hpp"
#include "LocationConfigData.hpp"
#include "UtilTemplate.hpp"
#include "StartLine.hpp"
#include "CgiProcessor.hpp"

using namespace std;

map<int, string>  statusContentInit()
{
    map<int, string>  m;

    m[100] = " Continue";
    m[101] = " Switching Protocols";
    m[102] = " Processing";
    m[103] = " Early Hints";
    m[200] = " OK";
    m[201] = " Created";
    m[202] = " Accepted";
    m[203] = " Non-Authoritative Information";
    m[204] = " No Content";
    m[205] = " Reset Content";
    m[206] = " Partial Content";
    m[207] = " Multi-Status";
    m[208] = " Already Reported";
    m[214] = " Transformation Applied";
    m[226] = " IM Used";
    m[300] = " Multiple Choices";
    m[301] = " Moved Permanently";
    m[302] = " Found";
    m[303] = " See Other";
    m[304] = " Not Modified";
    m[305] = " Use Proxy";
    m[307] = " Temporary Redirect";
    m[308] = " Permanent Redirect";
    m[400] = " Bad Request";
    m[401] = " Unauthorized";
    m[402] = " Payment Required";
    m[403] = " Forbidden";
    m[404] = " Not Found";
    m[405] = " Method Not Allowed";
    m[406] = " Not Acceptable";
    m[407] = " Proxy Authentication Required";
    m[408] = " Request Timeout";
    m[409] = " Conflict";
    m[410] = " Gone";
    m[411] = " Length Required";
    m[412] = " Precondition Failed";
    m[413] = " Payload Too Large";
    m[414] = " Request-URI Too Long";
    m[415] = " Unsupporred Media Type";
    m[416] = " Request Range Not Staisfiable";
    m[417] = " Expectation Failed";
    m[418] = " I’m a teapot";
    m[420] = " Enhance Your Calm";
    m[421] = " Misdirected Request";
    m[422] = " Unprocessable Entity";
    m[423] = " Locked";
    m[424] = " Failed Dependency";
    m[425] = " Too Early";
    m[426] = " Upgrade Required";
    m[428] = " Precondition Required";
    m[429] = " Too many Requests";
    m[431] = " Request Header Fields Too Large";
    m[444] = " No Response";
    m[450] = " Blocked by Windows Parental Controls";
    m[451] = " Unavailable For Legal Reasons";
    m[497] = " HTTP Request Sent to HTTPS Port";
    m[498] = " Token expired/invalid";
    m[499] = " Client Closed Request";
    m[500] = " Internal Server Error";
    m[501] = " Not Implemented";
    m[502] = " Bad Gateway";
    m[503] = " Service Unavailable";
    m[504] = " Gateway Timeout";
    m[506] = " Variant Also Negotiates";
    m[507] = " Insufficient Storage";
    m[508] = " Loop Detected";
    m[509] = " Bandwidth Limit Exceeded";
    m[510] = " Not Extended";
    m[511] = " Network Authentication Required";
    m[521] = " Web Server Is Down";
    m[522] = " Connection Timed Out";
    m[523] = " Origin Is Unreachable";
    m[525] = " SSL Handshake Failed";
    m[530] = " Site Frozen";
    m[599] = " Network Connect Timeout Error";
    return (m);
}

map<string, string>  sessionInit()
{
    map<string, string>  m;

    return (m);
}

vector<std::string> cgiHeaderInit()
{
    std::vector<std::string>    v;

    v.push_back("content-type");
    v.push_back("location");
    v.push_back("status");
    return (v);
}

std::map<std::string, std::string>  urlContentTypeInit()
{
    std::map<std::string, std::string> mimeTypes;

    // Initialize the map with file extensions and their MIME types
    // 1. Text Files
    mimeTypes["html"] = "text/html";
    mimeTypes["htm"] = "text/html";
    mimeTypes["css"] = "text/css";
    mimeTypes["js"] = "application/javascript";
    mimeTypes["txt"] = "text/plain";
    mimeTypes["xml"] = "application/xml";
    // 2. ImageFiles
    mimeTypes["jpg"] = "image/jpeg";
    mimeTypes["jpeg"] = "image/jpeg";
    mimeTypes["png"] = "image/png";
    mimeTypes["gif"] = "image/gif";
    mimeTypes["svg"] = "image/svg+xml";
    mimeTypes["webp"] = "image/webp";
    mimeTypes["ico"] = "image/x-icon";
    // 3. Audioand Video Files
    mimeTypes["mp3"] = "audio/mpeg";
    mimeTypes["ogg"] = "audio/ogg";
    mimeTypes["wav"] = "audio/wav";
    mimeTypes["mp4"] = "video/mp4";
    mimeTypes["webm"] = "video/webm";
    mimeTypes["ogg"] = "video/ogg";
    // 4. Archie Files
    mimeTypes["zip"] = "application/zip";
    mimeTypes["gz"] = "application/gzip";
    mimeTypes["tar"] = "application/x-tar";
    mimeTypes["tar.gz"] = "application/gzip";
    // 5. Documnt Files
    mimeTypes["pdf"] = "application/pdf";
    mimeTypes["doc"] = "application/msword";
    mimeTypes["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    mimeTypes["xls"] = "application/vnd.ms-excel";
    mimeTypes["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    mimeTypes["ppt"] = "application/vnd.ms-powerpoint";
    mimeTypes["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    // 6. OtherFiles
    mimeTypes["json"] = "application/json";
    mimeTypes["csv"] = "text/csv";
    mimeTypes["webmanifest"] = "application/manifest+json";
    return (mimeTypes);
}

map<int, string>    Response::statusContent = statusContentInit();
map<string, string> Response::session = sessionInit();
vector<std::string> Response::cgiHeader = cgiHeaderInit();
map<string, string> Response::urlContentType = urlContentTypeInit();

bool	Response::isCgiScriptInURL(string& str)
{
	const string	availCgiExtensions[2] = {".py", ".php"};
	size_t			cgiFilePos;
    LOG(cout << "str: " << str << endl);
	for (int i=0; i<2; i++)
	{
		cgiFilePos = str.find(availCgiExtensions[i]);
		if (cgiFilePos != string::npos)
			break ;
	}
	if (cgiFilePos == string::npos)
	{
		// if (request.method == POST)
        // {
		// 	request.status = 400;
        // }
		return (false);
	}
	return (true);
}

void    Response::makeFilePath(string& str)
{
    LocationConfigData *location = getLocationConfigData();

    // LOG(cout << "host: " << request.header["host"].front() << endl);
    LOG(cout << "str before: " << str << endl);
    str = location->getRoot() + "/" + str;
    string strDir = str.substr(0, str.find_last_of("/"));
    LOG(cout<<"str: "<<str<<endl);
    if (isDirectory(strDir.c_str()) == false)
    {
        request.status = 404;
        LOG(cout << "404 1\n");
        return ;
    }
    if (!isWithinBasePath(location->getRoot(), strDir))
    {
        request.status = 403;
        LOG(cout << "403 1\n");
        return ;
    }
    if (str[str.size() - 1] == '/' && isDirectory(str.c_str()))
    {
        // 없으면 index.html 이라 없을 일은 없음.
        LOG(cout << "index: " << location->getIndex() << endl);
        string temp = str;
        str += "/" + location->getIndex();
        if (!isFile(str.c_str()))
        {
            if (request.method == GET && location->getAutoindex())
                str = temp + "/";
            else
            {
                LOG(cout << "403 2\n";)
                request.status = 403;
            }
            return ;
        }
    }
    else if (!isFile(str.c_str()))
    {
        LOG(cout << "not file: " << str << endl);
        request.status = 404;
        return ;
    }
	if (isCgiScriptInURL(str))
		cgiFlag = true;  //flag 껴짐
    LOG(cout << "str: " << str << endl);
}

Response::Response() : cgiFlag(false), port(0), startHeaderLength(0), contentLength(0),
    pathEnv(""), start(""), header(""), content(""), entity(""),
    serverConfig(NULL), locationConfig(NULL)
{
    request.status = 0;
}

Response::Response(const Response& src)
{
    *this = src;
}

Response&    Response::operator=(const Response& src)
{
    if (this == &src)
        return (*this);
    cgiFlag = src.getCgiFlag();
    port = src.getPort();
    startHeaderLength = src.getStartHeaderLength();
    contentLength = src.getContentLength();
    start = src.getStart();
    header = src.getHeader();
    content = src.getContent();
    entity = src.getEntity();
    request = src.getRequest();
    keyHeader = src.getKeyHeader();
    // serverConfig = src.getServerConfig();
    locationConfig = src.getLocationConfigData();
    return (*this);
}

Response::~Response()
{
}

Response::Response(int port, string pathEnv_) : cgiFlag(false), port(port), startHeaderLength(0), contentLength(0), pathEnv(pathEnv_), serverConfig(NULL), locationConfig(NULL)
{
    request.fin = false;
    request.status = 0;
    request.port = port;
    request.url.clear();
    request.location.clear();
    request.query.clear();
    request.header.clear();
    request.contentFileName.clear();
}

int Response::getPort() const
{
    return (port);
}

size_t  Response::getStartHeaderLength() const
{
    return (startHeaderLength);
}

size_t  Response::getContentLength() const
{
    return (contentLength);
}

string Response::getStart() const
{
    return (start);
}

string Response::getHeader() const
{
    return (header);
}

string Response::getContent() const
{
    return (content);
}

string Response::getEntity() const
{
    return (entity);
}

Request Response::getRequest() const
{
    return (request);
}

int Response::getRequestStatus() const
{
    return (request.status);
}

bool	Response::getCgiFlag() const
{
	return (cgiFlag);
}

std::vector<std::string>    Response::getKeyHeader() const
{
    return (keyHeader);
}

LocationConfigData *Response::getLocationConfigData() const
{
    return (locationConfig);
}

void    Response::setCgiFlag(bool flag)
{
    cgiFlag = flag;
}

void        Response::setPort(int port)
{
    this->port = port;
}

void	Response::setPathEnv(string pathEnv_)
{
	pathEnv = pathEnv_;
}

void    Response::setRequest(Request &temp)
{
    request = temp;
}

void    Response::setRequestStatus(int status)
{
    request.status = status;
}

void    Response::setLocationConfigData(LocationConfigData *locationConfigData)
{
    locationConfig = locationConfigData;
}

size_t  Response::setCgiHeader(string &content_, size_t &status)
{
    size_t  crlfPos;
    size_t  headerPos;
    string  headerNamePull;
    string  headerNameKey;

    crlfPos = content_.find("\r\n");
    if (crlfPos != string::npos)
    {
        headerNamePull = content_.substr(0, crlfPos);
        // LOG(cout << "cgi crlf READ: "<<headerNamePull<<endl);
        headerPos = headerNamePull.find(":");
        if (headerPos != string::npos)
        {
            headerNameKey = headerNamePull.substr(0, headerPos);
            HeaderLine::eraseSpace(headerNameKey, 1);
            if (std::find(cgiHeader.begin(), cgiHeader.end(), headerNameKey) != cgiHeader.end())
            {
                content_ = content_.substr(crlfPos + 2);
                if (headerNameKey == "status")
                {
                    std::stringstream ss(headerNamePull.substr(headerPos + 1));
                    ss >> status;
                    if (status != 200)
                        request.status = status;
                    LOG(cout<<"request.status: " << request.status<<endl);
                    if (status >= 400)
                    {
                        makeError();
                        return (0);
                    }
                    // if (request.status >= 400)
                    //     makeHeader(headerNameKey, toString(request.status));
                    // else
                    //     makeHeader(headerNameKey, headerNamePull.substr(headerPos + 1));
                }
                else
                    makeHeader(headerNameKey, headerNamePull.substr(headerPos + 1));
                LOG(cout<<headerNameKey<<" : "<<headerNamePull.substr(headerPos + 1)<<endl);
                return (crlfPos + 2);
            }
        }
    }
    return (0);
}

void        Response::setCgiGetContent(string &content_)
{
    content = content_;
    makeEntity();
}

void	    Response::setCgiGetHeader(size_t contentLength_)
{
    LOG(cout << "setCgiGetHeader" << endl << endl;)
    makeHeader("content-length", toString(contentLength_));
}

size_t  Response::setCgiContent(string &content_, size_t &status)
{
    size_t  pos;
    size_t  temp;

    if (status == 600)
    {
        request.status = 500;
        makeError();
        return (0);
    }
    pos = content_.find("\r\n");
    if (pos == 0)
    {
        content_ = content_.substr(pos + 2);
        pos = 2;
    }
    else
        pos = 0;
    do
    {
        temp = setCgiHeader(content_, status);
        if (status >= 400)
            return (0);
        pos += temp;
    } while (temp != 0);
    content = content_;
    return (pos);
}

void	Response::setCgiContentLength(size_t contentLength_)
{
    contentLength = contentLength_;
    makeHeader("content-length", toString(contentLength));
    LOG(cout << "setCgiContentLength" << endl << endl);
    makeEntity();
    // LOG(std::cout<<"header: \n\n"<<header);
    // LOG(std::cout<<"================"<<std::endl);
}

void    Response::initRequest(Request msg)
{
    request = msg;
    // LOG(cout<<request.fin<<endl);
    // LOG(cout<<request.status<<endl);
    // LOG(cout<<request.port<<endl);
    // LOG(cout<<request.clientFd<<endl);
    // LOG(cout<<request.method<<endl);
    // LOG(cout<<request.clientIp<<endl);
    // LOG(cout<<request.url<<endl);
    // LOG(cout<<request.location<<endl);
    // LOG(cout<<request.version<<endl);
}

bool    Response::init()
{
    LOG(cout << "port: " << port << endl);
    cgiFlag = false;
    start.clear();
    header.clear();
    content.clear();
    entity.clear();
    if (request.status != 0)
        return (false);
    string host = request.header["host"].front();
    LOG(cout << "host : " << host << endl);
    // LOG(cout << "host: " << host << endl);
    try
    {
        serverConfig = Server::serverConfig->getServerData(host, port);
    }
    catch(const exception& e)
    {
        serverConfig = Server::serverConfig->getDefaultServer(port);
        if (serverConfig == NULL)
        {
            request.status = 400;
            return (true);
        }
    }
    return (false);
}

int Response::getDefaultErrorPage(int statusCode)
{
    string errorStr = getLocationConfigData()->getErrorPage()[statusCode];

    int fd = -1;

    if (errorStr.size() > 0)
        fd = open(errorStr.c_str(), O_RDONLY);
    if (fd != -1)
        return (fd);
    if (statusCode >= 400 && statusCode < 500)
        return (open(DEFAULT_400_ERROR_PAGE, O_RDONLY));
    if (statusCode >= 500)
        return (open(DEFAULT_500_ERROR_PAGE, O_RDONLY));
    return (open(DEFAULT_400_ERROR_PAGE, O_RDONLY));
}

void    Response::makeCookie(string& date)
{
    const string    characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    const size_t    charactersSize = characters.size();
    string          result;
    size_t          index;
    string          value;
    string          cookieValue;

    if (request.header["cookie"].empty())
    {
        do {
            for (size_t i = 0; i < 20; ++i)
            {
                index = rand() % charactersSize;
                result += characters[index];
            }
        } while (session.find(cookieValue) != session.end());
        session[result] = date;
        value = "session_id=" + result + "; Max-Age=3600";  //1시간
        // value = "session_id=" + result + "; Max-Age=60";  //1분
        makeHeader("Set-Cookie", value);
    }
    else
    {
        cookieValue = request.header["cookie"].front();
        index = cookieValue.find(';');
        if (index != string::npos)
            cookieValue = cookieValue.substr(0, index);
        index = cookieValue.find('=');
        if (index != string::npos)
            cookieValue = cookieValue.substr(index + 1);
        HeaderLine::eraseSpace(cookieValue, 0);
        if (session.find(cookieValue) == session.end())
            session[cookieValue] = date;
        // LOG(std::cout<<"cookieValue: "<<cookieValue<<std::endl<<std::endl);
        makeHeader("session", session[cookieValue]);
    }
}

void    Response::makeDefaultHeader()
{
    time_t              now;
    char*               dt;
    std::string         date;
    int                 order;
    std::string         temp;
    std::string         day[5];
    size_t              pos;

    now = time(0);
    dt = ctime(&now);
    std::string         str(dt);
    std::istringstream  strStream(str);
    order = 0;
    while (getline(strStream, temp, ' '))
    {
        if (temp == "")
            continue;
        pos = temp.find_last_not_of('\n');
        temp.erase(pos + 1);
        day[order++] = temp;
    }
    date = day[0] + ", " + day[2] + " " + day[1] + " " + day[4] + " " + day[3] + " GMT";
    makeHeader("date", date);
    makeHeader("server", "inghwang/0.0");
    // if (request.status < 400)
    //     makeHeader("connection", "keep-alive");
    // else
    //     makeHeader("connection", "close");
    makeCookie(date);
}

void    Response::makeError()
{
    LOG(cout << "makeError\n"<<request.status<<std::endl);
    if (request.status >= 300 && request.status < 400)
        return ;
    // if (request.status == 100)
    //     return ;
	LOG(cout << request.clientFd << std::endl);
    LocationConfigData	*location = getLocationConfigData();
    map<int, string>	errorPage;
    string              errorPath = "";
    if (location != NULL)
    {
        errorPage = location->getErrorPage();
        errorPath = errorPage[request.status];
    }
    else if (serverConfig != NULL)
    {
        errorPage = serverConfig->getErrorPage();
        errorPath = errorPage[request.status];
    }

    LOG(cout << "errorPath: " << errorPath << endl);
    int fd = -1;
    if (errorPath != "")
        fd = open(errorPath.c_str(), O_RDONLY);
    if (errorPath != "" && fd >= 0)
    {
        request.url = errorPath;
        makeContent(fd);  //how
    }
    else
    {
        CgiProcessor cgiProcessor(request, serverConfig, locationConfig, pathEnv);
		cgiProcessor.insertEnv("ERROR_CODE", toString(request.status));
        cgiProcessor.executeCGIScript(CgiProcessor::EXECUTE_PATH + CGI_ERROR_PAGE);
        setCgiFlag(true);
    }
}

int Response::checkRedirect()
{
    LocationConfigData  *location = getLocationConfigData();
    pair<int, string>   &redirect = location->getReturn();

    if (redirect.first != 0)
    {
        request.status = redirect.first;
        request.url = redirect.second;
        LOG(cout << request.url << endl);
        LOG(cout << request.status << endl);
        makeHeader("Location", redirect.second);
        // start = "HTTP/1.1 " + to_string(request.status) + statusContent[request.status] + "\r\n";
    }
    if (request.status > 0)
    {
        makeEntity();
        return (1);
    }
    return (0);
}

int Response::checkAllowedMethod() 
{
    LocationConfigData  *location = getLocationConfigData();
    vector<string>    &allowedMethods = location->getAllowedMethods();
    // for (auto iter = allowedMethods.begin(); iter != allowedMethods.end(); iter++)
    //     LOG(cout << "allowed method : " << endl);

    if (find(allowedMethods.begin(), allowedMethods.end(),
        StartLine::methodString[request.method]) == allowedMethods.end())
    {
        request.status = 405;
        makeError();
        return (1);
    }
    return (0);
}

void    Response::makeHeader(string key, string value)
{
    // if (std::find(keyHeader.begin(), keyHeader.end(), key) != keyHeader.end())
    //     return ;
    // keyHeader.push_back(key);
    header += key + ": " + value + "\r\n";
}

void    Response::makeContent(int fd)
{
    int         statReturn;
    struct stat fileStat;
    string  location = request.url;
    size_t  pos = location.find_last_of('.');

    string contentType;
    LOG(cout << "location: " << location << endl;)
    if (pos != string::npos)
    {
        string extension = location.substr(pos + 1);
        LOG(cout << "extension: " << extension << endl;)
        contentType = urlContentType[extension];
        if (contentType == "")
            contentType = "application/octet-stream";
    }
    else
        contentType = "application/octet-stream";
    LOG(cout << "[Response::makeContent] - fd + content-type: " << fd << ' ' << contentType << endl;)
    statReturn = stat(request.url.c_str(), &fileStat);
    if (throwIfError(errno, statReturn) < 0)  //makeError
    {
        request.status = 500;
        return ;
    }
    makeHeader("content-type", contentType);
    if (fileStat.st_size == 0)
    {
        request.status = 200;
        makeHeader("content-length", "0");
        return ;
    }
    Kq::plusEvent(fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
    cgiFlag = true;
    Kq::cgiFd[fd] = request.clientFd;
    Kq::pidPipe[fd] = -1;
    LOG(cout << "cgiFd[fd]: " << request.clientFd << endl);
    // ssize_t readSize;
    // char    buffer[4096];
    // ssize_t count = 0;
    // while (1)
    // {
    //     readSize = read(fd, buffer, 4095);
    //     if (readSize <= 0)
    //         break ;
    //     content.append(buffer, readSize);
    //     count += readSize;
    // }
    // LOG(cout<<"Content Size: "<<content.size()<<endl);
    // contentLength = count;
    // makeHeader("content-length", toString(count));
    // close(fd);
}

void    Response::makeEntity()
{
    entity.clear();
    if (request.status >= 400)
        makeHeader("connection", "close");
    else
        makeHeader("connection", "keep-alive");
    // LOG(std::cout<<request.status<<std::endl);
    if (request.status == 0)
        request.status = 200;
    entity = "HTTP/1.1 " + toString(request.status) + statusContent[request.status] + "\r\n";
    if (!header.empty())
        entity += header + "\r\n";
    startHeaderLength = entity.size();
    if (!content.empty())
        entity.append(content);
    // LOG(std::cout<<"request.status: "<<request.status<<std::endl);
    // LOG(cout<<entity.size()<<endl);
}

void    Response::makeGet()
{
    int fd;

    LOG(std::cout<<"Method: GET"<<std::endl);
    LOG(cout << "cgiFlag: " <<cgiFlag<<endl;)
    LOG(std::cout<<request.url.c_str()<<std::endl);
    CgiProcessor cgiProcessor(request, serverConfig, locationConfig, pathEnv);

    // cout << "is directory: " << isDirectory(request.url.c_str()) << endl;
    // cout << "location: " << getLocationConfigData()->getPath() << endl;
    // cout << "autoindex: " << getLocationConfigData()->getAutoindex() << endl;
    
    // LOG(cout << "is directory: " << isDirectory(request.url.c_str()) << endl);
    // LOG(cout << "location: " << getLocationConfigData()->getPath() << endl);
    // LOG(cout << "autoindex: " << getLocationConfigData()->getAutoindex() << endl);

    //directory 검사는 makeFilePath 에서 함
    // LOG(cout << "request url: " << request.url << endl);
    if (isDirectory(request.url.c_str()))
    {
        cgiFlag = true;
        LOG(cout << "directory listing" << endl);
    	cgiProcessor.executeCGIScript(CgiProcessor::EXECUTE_PATH + AUTOINDEX_PATH);
    }
	else if (cgiFlag)
	{
		cgiProcessor.executeCGIScript(request.url);
		if (request.status >= 400)
		{
			while (!cgiProcessor.getFin())
				cgiProcessor.executeCGIScript(CgiProcessor::EXECUTE_PATH + CGI_ERROR_PAGE);
		}
		// 나중에 content-length가 0일 때, 서버 에러 추가
		// start = "HTTP1.1 " + to_string(request.status) + statusContent[request.status] + "\r\n";
		// makeHeader("content-type", "text/html");
		// makeHeader("content-length", toString(contentLength)); //여기서 추가하고 나중에 또 추가함
		// content += cgiProcessor.getCgiContent();
		// LOG(cout << cgiProcessor.getCgiContent() << '\n');
        // LOG(std::cout<<header);
	}
	else
	{
        fd = open(request.url.c_str(), O_RDONLY);
        throwIfError(errno, fd);   //아래에 있다.
        if (fd < 0)
        {
            request.status = 404;
            LOG(cout << "fd error" << endl;)
            // start = "HTTP1.1 " + to_string(request.status) + statusContent[request.status] + "\r\n";
            // while (!cgiProcessor.getFin())
            // 	cgiProcessor.executeCGIScript(CgiProcessor::EXECUTE_PATH + CGI_ERROR_PAGE);
        }
        else
        {
            LOG(cout << "request.url: " << request.url << " fd: " << fd << endl;)
            makeContent(fd);
        }
        if (request.status >= 400)
            makeError();
	}
    // start = "HTTP1.1 " + to_string(request.status) + statusContent[request.status] + "\r\n";
}

void    Response::makePost()
{
    LOG(cout<<"Method: POST"<<endl);
	CgiProcessor cgiProcessor(request, serverConfig, locationConfig, pathEnv);
    LOG(cout << "request url: " << request.url << endl);
    LOG(cout << "[Response::makePost] - cgiFlag: " << cgiFlag << endl;)
    if (cgiFlag)
    {
        chdir(getDir(request.url).c_str());
        cgiProcessor.checkPostContentType(request.url);
    }
    else
    {
        int fd = open(request.url.c_str(), O_RDONLY);
		if (fd < 0)
			request.status = 404;
        else
            makeContent(fd);
    }
	if (request.status >= 400)
        makeError();
}

void    Response::makeDelete()
{
    LOG(cout<<"Method: DELETE"<<endl);

    if (cgiFlag)
    {
        CgiProcessor cgiProcessor(request, serverConfig, locationConfig, pathEnv);
        chdir(getDir(request.url).c_str()); 
        if (!cgiProcessor.isValidUploadPath())
        {
            LOG(cout << "Upload Path Error" << endl;)
            makeError();
            return ;
        }
        if (!throwIfError(errno, chdir(getDir(request.url).c_str())))
        {
            request.status = 500;
            makeError();
            return ;
        }
        LOG(cout << "request url: " << request.url << endl);
        cgiProcessor.executeCGIScript(request.url);
    }
    else
        request.status = 405;
	if (request.status >= 400)
        makeError();
}

void    Response::responseMake()
{
    LOG(cout << "cgiFlag: " <<cgiFlag<<endl;)
    if (request.status > 0 || init())
    {
        makeError();
        return ;
    }
    // LOG(cout << "request.status: " << request.status << endl);
    makeDefaultHeader();
    if (checkAllowedMethod())
        return ;
    // LOG(cout << "path: " << locationConfig->getPath() << endl);
    if (checkRedirect())
        return ;
    makeFilePath(request.url);
    if (request.status >= 400)
    {
        LOG(std::cout<<"ERROR\n"<<std::endl);
        makeError();
        return ;
    }
    switch (request.method)
    {
        case GET:
            makeGet();
            break ;
        case POST:
            makePost();
            break ;
        case DELETE:
            makeDelete();
            break ;
        default:
            break ;
    }
    if (!cgiFlag)
        makeEntity();
    return ;
}

