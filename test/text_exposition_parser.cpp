#include "text_exposition_parser.hpp"
#include <boost/spirit/include/qi_operator.hpp>
#include <boost/spirit/include/qi_real.hpp>
#include <boost/spirit/include/qi_rule.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/spirit/include/qi_skip.hpp>
#include <boost/spirit/include/qi_omit.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_char_class.hpp>
#include <boost/spirit/include/qi_parse_auto.hpp>

#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/vector.hpp>

#include <utility>
#include <string>
#include <map>

using namespace boost::spirit;
using namespace boost::spirit::qi;

BOOST_FUSION_ADAPT_STRUCT(Metric, metricName, kvMap, metricValue, timestamp);

Metrics MetricsParser::parseMetrics(const std::string lines)
{
    typedef std::string::iterator iter_t;
    
    auto value = boost::spirit::qi::double_;
    auto timestamp = (boost::spirit::qi::skip(boost::spirit::qi::space)[-boost::spirit::qi::ulong_long]);

    auto stringValue = +(boost::spirit::qi::alnum);
    auto labelName = stringValue;
    auto labelValue = '"' >> stringValue >> '"';

    auto metricName = stringValue;

    auto labelPair = labelName >> '=' >> labelValue >> -(boost::spirit::qi::char_(','));

    auto labelPairs = ( '{' >> *labelPair >> '}' ) >> ' ' | ' ';
        
    auto metricLine = metricName >> labelPairs >> value >> timestamp;

    auto commentLine = boost::spirit::qi::char_('#') >> *(char_ - eol);

    auto line = *(boost::spirit::qi::omit[commentLine] | boost::spirit::qi::omit[eol]) >> metricLine;
    auto parser = *line;
    
    std::vector<Metric> ms;

    boost::spirit::qi::parse(lines.begin(), lines.end(), parser, ms);
    return Metrics(ms);
}
