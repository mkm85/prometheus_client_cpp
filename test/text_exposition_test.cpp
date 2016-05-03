#include <boost/test/unit_test.hpp>

#include <prometheus/text_exposition.hpp>

#include <iostream>

using namespace prometheus;

BOOST_AUTO_TEST_CASE(ExpositionTest)
{
    CollectorRegistry registry;
    CounterPtr cf = CounterBuilder().name("foo").help("help").build();
    cf->labels()->inc();
    registry.add(cf);

    UntypedPtr up = UntypedBuilder().name("bar").build();
    registry.add(up);
    up->labels()->set(42.42);

    UntypedPtr up2 = UntypedBuilder().name("bar2").build();
    registry.add(up2);
    up2->labels()->set(1.0e-64);
    
    HistogramPtr hu = HistogramBuilder().name("baz").build();
    registry.add(hu);
    hu->labels()->observe(42);

    hu->labels({{"foo", "bar"}, {"baz", "42"}})->observe(300);
    
    TextExposition exposition;
    std::string out = exposition.collect(registry);
    std::cout << out << std::endl;
}
