#include <sys/stat.h>
#include "UtilTemplate.hpp"
#include "CgiProcessor.hpp"
#include "Kq.hpp"

string initEXCUTEPATH()
{
	return (realpath(".", NULL));
}

string CgiProcessor::EXECUTE_PATH = initEXCUTEPATH();

CgiProcessor::CgiProcessor(Request &request_, ServerConfigData *serverConfig_, LocationConfigData *locationConfig_, string pathEnv_)
	:request(request_)
	, serverConfig(serverConfig_)
	, locationConfig(locationConfig_)
	, pathEnv(pathEnv_)
	, contentLength(0)
	, fin(false)
{
}

CgiProcessor::~CgiProcessor()
{
}

string	CgiProcessor::getScriptFile()
{
	return (scriptFile);
}

string	CgiProcessor::getCgiContent()
{
	return (cgiContent);
}

size_t	CgiProcessor::getContentLength()
{
	return (contentLength);
}

bool	CgiProcessor::getFin()
{
	return (fin);
}

void	CgiProcessor::insertEnv(string key, string value)
{
	metaVariables.insert(make_pair(key, value));
}

void	CgiProcessor::setURLEnv()
{
	LOG(std::cout<<"host size: " <<request.header["host"].size()<<endl);
	insertEnv("SERVER_NAME", request.header["host"].front());
	insertEnv("SERVER_PORT", toString(request.port));
	insertEnv("PATH_INFO", request.url);
	insertEnv("SCRIPT_NAME", scriptFile);
	deque<string>	queryStrings;
	string			metaQueryString;
	for (map<string, string>::iterator iter=request.query.begin(); iter != request.query.end(); iter++)
	{
		string key = iter->first;
		string value = iter->second;
		queryStrings.push_back(key.append("=").append(value));
	}
	for (size_t idx = 0; idx < queryStrings.size(); idx++)
	{
		metaQueryString += queryStrings[idx];
		if (idx == queryStrings.size() - 1)
			continue;
		metaQueryString = metaQueryString.append("&");
	}
	insertEnv("QUERY_STRING", metaQueryString);
}

void	CgiProcessor::setStartHeaderEnv()
{
	insertEnv("SERVER_PROTOCOL", "HTTP/1.1");
	string	method;
	if (request.method == GET)
		method = "GET";
	else if (request.method == POST)
		method = "POST";
	else if (request.method == DELETE)
		method = "DELETE";
	insertEnv("REQUEST_METHOD", method);
	if (request.header.find("content-length") != request.header.end()
		&& request.header["content-length"].size() > 0)
	{
		insertEnv("CONTENT_LENGTH", request.header["content-length"].front());
	}
	if (request.header.find("content-type") != request.header.end()
		&& request.header["content-type"].size() > 0)
	{
		if (request.header["content-type"].size() > 1)
			insertEnv("CONTENT_TYPE", request.header["content-type"][0] + "; boundary=" + request.header["content-type"][1]);
		else
			insertEnv("CONTENT_TYPE", request.header["content-type"].front());
	}
	if (request.header.find("authorization") != request.header.end()
		&& request.header["authorization"].size())
	{
		insertEnv("AUTH_TYPE", request.header["authorization"][0]);
		if (request.header["authorization"].size() <= 2)
			insertEnv("REMOTE_USER", request.header["authorization"][1]);
	}
	insertEnv("REMOTE_ADDR", request.clientIp);
	insertEnv("REDIRECT_STATUS", "false");
	insertEnv("SCRIPT_FILENAME", scriptFile);
	string uploadPath = "";
	if (locationConfig != NULL)
	{
		uploadPath = locationConfig->getFastcgiParam()["UPLOAD_PATH"];
	}
	LOG(cout << "upload path: " << uploadPath << endl);
	insertEnv("UPLOAD_PATH", uploadPath);
}

void	CgiProcessor::selectCgiCmd(string url)
{
	// Find CGI Script File in URL
	const string	availCgiExtensions[2] = {".py", ".php"};
	string			cgiExtension;
	size_t			cgiFilePos;
	LOG(cout << "Url: " << url << endl);
	for (int i=0; i<2; i++)
	{
		cgiExtension = availCgiExtensions[i];
		cgiFilePos = url.find(cgiExtension);
		if (cgiFilePos != string::npos)
			break ;
	}
	LOG(cout << "cgiExtension: " << cgiExtension << endl);
	cgiCommand = (!cgiExtension.compare(".py")) ? "python3" : "php-cgi";
	scriptFile = url.substr(0, cgiFilePos + cgiExtension.size());
}

bool	CgiProcessor::isValidUploadPath()
{
	if (locationConfig == NULL)
	{
		request.status = 404;
		return (false);
	}
	string uploadPath = locationConfig->getFastcgiParam()["UPLOAD_PATH"];
	LOG(cout << "[isValidUploadPath() Function] - uploadPath: " << uploadPath << endl;)
	if (!uploadPath.compare("") || !isDirectory(uploadPath.c_str()))
	{
		request.status = 404;
		return (false);
	}
	return (true);
}

