#include <boost/test/unit_test.hpp>

#include <prometheus/text_exposition.hpp>

#include "text_exposition_parser.hpp"

#include <iostream>

using namespace prometheus;

BOOST_AUTO_TEST_SUITE(ExpositionTests)

BOOST_AUTO_TEST_CASE(counter)
{
    CollectorRegistry registry;
    CounterPtr cf = CounterBuilder().name("foo").help("help").build();
    registry.add(cf);
    cf->labels()->inc();
    cf->labels({{"foo", "bar"}})->inc(2);

    std::string out = TextExposition::collect(registry);
    
    auto metrics = MetricsParser::parseMetrics(out);
    auto metric1 = metrics.findMetric("foo");
    auto metric2 = metrics.findMetric("foo", {{"foo", "bar"}});
    BOOST_REQUIRE(metric1);
    BOOST_CHECK_EQUAL(metric1->value, 1);

    BOOST_REQUIRE(metric2);
    BOOST_CHECK_EQUAL(metric2->value, 2);
}

BOOST_AUTO_TEST_CASE(untyped)
{
    CollectorRegistry registry;
    UntypedPtr up = UntypedBuilder().name("bar").build();
    registry.add(up);
    up->labels()->set(42.42);
    std::string out = TextExposition::collect(registry);
    auto metrics = MetricsParser::parseMetrics(out);
    auto metric1 = metrics.findMetric("bar");
    BOOST_REQUIRE(metric1);
    BOOST_CHECK_EQUAL(metric1->value, 42.42);
}

BOOST_AUTO_TEST_CASE(histogram)
{
    CollectorRegistry registry;
    HistogramPtr h = HistogramBuilder().name("bar").build();
    registry.add(h);
    h->labels()->observe(42.42);
    std::string out = TextExposition::collect(registry);

    auto metrics = MetricsParser::parseMetrics(out);
    auto count = metrics.findMetric("bar_count");
    auto sum = metrics.findMetric("bar_sum");
    BOOST_REQUIRE(count);
    BOOST_REQUIRE(sum);
    BOOST_CHECK_EQUAL(count->value, 1);
    BOOST_CHECK_EQUAL(sum->value, 42.42);
}

BOOST_AUTO_TEST_CASE(gauge)
{
    CollectorRegistry registry;
    GaugePtr g = GaugeBuilder().name("foo").build();
    registry.add(g);
    g->labels()->set(4);
    g->labels()->inc(2);
    g->labels()->dec(3);
    std::string out = TextExposition::collect(registry);
    auto metrics = MetricsParser::parseMetrics(out);
    auto metric1 = metrics.findMetric("foo");
    BOOST_REQUIRE(metric1);
    BOOST_CHECK_EQUAL(metric1->value, 3);
}

BOOST_AUTO_TEST_SUITE_END()
