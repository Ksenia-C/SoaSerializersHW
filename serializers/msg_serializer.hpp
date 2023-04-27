#include "interface.hpp"

#include <msgpack.hpp>

#include <fstream>

namespace serializer
{
    struct TestDataMsg : TestData
    {
        TestDataMsg() = default;
        TestDataMsg(const TestData &other)
        {
            this->one_array = other.one_array;
            this->one_str = other.one_str;
            this->one_map = other.one_map;
            this->u64 = other.u64;
            this->f64 = other.f64;
        }
        MSGPACK_DEFINE(one_str, one_array, one_map, u64, f64);
    };
    class Serializer : public SerializerInterface<TestDataMsg>
    {
    public:
        Serializer() : SerializerInterface("msgpack") {}

    private:
        std::pair<double, double>
        estimate_one_run(TestDataMsg &test, int clock_mode) override
        {
            std::pair<double, double> result;
            struct timespec begin, end;

            {
                std::ofstream ofs(SDATA_FILE_NAME);
                clock_gettime(clock_mode, &begin);
                msgpack::pack(ofs, test);
                clock_gettime(clock_mode, &end);
            }
            result.first = one_test_time(begin, end);

            TestDataMsg other;
            {
                std::ifstream ifs(SDATA_FILE_NAME);
                std::stringstream buffer(std::stringstream::binary | std::stringstream::in | std::stringstream::out);
                buffer << ifs.rdbuf();

                clock_gettime(clock_mode, &begin);

                msgpack::object_handle oh = msgpack::unpack(buffer.str().data(), buffer.str().size());
                msgpack::object const &obj = oh.get();

                other = obj.as<TestDataMsg>();

                clock_gettime(clock_mode, &end);
            }
            result.second = one_test_time(begin, end);

            assert(other.u64 == test.u64);
            return result;
        }
    };
}