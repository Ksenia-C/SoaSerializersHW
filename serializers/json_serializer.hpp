#include "interface.hpp"

#include <boost/json/src.hpp>
using namespace boost::json;

#include <fstream>
#include <iostream>

namespace test_data
{
    void tag_invoke(value_from_tag, value &jv, TestData const &d)
    {

        jv = {
            {"one_str", d.one_str},
            {"one_array", value_from(d.one_array)},
            {"one_map", value_from(d.one_map)},
            {"u64", d.u64},
            {"f64", d.f64}};
    }

    template <class T>
    void extract(object const &obj, T &t, string_view key)
    {
        t = value_to<T>(obj.at(key));
    }

    TestData tag_invoke(value_to_tag<TestData>, value const &jv)
    {
        TestData c;
        object const &obj = jv.as_object();
        extract(obj, c.one_str, "one_str");
        extract(obj, c.one_array, "one_array");
        extract(obj, c.one_map, "one_map");
        extract(obj, c.u64, "u64");
        extract(obj, c.f64, "f64");
        return c;
    }
}

namespace serializer
{
    class Serializer : public SerializerInterface<TestData>
    {
    public:
        Serializer() : SerializerInterface("json") {}

    private:
        std::pair<double, double>
        estimate_one_run(TestData &test, int clock_mode) override
        {
            std::pair<double, double> result;
            struct timespec begin, end;

            {
                std::ofstream ofs(SDATA_FILE_NAME);
                clock_gettime(clock_mode, &begin);
                ofs << serialize(value_from(test));
                clock_gettime(clock_mode, &end);
            }
            result.first = one_test_time(begin, end);
            TestData other;
            {
                std::ifstream ifs(SDATA_FILE_NAME);
                clock_gettime(clock_mode, &begin);
                value jv = parse(ifs);

                other = std::move(value_to<TestData>(jv));
                clock_gettime(clock_mode, &end);
            }
            result.second = one_test_time(begin, end);

            assert(other.u64 == test.u64);
            return result;
        }
    };
}
