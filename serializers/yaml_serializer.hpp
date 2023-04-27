#include "interface.hpp"

#include "yaml-cpp/yaml.h"

#include <fstream>
#include <assert.h>

namespace serializer
{
    class Serializer : public SerializerInterface<TestData>
    {
    public:
        Serializer() : SerializerInterface("yaml") {}

    private:
        std::pair<double, double>
        estimate_one_run(TestData &test, int clock_mode) override
        {
            std::pair<double, double> result;
            struct timespec begin, end;

            {
                std::ofstream ofs(SDATA_FILE_NAME);
                YAML::Emitter out(ofs);
                clock_gettime(clock_mode, &begin);

                out << YAML::BeginMap;
                out << YAML::Key << "one_str";
                out << YAML::Value << test.one_str;
                out << YAML::Key << "one_array";
                out << YAML::Value << test.one_array;
                out << YAML::Key << "one_map";
                out << YAML::Value << test.one_map;
                out << YAML::Key << "u64";
                out << YAML::Value << test.u64;
                out << YAML::Key << "f64";
                out << YAML::Value << test.f64;
                out << YAML::EndMap;
                ofs.flush();
                clock_gettime(clock_mode, &end);
            }
            result.first = one_test_time(begin, end);

            TestData other;
            {
                clock_gettime(clock_mode, &begin);
                YAML::Node config = YAML::LoadFile(SDATA_FILE_NAME);

                other.one_str = std::move(config["one_str"].as<typeof(other.one_str)>());
                other.one_array = std::move(config["one_array"].as<typeof(other.one_array)>());
                other.one_map = std::move(config["one_map"].as<typeof(other.one_map)>());
                other.u64 = config["u64"].as<typeof(other.u64)>();
                other.f64 = config["f64"].as<typeof(other.f64)>();
                clock_gettime(clock_mode, &end);
            }
            result.second = one_test_time(begin, end);

            assert(other.u64 == test.u64);
            return result;
        }
    };
}
