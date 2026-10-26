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
    bool Deserialize(const std::string &s)
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
    bool Deserialize(const std::string &s)
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

    void ShowResult() { std::cout << _result << "[" << _code << "]" << std::endl; }

private:
    int _result;
    int _code;
};

using func_t = std::function<Response(Request &)>;
const std::string sep = "\r\n";

class Protocal
{
public:
    Protocal()
    {
    }
    Protocal(func_t func)
        : _func(func)
    {
    }
    void GetRequest(std::shared_ptr<Socket> &sock, InetAddr &client)
    {
        // 读取
        std::string buffer_queue;
        while (true)
        {
            int n = sock->Recv(&buffer_queue);
            if (n > 0)
            {
                std::string json_package;
                // 1. 不断解析报文，提取完整的json请求，直到不完整，就让服务器继续读取
                while (Decode(buffer_queue, &json_package))
                {
                    // 一定拿到了一个完整的报文
                    // 2. 请求json串，反序列化
                    Request req;
                    bool ok = req.Deserialize(json_package);
                    if (!ok)
                        continue;
                    // 3. 我一定得到了一个内部属性已经被设置了的req了.
                    // 通过req->resp, 不就是要完成计算功能嘛！！业务
                    Response resp = _func(req);

                    // 4. 序列化
                    std::string json_str = resp.Serialize();

                    // 5. 添加自定义长度
                    std::string send_str = Encode(json_str); // 携带长度的应答报文了"len\r\n{result:XXX, code:XX}\r\n"

                    // 6. 直接发送
                    sock->Send(send_str);
                }
            }
            else if (n == 0)
            {
                LOG(Loglevel::INFO) << "client:" << client.AddrString() << "Quit!";
                break;
            }
            else
            {
                LOG(Loglevel::ERROR) << "client:" << client.AddrString() << ", recv error";
                break;
            }
        }
    }

    bool GetResponse(std::shared_ptr<Socket> &sock, std::string &recv_buf, Response &resp)
    {
        while (true)
        {
            std::string json_package;
            int n = sock->Recv(&recv_buf);
            if (n > 0)
            {
                // recv_buf为读取到的信息
                while (Decode(recv_buf, &json_package))
                {
                    // json_package一定是recv_buf内的json请求, 需要将其反序列化到resp内
                    if (!resp.Deserialize(json_package))
                    {
                        LOG(Loglevel::WARNING) << "unvalid response json";
                        return false;
                    }
                    // 反序列化完成, resp内已有结果, 它的使命结束了
                    return true;
                }
            }
            else if (n == 0)
            {
                LOG(Loglevel::INFO) << "server quit!";
                return false;
                break;
            }
            else
            {
                LOG(Loglevel::ERROR) << "server recv error";
                return false;
                break;
            }
        }
        return false;
    }

    // 用于提取自定义报文: len\r\n{XXX:XXX,XX:XX}\r\n
    bool Decode(std::string &buffer, std::string *package)
    {
        ssize_t pos = buffer.find(sep);
        if (pos == std::string::npos)
            return false; // 让调用方继续从内核中读取数据
        std::string package_len_str = buffer.substr(0, pos);
        int package_len_int = 0;
        try
        {
            // errpos表示第一个未转换位置, 也就是出错的位置, 如果转换正确, 那它应该是最后位置
            size_t errpos = 0;
            package_len_int = std::stoi(package_len_str, &errpos);
            if (errpos != package_len_str.size() || package_len_int <= 0)
                return false;
        }
        catch (const std::exception &)
        {
            return false;
        }

        // buffer 一定有长度，但是一定有完整的报文吗？
        int target_len = package_len_str.size() + package_len_int + 2 * sep.size();
        if (buffer.size() < target_len)
            return false;

        // buffer一定至少有一个完整的报文！
        *package = buffer.substr(pos + sep.size(), package_len_int);
        buffer.erase(0, target_len);
        return true;
    }

    // 用于将json串转化为自定义报文: len\r\n{XXX:XXX,XX:XX}\r\n
    std::string Encode(std::string &msg)
    {
        std::string len = std::to_string(msg.size());
        return len + sep + msg + sep;
    }

    std::string BuildReqEncodeString(int x, int y, char oper)
    {
        Request req(x, y, oper);
        std::string jsonstr = req.Serialize();
        return Encode(jsonstr);
    }

private:
    func_t _func;
};
