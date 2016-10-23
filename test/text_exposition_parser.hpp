
#include <boost/fusion/include/adapt_struct.hpp>

#include <boost/optional.hpp>


#include <map>
#include <vector>


struct Metric {
 public:
    std::string metricName;
    std::map<std::string, std::string> kvMap;
    double metricValue;
    boost::optional<uint64_t> timestamp;
};

class Metrics {
 public:
    Metrics(std::vector<Metric> metrics) : metrics_(metrics)
    {};
    std::vector<Metric> getMetrics(const std::string& metricName)
    {
        std::vector<Metric> result;
        std::copy_if(metrics_.begin(), metrics_.end(), std::back_inserter(result), [metricName](const Metric& val) {
                return (val.metricName == metricName);
            });
        return result;
    }
    std::vector<Metric> getAllMetrics() { return metrics_; }
 private:
    std::vector<Metric> metrics_;
};

class MetricsParser {
 public:
    static Metrics parseMetrics(const std::string text);
};
