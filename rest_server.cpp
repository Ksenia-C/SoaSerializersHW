
#include "serializers/serializer.hpp"

#include <iostream>
#include <math.h>
#include <memory>
#include <cstdlib>
#include <restbed>

using namespace std;
using namespace restbed;

void get_method_handler(const shared_ptr<Session> session)
{
    const auto request = session->get_request();

    int content_length = request->get_header("Content-Length", 0);
    size_t test_size = request->get_query_parameter("test_size", 0);

    session->fetch(content_length, [test_size](const shared_ptr<Session> session, const Bytes &body)
                   {
                    try {
                        serializer::Serializer ser;
                        auto test_result_wall = ser.estimate_timings(100, CLOCK_REALTIME, test_size == 0? std::nullopt : std::optional(test_size));
                        auto test_result_cpu = ser.estimate_timings(100, CLOCK_PROCESS_CPUTIME_ID, test_size == 0? std::nullopt : std::optional(test_size));

                        // waiting for gcc13 with std::format...
                        char fmt_data[128];
                        std::size_t fmt_line_sz = std::sprintf(fmt_data, "WAll: %s - %d - %fms - %fms\n", ser.get_name().data(), test_result_wall.test_struct_sz, test_result_wall.serialize_mean_time, test_result_wall.deserialize_mean_time);
                        std::string return_body(fmt_data, fmt_data + fmt_line_sz);

                        fmt_line_sz = std::sprintf(fmt_data, "CPU: %s - %d - %fms - %fms\n", ser.get_name().data(), test_result_wall.test_struct_sz, test_result_cpu.serialize_mean_time, test_result_cpu.deserialize_mean_time);

                        return_body+= std::string(fmt_data, fmt_data + fmt_line_sz);

                        session->close(OK, return_body, {{"Content-Length", to_string(return_body.size())}});}
                    catch (std::exception& err) {
                        std::string results = "Unexpected error happened: ";
                        results.append(err.what());
                        session->close(INTERNAL_SERVER_ERROR, results, {{"Content-Length", to_string(results.size())}});
                    } });
}

int main(const int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./server port" << std::endl;
        return 1;
    }
    auto resource = make_shared<Resource>();
    resource->set_path("/get_result");
    resource->set_method_handler("GET", get_method_handler);

    auto settings = make_shared<Settings>();
    settings->set_port(atoi(argv[1]));
    settings->set_default_header("Connection", "close");
    settings->set_reuse_address(true);

    std::cout << "Server  is running" << std::endl;
    Service service;
    service.publish(resource);
    service.start(settings);

    return 0;
}