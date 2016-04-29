#include <boost/test/unit_test.hpp>

#include <prometheus/text_exposition.hpp>

#include <iostream>

using namespace prometheus;

BOOST_AUTO_TEST_CASE(ExpositionTest)
{
    CollectorRegistry registry;
    CounterPtr cf = CounterBuilder().name("foo").help("help").build();
    cf->inc();
    registry.add(cf);

    UntypedPtr up = UntypedBuilder().name("bar").build();
    registry.add(up);
    up->set(42.42);

    HistogramPtr hu = HistogramBuilder().name("baz").build();
    registry.add(hu);
    hu->observe(42);
    
    TextExposition exposition;
    std::string out = exposition.collect(registry);
    std::cout << out << std::endl;
}
