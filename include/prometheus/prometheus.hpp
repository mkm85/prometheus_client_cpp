#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

#include <boost/variant.hpp>

namespace prometheus {

typedef std::map<std::string, std::string> Labels;

class CounterSample {
 public:
    double value;
};

class GaugeSample {
 public:
    double value;
};

class BucketSample {
 public:
    uint64_t cumulativeCount;
    double upperBound;
};

class HistogramSample {
 public:
    uint64_t count;
    double sum;
    std::vector<BucketSample> buckets;
};

class QuantileSample {
 public:
    double quantile;
    double value;
};

class SummarySample {
 public:
    double count;
    double sum;
    std::vector<QuantileSample> quantiles;
};

class UntypedSample {
 public:
    double value;
};

typedef boost::variant<CounterSample, GaugeSample, HistogramSample, SummarySample, UntypedSample> Sample;



class MetricFamilySample {
 public:
    Labels labels;
    Sample sample;
};

enum class MetricType { COUNTER, GAUGE, HISTOGRAM, SUMMARY, UNTYPED };

class MetricFamilySamples
{
 public:
    std::string name;
    std::string help;
    MetricType type;
    std::vector<MetricFamilySample> samples;
};

class Collector;
typedef std::shared_ptr<Collector> CollectorPtr;

class Collector
{
 public:
    virtual std::vector<MetricFamilySamples> collect() = 0;
};

class CounterValue;
typedef std::shared_ptr<CounterValue> CounterValuePtr;

class CounterValue {
 public:
    void inc() { inc(1.0); }
    void inc(double val) {
        auto current = counter_.load();
        while (!counter_.compare_exchange_weak(current, current + val)) {
        }
    }
    CounterSample collect() { return CounterSample{counter_}; }
 protected:
    std::atomic<double> counter_ = { 0.0 };
};

class GaugeValue;
typedef std::shared_ptr<GaugeValue> GaugeValuePtr;

class GaugeValue {
 public:
    void inc() { inc(1.0); }
    void inc(double val) {
        auto current = gauge_.load();
        while (!gauge_.compare_exchange_weak(current, current + val)) {
        }
    }
    void dec() { dec(1.0); }
    void dec(double val) {
        auto current = gauge_.load();
        while (!gauge_.compare_exchange_weak(current, current - val)) {
        }
    }
    void set(double val) {
        gauge_ = val;
    }
    GaugeSample collect() { return GaugeSample{gauge_}; }
 protected:
    std::atomic<double> gauge_ = { 0.0 };
};

class HistogramValue;
typedef std::shared_ptr<HistogramValue> HistogramValuePtr;

class Bucket {
 public:
    Bucket(double limit) : limit_(limit) {}
    std::atomic<uint64_t> count_ = { 0 };
    double limit_ = { 0.0 };
};

class HistogramValue {
 public:
    HistogramValue(std::vector<double> buckets) {
        for(auto b : buckets) {
            std::unique_ptr<Bucket> bucket(new Bucket(b));
            buckets_.push_back(std::move(bucket));
        }
    }

    void observe(double val) {
        for (auto& b : buckets_) {
            //put the value into all buckets which has a limit above the value
            if (b->limit_ >= val) {
                b->count_++;
            }
        }
        count_++;
        auto current = sum_.load();
        while (!sum_.compare_exchange_weak(current, current + val)) {
        }
    }
    HistogramSample collect() {
        std::vector<BucketSample> bucketSamples;
        for (auto& b : buckets_) {
            bucketSamples.push_back(BucketSample{b->count_, b->limit_});
        }
        bucketSamples.push_back(BucketSample{count_, std::numeric_limits<double>::infinity()});
        return HistogramSample{count_, sum_, bucketSamples};
    }
 protected:
    std::atomic<uint64_t> count_ = { 0 };
    std::atomic<double> sum_ = { 0.0 };
    std::vector<std::unique_ptr<Bucket>> buckets_;
};

class UntypedValue {
 public:
    void set(double value) {
        value_ = value;
    }
    UntypedSample collect() {
        return UntypedSample{value_};
    }
 protected:
    std::atomic<double> value_;
};

// Convert a ValueType to a MetricType enum
template<typename T> struct GetType { static MetricType type(); };
template<> inline MetricType GetType<CounterValue>::type()   { return MetricType::COUNTER; }
template<> inline MetricType GetType<GaugeValue>::type()     { return MetricType::GAUGE; }
template<> inline MetricType GetType<HistogramValue>::type() { return MetricType::HISTOGRAM; }
template<> inline MetricType GetType<UntypedValue>::type()   { return MetricType::UNTYPED; }


// The simple collector is a the basic class for all simple
// collectors. A simple collector is one which can just collect one
// metric type.
template<typename T>
class SimpleCollector : public Collector, public std::enable_shared_from_this<SimpleCollector<T> >
{
 public:

    static std::shared_ptr<SimpleCollector<T> > create(const std::string& name, const std::string& help, std::function< std::shared_ptr<T>(void)> typeCreator)
    {
        auto ptr = std::shared_ptr<SimpleCollector<T> >(new SimpleCollector<T>(name, help, typeCreator));
        ptr->init();
        return ptr;
    }
    void init() {
    }

