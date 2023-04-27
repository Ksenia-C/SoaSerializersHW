#include <iostream>
#include <math.h>
#include <memory>
#include <cstdlib>
#include <restbed>
#include <cstring>
#include "yaml-cpp/yaml.h"

using namespace std;
using namespace restbed;

std::pair<std::map<string, string> &, std::map<string, int> &> get_all_handlers_addresses()
{
    static std::map<string, string> g_host_map;
    static std::map<string, int> g_port_map;

    if (g_port_map.empty())
    {
        YAML::Node config = YAML::LoadFile("docker-compose.yaml")["services"];
        for (auto it = config.begin(); it != config.end(); ++it)
        {
            const auto &key_str = it->first.as<std::string>();
            std::string format_name(key_str.begin() + std::strlen("serializer-"), key_str.end());

            if ("proxi" == format_name)
            {
                continue;
            }

            auto &node = it->second;
            auto port_str = node["environment"][0].as<std::string>();
            int port = atoi(std::string(port_str.begin() + std::strlen("PORT="), port_str.end()).c_str());
            auto host = node["container_name"].as<std::string>();

            g_host_map[format_name] = host;
            g_port_map[format_name] = port;
        }
    }
    return {g_host_map, g_port_map};
}

std::pair<std::map<string, string>, std::map<string, int>> get_handlers_addresses(const std::string &format)
{
    const auto &[g_host_map, g_port_map] = get_all_handlers_addresses();
    return {{{format, g_host_map[format]}}, {{format, g_port_map[format]}}};
}

void get_method_handler(const shared_ptr<Session> session)
{
    const auto request = session->get_request();

    int content_length = request->get_header("Content-Length", 0);
    auto name = request->get_path_parameter("format");
    std::string test_size = request->get_query_parameter("test_size", "0");

    const auto &[hosts, ports] = name != "all" ? get_handlers_addresses(name) : get_all_handlers_addresses();

    session->fetch(content_length, [&](const shared_ptr<Session> session, const Bytes &body)
                   {
                       try
                       {
                           std::vector<std::future<std::shared_ptr<Response>>> proxy_replies;
                           std::string results;

                           for (auto &[key, port] : ports)
                           {
                               auto request = make_shared<Request>();
                               request->set_method("GET");
                               request->set_path("/get_result");
                               request->set_port(port);
                               request->set_host(hosts.at(key));
                               request->set_query_parameter("test_size", test_size);

                               proxy_replies.push_back(Http::async(request, [results = &results](const shared_ptr<Request> request, const shared_ptr<Response> reply)
                                                                   {
                    auto length = reply->get_header("Content-Length", 0);

                    if (length == 0)
                    {
                        results->append("\nFailed to run experiments in ").append(request->get_host());
                    }
                    else
                    {
                        auto reply_body = Http::fetch(length, reply);
                        auto body = (const char *)reply_body.data();
                        results->append("\n").append(std::string(body, body + length));
                    } }));
                           }
                           for (auto &proxy_reply : proxy_replies)
                           {
                               proxy_reply.wait();
                           }
                           session->close(OK, results, {{"Content-Length", to_string(results.size())}});
                       }
                       catch (std::exception &err)
                       {
                            std::string results = "Unexpected error happened: ";
                            results.append(err.what());
                            session->close(INTERNAL_SERVER_ERROR, results, {{"Content-Length", to_string(results.size())}});
                       } });
}

int main(const int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./proxy_server port" << std::endl;
        return 1;
    }
    auto resource = make_shared<Resource>();
    resource->set_path("/get_result/{format: .*}");
    resource->set_method_handler("GET", get_method_handler);

    auto settings = make_shared<Settings>();
    settings->set_port(atoi(argv[1]));
    settings->set_default_header("Connection", "close");
    settings->set_reuse_address(true);

    std::cerr << "Proxy is running" << std::endl;
    Service service;
    service.publish(resource);
    service.start(settings);

    return 0;
}