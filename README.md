## Prometheus c++ client library

This prometheus c++ client library tries to be as easy to use as
possible. The library only consist of header files such that the ease
of use is maximised.

The library currently implements Counters, Gauges, Histogram and
Untyped. Summaries are not implemented.

The library does not contain an http server and does only implement
the text based exposition format. Again to keep dependencies at a
minimum.