    SimpleCollector(const std::string& n, const std::string& h, std::function< std::shared_ptr<T>(void)> typeCreator) : name(n), help(h), typeCreator_(typeCreator) { }
    std::shared_ptr<T> labels() {
        // return default metric without labels
        return labels(Labels());
    }

    std::shared_ptr<T> labels(Labels labels) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::shared_ptr<T> metric = metricMap[labels];
        if (!metric) {
            metric = typeCreator_();
            metricMap[labels] = metric;
        }
        return metric;
    }

    void remove(Labels labels) { metricMap.erase(labels); }
    void clear() { metricMap.clear(); }
    MetricFamilySamples collectSamples() {
        MetricFamilySamples samples {name, help, GetType<T>::type()};

        for (auto c : metricMap) {
            samples.samples.push_back(MetricFamilySample {c.first, c.second->collect() });
        }
        return samples;
    };
    std::vector<MetricFamilySamples> collect() {
        return { collectSamples() };
    }
    std::string name;
    std::string help;
    std::map<Labels, std::shared_ptr<T> > metricMap;
 private:
    std::function< std::shared_ptr<T> (void)> typeCreator_;
    std::mutex mutex_;
};


typedef SimpleCollector<CounterValue> Counter;
typedef std::shared_ptr<Counter> CounterPtr;

typedef SimpleCollector<GaugeValue> Gauge;
typedef std::shared_ptr<Gauge> GaugePtr;

typedef SimpleCollector<HistogramValue> Histogram;
typedef std::shared_ptr<Histogram> HistogramPtr;

typedef SimpleCollector<UntypedValue> Untyped;
typedef std::shared_ptr<Untyped> UntypedPtr;



class CollectorRegistry;
typedef std::shared_ptr<CollectorRegistry> CollectorRegistryPtr;

class CollectorRegistry {
 public:
    // Using add and remove since register is a reserved keyword.
    void add(CollectorPtr collector) {
        std::lock_guard<std::mutex> lock(mutex_);
        collectors.push_back(collector);
    }
    void remove(CollectorPtr collector);
    std::vector<CollectorPtr> collectors;
 private:
    std::mutex mutex_;
};

class DefaultCollectorRegistry {
 public:
    static CollectorRegistryPtr instance() {
        static std::mutex mutex_;
        static CollectorRegistryPtr registry_;
        std::lock_guard<std::mutex> lock(mutex_);
        if (!registry_) {
            registry_ = std::make_shared<CollectorRegistry>();
        }
        return registry_;
    }
};


class CounterBuilder {
 public:
    CounterBuilder name(const std::string& name) { name_ = name; return *this; }
    CounterBuilder help(const std::string& help) { help_ = help; return *this; }

    CounterPtr build() {
        return Counter::create(name_, help_, [](){ return std::make_shared<CounterValue>(); });
    }
    CounterPtr add(CollectorRegistryPtr registry = DefaultCollectorRegistry::instance()) {
        CounterPtr ptr = build();
        registry->add(ptr);
        return ptr;

    }
 private:
    std::string name_ = "default";
    std::string help_ = "default help";
};


class GaugeBuilder {
 public:
    GaugeBuilder name(const std::string& name) { name_ = name; return *this; }
    GaugeBuilder help(const std::string& help) { help_ = help; return *this; }

    GaugePtr build() {
        return Gauge::create(name_, help_, [](){ return std::make_shared<GaugeValue>(); });
    }
    GaugePtr add(CollectorRegistryPtr registry = DefaultCollectorRegistry::instance()) {
        GaugePtr ptr = build();
        registry->add(ptr);
        return ptr;

    }
 private:
    std::string name_ = "default";
    std::string help_ = "default help";
};

class HistogramBuilder {
 public:
    HistogramBuilder name(const std::string& name) { name_ = name; return *this; }
    HistogramBuilder help(const std::string& help) { help_ = help; return *this; }
    HistogramBuilder buckets(const std::vector<double>& buckets) { buckets_ = buckets; return *this; }

    HistogramPtr build() {
        std::vector<double> buckets = buckets_;
        return Histogram::create(name_, help_, [buckets](){return std::make_shared<HistogramValue>(buckets); });
    }
    HistogramPtr add(CollectorRegistryPtr registry = DefaultCollectorRegistry::instance()) {
        HistogramPtr ptr = build();
        registry->add(ptr);
        return ptr;

    }
 private:
    std::string name_ = "default";
    std::string help_ = "default help";
    std::vector<double> buckets_ = { .005, .01, .025, .05, .1, .25, .5, 1, 2.5, 5, 10 };
};

class UntypedBuilder {
 public:
    UntypedBuilder name(const std::string& name) { name_ = name; return *this; }
    UntypedBuilder help(const std::string& help) { help_ = help; return *this; }

    UntypedPtr build() {
        return Untyped::create(name_, help_, [](){ return std::make_shared<UntypedValue>(); });
    }
    UntypedPtr add(CollectorRegistryPtr registry = DefaultCollectorRegistry::instance()) {
        UntypedPtr ptr = build();
        registry->add(ptr);
        return ptr;

    }
 private:
    std::string name_ = "default";
    std::string help_ = "default help";
};



} // namespace