void	CgiProcessor::checkPostContentType(const string path)
{
    LOG(cout<<"content-length: "<<request.header["content-length"].front()<<endl);
    LOG(cout<<"content-type: "<<request.header["content-type"].front()<<endl);
	if (request.header.find("content-type") == request.header.end()
		|| request.header.find("content-length") == request.header.end()
		|| atol(request.header["content-length"].front().c_str()) <= 0)
	{
		request.status = 400;
		return ;
	}
	if (atol(request.header["content-length"].front().c_str()) > locationConfig->getClientMaxBodySize())
	{
		LOG(cout << "location path : " << locationConfig->getPath() << endl);
		LOG(cout << "client_max_body_size : " << locationConfig->getClientMaxBodySize() << endl);
		request.status = 413;
		return ;
	}
	if (!request.header["content-type"].front().compare("application/x-www-form-urlencoded")
		|| !request.header["content-type"].front().compare("application/json")
		|| !request.header["content-type"].front().compare("multipart/form-data"))
	{
		if (!request.header["content-type"].front().compare("multipart/form-data")
			&& !isValidUploadPath())
		{
			LOG(cout << "Upload Path Error" << endl;)
			return ;
		}
		insertEnv("CONTENT_FILENAME", request.contentFileName);
		executeCGIScript(path);
	}
	else
		request.status = 400;
}

bool	CgiProcessor::isDirectory(const char *binPath)
{
	struct stat	fileInfo;

	if (stat(binPath, &fileInfo) == -1)
	{
		return (false);
	}
	if (S_ISDIR(fileInfo.st_mode))
		return (true);
	return (false);
}

bool	CgiProcessor::checkErr(const char *binPath)
{
	if (binPath == 0)
		return (true);
	else if (access(binPath, F_OK))
		return (true);
	else if (isDirectory(binPath))
		return (true);
	else if (access(binPath, X_OK))
		return (true);
	return (false);
}

bool	CgiProcessor::findCgiCmdPath()
{
	vector<string> pathEnvList;
	istringstream iss(pathEnv);
	string buffer;
	while (getline(iss, buffer, ':'))
		pathEnvList.push_back(buffer);
	for (vector<string>::iterator iter = pathEnvList.begin(); iter != pathEnvList.end(); iter++)
	{
		string cmdPath = (*iter).append("/").append(cgiCommand);
		if (!checkErr(cmdPath.c_str()))
		{
			cgiCommand = cmdPath;
			return (true);
		}
	}
	return (false);
}

void	CgiProcessor::executeCGIScript(const string path)
{
	selectCgiCmd(path);
	setURLEnv();
	setStartHeaderEnv();
	int pipefd[2];
	LOG(cout << "path: " << path << endl;)
	if (!findCgiCmdPath() || pipe(pipefd) < 0)
	{
		LOG(cout << "No CGI Command " << cgiCommand << endl);
		LOG(cout << "or pipe() error " << endl);
		request.status = 500;
		return ;
	}
	if (!throwIfError(errno, fcntl(pipefd[0], F_SETFL, O_NONBLOCK)))
	{
		request.status = 500;
		return ;
	}
	LOG(std::cout<<"pipe fd: " << pipefd[0] << ", " << pipefd[1]<<std::endl);
	pid_t pid = fork();
	if (pid == -1)
	{
		LOG(cout << "fork() error" << endl;)
		close(pipefd[0]);  //makeError 아래에서 처리함
		close(pipefd[1]);  //makeError 아래에서 처리함
		request.status = 500;
		return ;
	}
	if (pid == 0)
	{
		if (!throwIfError(errno, close(pipefd[0])))
			exit(2);	//exit  logic 생각해보기
		if (!throwIfError(errno, dup2(pipefd[1], STDOUT_FILENO)))
			exit(2);  //exit
		if (!throwIfError(errno, close(pipefd[1])))
			exit(2);  //exit
		char *argv[] = {const_cast<char *>(&cgiCommand[0]), const_cast<char *>(&path[0]), NULL};
		char **envp = new char*[metaVariables.size() + 1];
		size_t	idx = 0;
		for (map<string, string>::iterator iter = metaVariables.begin(); iter != metaVariables.end(); iter++)
		{
			string	env;
			env = env.append(iter->first).append("=").append(iter->second);
			envp[idx] = new char[env.size() + 1];
			strcpy(envp[idx++], env.c_str());
		}
		envp[metaVariables.size()] = 0;
		int fd = open(request.contentFileName.c_str(), O_RDONLY, 0644);
		dup2(fd, STDIN_FILENO);
		close(fd);
		if (execve(&cgiCommand[0], argv, envp) == -1)
		{
			request.status = 500;
			LOG(std::cerr<<"execve error"<<endl);
			exit(1);
		}
	}
	else
	{
		close(pipefd[1]);
		Kq::processor.push_back(pid);
		if (!throwIfError(errno, chdir(EXECUTE_PATH.c_str())))  //이게 실패할 경우에 무조건 적으로 터짐
		{
			request.status = 500;
			return ;
		}
		Kq::plusEvent(pipefd[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
		Kq::cgiFd[pipefd[0]] = request.clientFd;
		Kq::clientToCgiFd[request.clientFd] = pipefd[0];
		Kq::pidPipe[pipefd[0]] = pid;
	}
}
