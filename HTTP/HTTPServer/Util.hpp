#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>

class Util
{
public:
    static bool ReadContentFile(const std::string filename, std::string *out)
    {
        std::fstream fs(filename);
        if (!fs.is_open())
            return false;
        std::stringstream ss;
        ss << fs.rdbuf();
        *out = ss.str();
        fs.close();
        return true;
    }
    static bool ReadOneline(std::string &reqstr, std::string *out, const std::string &sep)
    {
        size_t pos = reqstr.find(sep);
        if (pos == std::string::npos)
            return false;
        *out = reqstr.substr(0, pos);
        reqstr.erase(0, pos + sep.size());
        return true;
    }
};
