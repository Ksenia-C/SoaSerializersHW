#include "test_struct.hpp"

#include <optional>
#include <string>
#include <time.h>
#include <utility>

namespace serializer
{

    const static std::string SDATA_FILE_NAME = "saved_serialize_data";

    using TestData = test_data::TestData;

    double one_test_time(struct timespec &begin, struct timespec &end)
    {
        return (end.tv_sec - begin.tv_sec) * 1e3 + (end.tv_nsec - begin.tv_nsec) * 1e-6;
    }

    template <class TestDataExtend>
    class SerializerInterface
    {
    public:
        struct ExperimentReturnType
        {
            double serialize_mean_time;
            double deserialize_mean_time;
            size_t test_struct_sz;
        };

        SerializerInterface(const char *name) : name_(name)
        {
        }

        std::string &get_name() { return name_; }

        ExperimentReturnType estimate_timings(size_t experiment_cnt, int clock_mode, std::optional<size_t> test_size = std::nullopt)
        {
            ExperimentReturnType result;
            TestData base_data = test_size.has_value() ? test_data::get_big_test(*test_size) : test_data::get_small_test();
            result.test_struct_sz = base_data.get_data_size();
            TestDataExtend test = base_data;
            for (auto _ = 0; _ < experiment_cnt; ++_)
            {
                auto one_run = estimate_one_run(test, clock_mode);
                result.serialize_mean_time += one_run.first;
                result.deserialize_mean_time += one_run.second;
            }
            result.serialize_mean_time /= experiment_cnt;
            result.deserialize_mean_time /= experiment_cnt;
            return result;
        }

    private:
        std::string name_ = "interface";

        virtual std::pair<double, double>
        estimate_one_run(TestDataExtend &test, int clock_mode) = 0;
    };
}