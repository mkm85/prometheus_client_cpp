#include <boost/test/unit_test.hpp>

#include <prometheus/prometheus.hpp>

using namespace prometheus;

BOOST_AUTO_TEST_CASE(TestDefaultRegistry)
{
    CounterPtr cf = CounterBuilder().name("foo").add();
    cf->inc();
    BOOST_CHECK_EQUAL(DefaultCollectorRegistry::instance()->collectors.size(), 1);
}
