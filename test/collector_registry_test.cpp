#include <boost/test/unit_test.hpp>

#include <prometheus/prometheus.hpp>

using namespace prometheus;

BOOST_AUTO_TEST_CASE(TestDefaultRegistry)
{
    CounterPtr cf = CounterBuilder().name("foo").add();
    cf->labels()->inc();
    BOOST_CHECK_EQUAL(DefaultCollectorRegistry::instance()->collectors.size(), 1);
}

BOOST_AUTO_TEST_CASE(TestRegistry)
{
    auto registry = std::make_shared<CollectorRegistry>();
    CounterPtr cf = CounterBuilder().name("foo").add(registry);
    cf->labels()->inc();
    BOOST_CHECK_EQUAL(registry->collectors.size(), 1);
}
