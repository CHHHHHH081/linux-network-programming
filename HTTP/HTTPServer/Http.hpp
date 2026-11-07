#pragma once
#include "TCPServer.hpp"
#include "log.hpp"
#include "Util.hpp"
#include <iostream>
#include <memory>
#include <sstream>
#include <fstream>
#include <string>
#include <unordered_map>

using namespace LogModule;

const std::string gspace = " ";
const std::string glinespace = "\r\n";
const std::string glinesep = ": ";
const std::string emptyline = "\r\n\r\n";
const std::string webroot = "./wwwroot";
const std::string homepage = "index.html";
const std::string errpage = "errpage.html";

class HttpRequest
{
public:
    HttpRequest()
    {
    }
    std::string Serialize()
    {
        std::string ret;
        ret = _method + gspace + _uri + gspace + _version + glinespace;
        for (auto kv : _headers)
        {
            ret += (kv.first + glinesep + kv.second + glinespace);
        }
        ret += (glinespace + _text);
        return ret;
    }

    void ParseReq(std::string reqline)
    {
        std::stringstream ss(reqline);
        ss >> _method >> _uri >> _version;
    }

    bool DeSerialize(std::string &reqstr)
    {
        std::string res;
        bool stat = Util::ReadOneline(reqstr, &res, glinespace);
        LOG(Loglevel::DEBUG) << reqstr;
        ParseReq(res);
        if (_uri == "/")
            _uri = webroot + _uri + homepage;
        else
            _uri = webroot + _uri;
        LOG(Loglevel::DEBUG) << "_method: " << _method;
        LOG(Loglevel::DEBUG) << "_uri: " << _uri;
        LOG(Loglevel::DEBUG) << "_version: " << _version;
        return true;
    }
    std::string URI() { return _uri; }
    ~HttpRequest()
    {
    }

private:
    std::string _method;
    std::string _uri;
    std::string _version;
    std::unordered_map<std::string, std::string> _headers;
    std::string _text;
};

class HttpResponse
{
public:
    HttpResponse()
    {
    }
    std::string Serialize()
    {
        std::string ret;
        ret = _version + gspace + std::to_string(_code) + gspace + _desc + glinespace;
        for (auto kv : _headers)
        {
            ret += (kv.first + glinesep + kv.second + glinespace);
        }
        ret += (glinespace + _text);
        return ret;
    }
    // bool DeSerialize()
    // {
    // }
    bool SetTargetFile(const std::string targetfile) // 即URI
    {
        _targetfile = targetfile;
        return true;
    }
    bool MakeResponse()
    {
        bool res = Util::ReadContentFile(_targetfile, &_text);
        if (!res)
        {
            _code = 404;
            _desc = "not found";
            std::string errpage = homepage + errpage;
            SetTargetFile(errpage);
            SetTargetFile(_targetfile);
            Util::ReadContentFile(_targetfile, &_text);
            return false;
        }
        _version = "HTTP/1.1";
        _code = 200;
        _desc = "OK";
        _headers["Content-Type"] = "text/html; charset=utf-8";
        _headers["Content-Length"] = std::to_string(_text.size());
        return true;
    }
    ~HttpResponse()
    {
    }

    // private:
    std::string _version;
    int _code;
    std::string _desc;
    std::unordered_map<std::string, std::string> _headers;
    std::string _targetfile;
    std::string _text;
};

class Http
{
public:
    Http(uint16_t port)
        : tcpsvrptr(std::make_shared<TCPServer>(port))
    {
    }
    void HandlerRequest(std::shared_ptr<Socket> &sock, InetAddr &client)
    {
        std::string reqstr;
        int n = sock->Recv(&reqstr);
        if (n > 0)
        {
            std::cout << reqstr << std::endl;
            HttpRequest req;
            req.DeSerialize(reqstr);
            HttpResponse resp;
            resp.SetTargetFile(req.URI());
            resp.MakeResponse();
            std::string ret = resp.Serialize();
            std::cout << ret << std::endl;
            sock->Send(ret);
#ifdef DEBUG
#define DEBUG
            std::cout
                << reqstr << std::endl;
            // 先直接构建完整应答调试
            HttpResponse resp;
            resp._version = "HTTP/1.1";
            resp._code = 200;
            resp._desc = "OK";
            resp._text = "<!DOCTYPE html>\n<html>\n<head>\n    <title>Hello World</title>\n    <style>\n        body { background-color: white; }\n    </style>\n</head>\n<body>\n    <h1>Hello, World!</h1>\n</body>\n</html>";
            std::string ret = resp.Serialize();
            sock->Send(ret);
#endif
        }
    }
    void Start()
    {
        tcpsvrptr->Run([this](std::shared_ptr<Socket> &sock, InetAddr &client)
                       { this->HandlerRequest(sock, client); });
    }
    ~Http()
    {
    }

private:
    std::shared_ptr<TCPServer> tcpsvrptr;
};
