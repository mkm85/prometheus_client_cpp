#include <boost/test/unit_test.hpp>

#include <prometheus/prometheus.hpp>
#include <prometheus/text_exposition.hpp>

#include "text_exposition_parser.hpp"

/**
 * this tests shows how to use a custom collector to collect custom
 * metrics. this could be used to collect system metrics such as
 * memory used
 */
class CustomCollector : public prometheus::Collector {
 public:
    std::vector<prometheus::MetricFamilySamples> collect() {
        std::vector<prometheus::MetricFamilySamples> metricFamilySamples;
        {
            prometheus::MetricFamilySamples memoryUsed;
            memoryUsed.name = "memory_used";
            memoryUsed.help = "System memory usage";
            memoryUsed.type = prometheus::MetricType::GAUGE;
            memoryUsed.samples = {
                prometheus::MetricFamilySample { {}, prometheus::GaugeSample { 21.0 } }
            };
            metricFamilySamples.push_back(memoryUsed);
        }
        {
            prometheus::MetricFamilySamples memoryAllocated;
            memoryAllocated.name = "memory_allocated";
            memoryAllocated.help = "memory allocated in process lifetime";
            memoryAllocated.type = prometheus::MetricType::COUNTER;
            memoryAllocated.samples = {
                prometheus::MetricFamilySample { {{"label", "value"}}, prometheus::GaugeSample { 42.0 } }
            };
            metricFamilySamples.push_back(memoryAllocated);
        }
        return metricFamilySamples;
    }
};


BOOST_AUTO_TEST_CASE(TestCustomCollector)
{
    auto customCollector = std::make_shared<CustomCollector>();
    prometheus::CollectorRegistry registry; 
    registry.add(customCollector);

    std::string out = prometheus::TextExposition::collect(registry);

    auto metrics = MetricsParser::parseMetrics(out);
    auto metric1 = metrics.findMetric("memory_used");
    auto metric2 = metrics.findMetric("memory_allocated", {{"label", "value"}});
    BOOST_REQUIRE(metric1);
    BOOST_CHECK_EQUAL(metric1->value, 21.0);

    BOOST_REQUIRE(metric2);
    BOOST_CHECK_EQUAL(metric2->value, 42.0);
    
}
