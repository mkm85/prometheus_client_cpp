#include <boost/test/unit_test.hpp>

#include "text_exposition_parser.hpp"

BOOST_AUTO_TEST_SUITE(text_exposition_parser)

BOOST_AUTO_TEST_CASE(parseSimpleMetric)
{
    const std::string input = "foobar 42";
    auto metrics = MetricsParser::parseMetrics(input);
    auto ms = metrics.getMetrics("foobar");
    BOOST_REQUIRE_EQUAL(ms.size(), 1);
    auto m = ms[0];
    BOOST_CHECK_EQUAL("foobar", m.metricName);
    BOOST_CHECK_EQUAL(m.metricValue, 42);
}

BOOST_AUTO_TEST_CASE(parseLabel)
{
    const std::string input = "foobar{foo=\"bar\"} 42";
    auto metrics = MetricsParser::parseMetrics(input);
    auto ms = metrics.getMetrics("foobar");
    BOOST_REQUIRE_EQUAL(ms.size(), 1);
    auto m = ms[0];
    BOOST_CHECK_EQUAL("foobar", m.metricName);
    BOOST_CHECK_EQUAL(m.metricValue, 42);

    BOOST_CHECK_EQUAL(m.kvMap["foo"], "bar");
}

BOOST_AUTO_TEST_CASE(parseLabels)
{
    const std::string input = "foobar{foo=\"bar\",baz=\"4242\"} 42";
    auto metrics = MetricsParser::parseMetrics(input);
    auto ms = metrics.getMetrics("foobar");
    BOOST_REQUIRE_EQUAL(ms.size(), 1);
    auto m = ms[0];
    BOOST_CHECK_EQUAL("foobar", m.metricName);
    BOOST_CHECK_EQUAL(m.metricValue, 42);

    BOOST_CHECK_EQUAL(m.kvMap["foo"], "bar");
    BOOST_CHECK_EQUAL(m.kvMap["baz"], "4242");
}

BOOST_AUTO_TEST_CASE(parseTimestamp)
{
    const std::string input = "foobar 42 424242";
    auto metrics = MetricsParser::parseMetrics(input);
    auto ms = metrics.getMetrics("foobar");
    BOOST_REQUIRE_EQUAL(ms.size(), 1);
    auto m = ms[0];
    BOOST_CHECK_EQUAL("foobar", m.metricName);
    BOOST_CHECK_EQUAL(m.metricValue, 42);
    BOOST_REQUIRE(m.timestamp);
    BOOST_CHECK_EQUAL(*(m.timestamp), 424242);
}

BOOST_AUTO_TEST_CASE(parseMetrics)
{
    const std::string input =
        "foobar 42 424242\n"
        "bar 41";
    Metrics metrics = MetricsParser::parseMetrics(input);
    BOOST_CHECK_EQUAL(metrics.getAllMetrics().size(), 2);
}

BOOST_AUTO_TEST_CASE(parseMetricsWithComments)
{
    const std::string input =
        "foobar 42 424242\n"
        "# comment\n"
        "\n"
        "bar 41\n";
    Metrics metrics = MetricsParser::parseMetrics(input);
    BOOST_CHECK_EQUAL(metrics.getAllMetrics().size(), 2);
}

BOOST_AUTO_TEST_CASE(parseMetricsWithCommentsEmptyMetrics)
{
    const std::string input =
        "# comment\n"
        "\n";
    Metrics metrics = MetricsParser::parseMetrics(input);
    BOOST_CHECK_EQUAL(metrics.getAllMetrics().size(), 0);
}




BOOST_AUTO_TEST_SUITE_END()
