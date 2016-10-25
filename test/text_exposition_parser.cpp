#include "text_exposition_parser.hpp"
#include <boost/spirit/include/qi_operator.hpp>
#include <boost/spirit/include/qi_real.hpp>
#include <boost/spirit/include/qi_rule.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/spirit/include/qi_skip.hpp>
#include <boost/spirit/include/qi_omit.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_list.hpp>
#include <boost/spirit/include/qi_char_class.hpp>
#include <boost/spirit/include/qi_parse_auto.hpp>

#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/vector.hpp>

#include <utility>
#include <string>
#include <map>


BOOST_FUSION_ADAPT_STRUCT(Metric, name, tags, value, timestamp);

Metrics MetricsParser::parseMetrics(const std::string lines)
{
    using namespace boost::spirit;
    using namespace boost::spirit::qi;

    auto quotedString = '"' >> *(char_ - char_("\"")) >> '"';
    auto metricName = +(alnum | char_("_:"));
    auto metricLabel = +(alnum | char_("_"));
        
    auto labelPair = metricLabel >> '=' >> quotedString;
    auto labelPairs = -( '{' >> (labelPair % ',') >> '}' );
        
    auto metricLine = metricName >> labelPairs >> ' ' >> double_ >> omit[*space] >> -ulong_long;

    auto commentLine = char_('#') >> *(char_ - eol);

    auto line = *(omit[commentLine] | omit[eol]) >> metricLine;
    auto parser = *line;
    
    std::vector<Metric> ms;

    parse(lines.begin(), lines.end(), parser, ms);
    return Metrics(ms);
}
