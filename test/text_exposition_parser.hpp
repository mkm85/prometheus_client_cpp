
#include <boost/fusion/include/adapt_struct.hpp>

#include <boost/optional.hpp>


#include <map>
#include <vector>
#include <iterator>


struct Metric {
 public:
    std::string name;
    std::map<std::string, std::string> tags;
    double value;
    boost::optional<uint64_t> timestamp;
};

class Metrics {
 public:
    Metrics(std::vector<Metric> metrics) : metrics_(metrics)
    {};
    std::vector<Metric> getMetrics(const std::string& name)
    {
        std::vector<Metric> result;
        std::copy_if(metrics_.begin(), metrics_.end(), std::back_inserter(result), [name](const Metric& val) {
                return (val.name == name);
            });
        return result;
    }
    std::vector<Metric> getAllMetrics() { return metrics_; }
    boost::optional<Metric> findMetric(const std::string& name, std::map<std::string, std::string> tags = std::map<std::string, std::string>()) {
        std::vector<Metric> result;
        std::copy_if(metrics_.begin(), metrics_.end(), std::back_inserter(result), [&](const Metric& val) {
                return (val.name == name && val.tags == tags);
            });
        if (result.size() > 0) {
            return result[0];
        }
        return boost::none;
    }
 private:
    std::vector<Metric> metrics_;
};

class MetricsParser {
 public:
    static Metrics parseMetrics(const std::string text);
};
