#include <boost/program_options.hpp>

#include <prometheus/prometheus.hpp>

#include <iostream>
#include <thread>
#include <future>

namespace po=boost::program_options;
using namespace prometheus;

class Tester {
 public:
    Tester(int threads, uint64_t n, MetricType metric) : iterations_(n), threads_(threads) {
        
    }

    void run() {
        prometheus::CounterPtr counter = prometheus::CounterBuilder().name("foo").help("bar").add();
        std::vector<decltype(std::async(std::launch::async, Tester::counterTest, counter, iterations_))> ops;
        for (int i = 0; i < threads_; i++) {
            ops.push_back(std::async(std::launch::async,
                                     Tester::counterTest, counter, iterations_));
        }
        for (auto& h : ops) {
            h.get();
        }
    }
 private:
    
    static void counterTest(CounterPtr counter, uint64_t n) {
        CounterValuePtr c = counter->labels();
        for (uint64_t i = 0; i < n; i++) {
            c->inc();
        }
    }

    uint64_t iterations_;
    int threads_;
    
};

int main(int argc, const char* argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("threads", po::value<int>()->default_value(1), "Number of threads to run test on")
        ("metric", po::value<std::string>()->default_value("counter"), "Metric type to test, (counter|gauge|histogram|untyped)")
        ("iterations", po::value<uint64_t>()->default_value(1000), "number of iterations pr thread")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }

    Tester t(vm["threads"].as<int>(),vm["iterations"].as<uint64_t>(),MetricType::COUNTER);
    t.run();
}
