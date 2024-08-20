#ifndef CGI_PROCESSOR_HPP
# define CGI_PROCESSOR_HPP
# include <map>
# include <deque>
# include "Response.hpp"
# include "ServerConfigData.hpp"
using namespace std;

struct Request;

class CgiProcessor
{
public:
	CgiProcessor(Request &request_, ServerConfigData *serverConfig_, LocationConfigData *locationConfig_);
	~CgiProcessor();
	CgiProcessor(const CgiProcessor& rhs);
	CgiProcessor&	operator=(const CgiProcessor& rhs);
	string	getScriptFile();
	string	getCgiContent();
	size_t	getContentLength();
	void	insertEnv(string key, string value);
	void	setURLEnv();
	bool	setStartHeaderEnv();
	bool	checkURL(string url);
	bool	checkPostContentType();
	void	executeCGIScript(const string path);
private:
	CgiProcessor();
	Request 			&request;
	ServerConfigData	*serverConfig;
	LocationConfigData	*locationConfig;
	string				scriptFile;
	string				cgiCommand;
	string				cgiContent;
	size_t				contentLength;
	map<string, string>			metaVariables;
	map<string, deque<string> > cgiResponseHeader;
};

#endif
