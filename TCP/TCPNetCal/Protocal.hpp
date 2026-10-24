#pragma once
#include <climits>
#include <jsoncpp/json/json.h>
#include "Socket.hpp"

using namespace SocketModule;

class Request
{
public:
    Request() {}
    Request(int x, int y, char oper)
        : _x(x), _y(y), _oper(oper)
    {
    }
    std::string Serialize()
    {
        Json::Value root;
        root["x"] = _x;
        root["y"] = _y;
        root["oper"] = _oper;
        Json::StreamWriterBuilder build; // StreamWriter的工厂, 用于创建StreamWriter对象
        std::shared_ptr<Json::StreamWriter> writer(build.newStreamWriter());
        std::stringstream ss;
        writer->write(root, &ss);
        return ss.str();
    }
    bool Deserialize(std::string s)
    {
        Json::Value root;
        Json::Reader reader;
        bool readflag = reader.parse(s, root);
        if (!readflag)
        {
            std::cerr << "Json read err" << std::endl;
            return readflag;
        }
        _x = root["x"].asInt();
        _y = root["y"].asInt();
        _oper = root["oper"].asInt();
        return readflag;
    }

    int X() { return _x; }
    int Y() { return _y; }
    char Oper() { return _oper; }

private:
    int _x;
    int _y;
    char _oper;
};

class Response
{
public:
    Response()
        : _result(0), _code(0)
    {
    }
    Response(int res, int code)
        : _result(res), _code(code)
    {
    }
    std::string Serialize()
    {
        Json::Value root;
        root["result"] = _result;
        root["code"] = _code;
        Json::StreamWriterBuilder builder;
        std::shared_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        std::stringstream ss;
        writer->write(root, &ss);
        return ss.str();
    }
    bool Deserialize(std::string s)
    {
        Json::Value root;
        Json::Reader reader;
        bool readflag = reader.parse(s, root);
        if (!readflag)
        {
            std::cerr << "Json read err" << std::endl;
            return readflag;
        }
        _result = root["result"].asInt();
        _code = root["code"].asInt();
        return readflag;
    }

    int Result() { return _result; }
    int Code() { return _code; }

    void SetResult(int result) { _result = result; }
    void SetCode(int code) { _code = code; }

private:
    int _result;
    int _code;
};

using func_t = std::function<Response(Request &)>;
const std::string sep = "\r\n";

class Protocal
{
public:
    Protocal(func_t func)
        : _func(func)
    {
    }
    void GetRequest(std::shared_ptr<Socket> &sock, InetAddr &addr)
    {
        std::string buffer_queue;
        while (true)
        {
            // 尽量处理缓冲区中已有的完整报文
            bool badframe = false;
            while (true)
            {
                std::string json_package;
                int status = Decode(buffer_queue, &json_package);
                if (status <= 0)
                {
                    // 0: 数据不足, 1: 已取出一个完整报文, -1: 非法报文
                    badframe = (status < 0);
                    break;
                }
                // json_package 已是完整请求, 反序列化
                Request req;
                if (!req.Deserialize(json_package))
                    continue;
                // 回调业务函数完成计算
                Response resp = _func(req);
                std::string ret = resp.Serialize();
                int sendn = sock->Send(ret);
                (void)sendn;
            }
            if (badframe)
            {
                LOG(Loglevel::INFO) << "bad frame, quit";
                break; // 协议非法, 断开连接
            }

            // 缓冲区无完整报文, 继续读取
            int n = sock->Recv(&buffer_queue);
            if (n == 0)
            {
                LOG(Loglevel::INFO) << "quit";
                break; // 对端关闭
            }
            if (n < 0)
                break; // 读取出错
        }
    }

    // 用于提取自定义报文: len\r\n{XXX:XXX,XX:XX}\r\n, 放到json里
    // 返回: 1=已取出完整报文; 0=数据不完整需继续读; -1=协议非法
    int Decode(std::string &msg, std::string *json)
    {
        size_t pos = msg.find(sep);
        if (pos == std::string::npos)
            return 0; // 尚无长度前缀分隔符, 数据不足

        std::string len_str = msg.substr(0, pos);
        // 校验长度前缀: 必须是非负整数且不超过 int 范围
        int len = 0;
        try
        {
            size_t idx = 0;
            long v = std::stol(len_str, &idx);
            if (idx != len_str.size() || v < 0 || v > INT_MAX)
                return -1; // 非法长度前缀
            len = static_cast<int>(v);
        }
        catch (...)
        {
            return -1;
        }

        // 完整自定义报文长度
        int targetlen = len_str.size() + 2 * sep.size() + len;
        if (msg.size() < targetlen)
            return 0; // 报文未到齐
        *json = msg.substr(pos + sep.size(), len);
        msg.erase(0, targetlen);
        return 1;
    }

    // 用于将json串转化为自定义报文: len\r\n{XXX:XXX,XX:XX}\r\n
    std::string Encode(std::string &msg)
    {
        std::string len = std::to_string(msg.size());
        return len + sep + msg + sep;
    }

private:
    func_t _func;
};
