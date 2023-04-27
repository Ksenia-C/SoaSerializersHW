#include "interface.hpp"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>

#include <fstream>

namespace serializer
{
    struct TestDataProto : TestData
    {
        TestDataProto() = default;
        TestDataProto(const TestData &other)
        {
            this->one_array = other.one_array;
            this->one_str = other.one_str;
            this->one_map = other.one_map;
            this->u64 = other.u64;
            this->f64 = other.f64;
        }

        template <class Archive>
        void serialize(Archive &ar, const unsigned int version)
        {
            ar &one_str;
            ar &one_array;
            ar &one_map;
            ar &u64;
            ar &f64;
        }
    };
    class Serializer : public SerializerInterface<TestDataProto>
    {
    public:
        Serializer() : SerializerInterface("text") {}

    private:
        std::pair<double, double>
        estimate_one_run(TestDataProto &test, int clock_mode) override
        {
            std::pair<double, double> result;
            struct timespec begin, end;

            {
                std::ofstream ofs(SDATA_FILE_NAME);
                clock_gettime(clock_mode, &begin);

                boost::archive::text_oarchive oa(ofs);
                oa << test;
                clock_gettime(clock_mode, &end);
            }
            result.first = one_test_time(begin, end);

            TestDataProto other;
            {
                std::ifstream ifs(SDATA_FILE_NAME);
                clock_gettime(clock_mode, &begin);

                boost::archive::text_iarchive ia(ifs);
                ia >> other;
                clock_gettime(clock_mode, &end);
            }
            result.second = one_test_time(begin, end);

            assert(other.u64 == test.u64);
            return result;
        }
    };
}