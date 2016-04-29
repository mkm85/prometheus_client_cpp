#include <prometheus/text_exposition.hpp>

#include <boost/variant.hpp>

namespace prometheus {

std::string TextExposition::collect(CollectorRegistry& registry)
{
    std::stringstream result;
    for (auto c : registry.collectors_) {
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

void Encoder::operator()(const CounterSample& counter) const {
    ss_ << name_;
    printLabels();
    ss_ << " " << counter.value;
    ss_ << "\n";
}
void Encoder::operator()(const GaugeSample& gauge) const {
    ss_ << name_;
    printLabels();
    ss_ << " " << gauge.value << "\n";  
}
void Encoder::operator()(const HistogramSample& histogram) const {
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
void Encoder::operator()(const SummarySample& summary) const {
}
void Encoder::operator()(const UntypedSample& untyped) const
{
    ss_ << name_;
    printLabels();
    ss_ << " " << untyped.value << "\n";   
}

void Encoder::printLabels(Labels labels) const
{
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
void Encoder::printLabels() const
{
    printLabels(Labels());
}

void Encoder::printDouble(std::stringstream& ss, double val) const
{
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

} // namespace
