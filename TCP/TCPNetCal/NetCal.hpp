#pragma once
#include "Protocal.hpp"
#include <iostream>

enum ResCode
{
    INFO = 0,
    ERROR,
    UNKNOW
};

// 利用Cal完成计算, 即业务功能, 将Request转为Response
class Cal
{
public:
    Cal()
    {
    }
    Response Execute(Request &req)
    {
        Response resp;
        switch (req.Oper())
        {
        case '+':
        {
            resp.SetResult(req.X() + req.Y());
            resp.SetCode(INFO);
            break;
        }
        case '-':
        {
            resp.SetResult(req.X() - req.Y());
            resp.SetCode(INFO);
            break;
        }
        case '*':
        {
            resp.SetResult(req.X() * req.Y());
            resp.SetCode(INFO);
            break;
        }
        case '/':
        {
            if (req.Y() == 0)
            {
                resp.SetCode(ERROR);
                break;
            }
            resp.SetResult(req.X() / req.Y());
            resp.SetCode(INFO);
            break;
        }
        case '%':
        {
            if (req.Y() == 0)
            {
                resp.SetCode(ERROR);
                break;
            }
            resp.SetResult(req.X() % req.Y());
            resp.SetCode(INFO);
            break;
        }
        default:
        {
            resp.SetCode(UNKNOW);
            break;
        }
        }
        return resp;
    }
};
