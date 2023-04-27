#include "interface.hpp"

#include "test_data.pb.h"

#include <fstream>
#include <fcntl.h>
#include <assert.h>

namespace serializer
{
    struct TestDataProto
    {
        TestDataProto() = default;
        TestDataProto(const TestData &test)
        {
            impl.set_one_str(test.one_str);
            impl.mutable_one_array()->Add(test.one_array.begin(), test.one_array.end());
            impl.set_f64(test.f64);
            impl.set_u64(test.u64);
            impl.mutable_one_map()->insert(test.one_map.begin(), test.one_map.end());
        }

        TEST_DATA::Test_Data impl;
    };
    class Serializer : public SerializerInterface<TestDataProto>
    {
    public:
        Serializer() : SerializerInterface("proto") {}

    private:
        std::pair<double, double>
        estimate_one_run(TestDataProto &test, int clock_mode) override
        {
            std::pair<double, double> result;
            struct timespec begin, end;

            {
                std::ofstream ofs(SDATA_FILE_NAME, std::ios::out | std::ios::trunc | std::ios::binary);
                assert(!ofs.fail());
                clock_gettime(clock_mode, &begin);

                assert(test.impl.SerializeToOstream(&ofs));
                clock_gettime(clock_mode, &end);
            }
            result.first = one_test_time(begin, end);

            TEST_DATA::Test_Data another;
            {
                std::ifstream ifs(SDATA_FILE_NAME, std::ios::in | std::ios::binary);
                assert(!ifs.fail());
                clock_gettime(clock_mode, &begin);

                assert(another.ParseFromIstream(&ifs));
                clock_gettime(clock_mode, &end);
            }
            result.second = one_test_time(begin, end);

            assert(test.impl.u64() == another.u64());
            return result;
        }
    };
}