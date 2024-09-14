/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ContentLine.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: inghwang <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/20 16:24:10 by inghwang          #+#    #+#             */
/*   Updated: 2024/07/20 16:24:12 by inghwang         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ContentLine.hpp"
#include "UtilTemplate.hpp" 

ContentLine::ContentLine() : first(true), completion(false), fd(0), port(0), contentLength(0), maxSize(0)
{}

ContentLine::ContentLine(const ContentLine& src) : first(src.getFirst()), completion(src.getCompletion()), contentType(src.getContentType()), fd(src.getFd()), port(src.getPort()), contentLength(src.getContentLength()), maxSize(src.getMaxSize()), fileName(src.getFileName()), chunked(src.getChunked()), content(src.getContent())
{}

ContentLine::~ContentLine()
{}

ContentLine::ContentLine(int port) : first(true), completion(false), fd(0), port(port), contentLength(0), maxSize(0)
{}

ContentLine& ContentLine::operator=(const ContentLine& src)
{
    first = src.getFirst();
    completion = src.getCompletion();
    contentType = src.getContentType();
    port = src.getPort();
    contentLength = src.getContentLength();
    chunked = src.getChunked();
    content = src.getContent();
    return (*this);
}

bool    ContentLine::getFirst() const
{
    return (first);
}

bool    ContentLine::getCompletion() const
{
    return (completion);
}

CONTENTTYPE ContentLine::getContentType() const
{
    return (contentType);
}

int ContentLine::getFd() const
{
    return (fd);
}

int ContentLine::getContentLength() const
{
    return (contentLength);
}

int ContentLine::getPort() const
{
    return (port);
}

size_t  ContentLine::getMaxSize() const
{
    return (maxSize);
}

std::string ContentLine::getFileName() const
{
    return (fileName);
}

std::string ContentLine::getChunked() const
{
    return (chunked);
}

std::vector<std::string>    ContentLine::getContent() const
{
    return (content);
}

void    ContentLine::initContentLine(int initLength, CONTENTTYPE initType)
{
    contentLength = initLength;
    contentType = initType;
}

bool    ContentLine::fileExist(const char *fileName_)
{
    std::ifstream file(fileName_);

    bool exists = file.is_open();
    if (exists)
        file.close();
    return (exists);
}

bool    ContentLine::tempFileMake(int &fd_)
{
    std::string     fileName_ = CgiProcessor::EXECUTE_PATH + "/.tempContent/";
    size_t          num;

    mkdir(".tempContent", 0777);
    num = fd_;
    while (1)
    {
        if (!fileExist((fileName_ + toString(num)).c_str()))
        {
            LOG(std::cout<<(fileName_ + toString(num)).c_str()<<std::endl);
            fileName = fileName_ + toString(num);
            break ;
        }
        num++;
    }
    fd = open(fileName.c_str(), O_WRONLY | O_CREAT, 0777);
    LOG(std::cout<<"MAkE FILE"<<std::endl);
    if (fd < 0)
        return (false);
    LOG(std::cout<<"fileName: "<<fileName<<std::endl);
    return (true);
}

int ContentLine::chunkedEntity(int &status)
{
    std::istringstream  chunkedStream(chunked);
    std::string         temp;
    size_t              ans;
    int                 size;

    ans = 0;
    while (getline(chunkedStream, temp))
    {
        if (temp[temp.size() - 1] != '\r')
        {
            status = 400;
            return (-1);
        }
        temp.erase(temp.size() - 1);
        if (ans % 2 == 0)
        {
            try
            {
                size = std::stoi(temp, NULL, 16);
            }
            catch(const std::exception& e)
            {
                status = 400;
                return (-1);
            }
            if (size < 0)
            {
                status = 400;
                return (-1);
            }
        }
        else
        {
            if (temp.size() != static_cast<size_t>(size))
                return (-1);
            write(fd, &temp[0], size);
        }
        ans++;
    }
    completion = true;
    if (!throwIfError(errno, close(fd)))
    {
        status = 500;
        return (-1);  //makeError
    }
    return (0);
}

//readSize 가 msg 사이즈임
int ContentLine::makeContentLine(std::string &str, size_t &readSize, int &status, int &fd_)
{
    size_t  flag;

    maxSize += readSize;
    if (maxSize > 8000000000)
    {
        status = 413;
        return (-1);
    }
    if (first)
    {
        if (!tempFileMake(fd_))
        {
            status = 500;
            return (1);
        }
        first = false;
    }
    if (contentType == CONTENT)
    {
        //TODO: readSize 에 계속 쌓임
        LOG(std::cout<< "content Length: " << contentLength<<' '<<readSize<<std::endl);
        if (contentLength >= static_cast<int>(readSize))
        {
            contentLength -= static_cast<int>(readSize);
            // content.push_back(str);
            write(fd, &str[0], readSize);
            readSize  = 0;
            str.clear();
            if (contentLength == 0)
            {
                completion = true;
                if (!throwIfError(errno, close(fd)))
                {
                    status = 500;
                    return (-1);
                }
            }
        }
        else
        {
            // content.push_back(str.substr(0, contentLength));
            flag = write(fd, &str[0], contentLength);
            // contentLength = 0;
            str = str.substr(flag);
            completion = true;
            readSize -= flag;
            if (!throwIfError(errno, close(fd)))
            {
                status = 500;
                return (-1);
            }
        }
    }
    else if (contentType == TRANSFER)
    {
        chunked.append(&str[0], readSize);
        readSize = 0;
        str.clear();
        LOG(cout<<"chunked: "<<chunked<<endl);
        flag = chunked.find("0\r\n");
        // flag = str.find("0\r\n");  //talnet 때문에 임시로 대체함
        // 에러 발생시 중간에 빠져 나왔을 떄
        if (flag != std::string::npos)
        {
            //chunked 크기 확인하기
            str = chunked.substr(flag + 3);
            if (str[0] == '\r' && str[1] == '\n')
                str = str.substr(2);
            chunked = chunked.substr(0, flag);
            // str = str.substr(flag + 3);
            if (chunkedEntity(status) < 0)
                return (-1);
            readSize = str.size();
        }
    }
    return (0);
}

