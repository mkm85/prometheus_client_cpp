#pragma once

#include "prometheus.hpp"

#include <boost/variant.hpp>

#include <sstream>

namespace prometheus {

class Encoder : public boost::static_visitor<void> {
 public:
    Encoder(const std::string& name, const Labels& labels, std::stringstream& ss) : name_(name), labels_(labels), ss_(ss) {}
    void operator()(const CounterSample& counter) const {
        ss_ << name_;
        printLabels();
        ss_ << " " << counter.value;
        ss_ << "\n";
    }
    void operator()(const GaugeSample& gauge) const {
        ss_ << name_;
        printLabels();
        ss_ << " " << gauge.value << "\n";  
    }
    void operator()(const HistogramSample& histogram) const {
        for (auto s : histogram.buckets) {
            ss_ << name_ << "_bucket";
            std::stringstream ss;
            std::stringstream upperBound;
            printDouble(upperBound, s.upperBound);
            printLabels({ {"le", upperBound.str() } } );
            ss_ << " " << s.cumulativeCount << "\n";
        }
        ss_ << name_ << "_sum";
        printLabels();
        ss_ << " " << histogram.sum << "\n";
        
        ss_ << name_ << "_count";
        printLabels();
        ss_ << " " << histogram.count << "\n";
    }
    void operator()(const SummarySample& summary) const {
        // TODO
    }
    void operator()(const UntypedSample& untyped) const {
        ss_ << name_;
        printLabels();
        ss_ << " " << untyped.value << "\n";
    }
 private:
    const std::string& name_;
    const Labels& labels_;
    std::stringstream& ss_;
    void printLabels() const {
        printLabels(Labels());
    }
    void printLabels(Labels labels) const {
        bool first = true;
        if (labels_.empty() && labels.empty()) {
            return;
        }
        ss_ << "{";
        for (const auto& l : labels) {
            if (first) {
                first = false;
            } else {
                ss_ << ",";
            }
            ss_ << l.first << "=\"" << l.second << "\""; 
        }
        for (const auto& l : labels_) {
            if (first) {
                first = false;
            } else {
                ss_ << ",";
            }
            ss_ << l.first << "=\"" << l.second << "\""; 
        }
        ss_ << "}";
    }

    void printDouble(std::stringstream& ss, double val) const {
        if (val == INFINITY) {
            ss << "+Inf";
            return;
        }
        if (val == -INFINITY) {
            ss << "-Inf";
            return;
        }
        ss << val;
        return;
    }
};

class TextExposition {

 public:
    void printHeader(std::stringstream& ss);

    std::string collect(CollectorRegistry& registry) {
        std::stringstream result;
        for (auto c : registry.collectors) {
            for (auto m : c->collect()) {
                // print header
                for (auto s : m.samples) {
                    Encoder const encoder(m.name, s.labels, result);
                    boost::apply_visitor(encoder, s.sample);
                }
            }
        }
        return result.str();
    }
};

} //namespace
