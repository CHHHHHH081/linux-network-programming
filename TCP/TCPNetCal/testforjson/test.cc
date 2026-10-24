#include <jsoncpp/json/json.h>
#include <string>
#include <memory>
#include <iostream>
#include <sstream>

int main()
{
    // Json::Value root;
    // root["name"] = "zhangsan";
    // root["age"] = 18;
    // root["addr"] = "Beijing";
    // Json::StreamWriterBuilder builder;
    // std::shared_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    // std::stringstream ss;
    // writer->write(root, &ss);
    // std::cout << ss.str() << std::endl;

    std::string json_string = "{\"name\":\"zhangsan\", \"age\":18, \"addr\":\"Beijing\"}";
    Json::Value root;
    Json::Reader reader;
    bool readflag = reader.parse(json_string, root);
    if (!readflag)
    {
        std::cerr << "Json read err" << std::endl;
        return 1;
    }
    std::cout << "name: " << root["name"].asString() << std::endl;
    std::cout << "age: " << root["age"].asInt() << std::endl;
    std::cout << "addr: " << root["addr"].asString() << std::endl;
}
