// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing ( http://blog.csdn.net/jq0123 )
// Get universal milliseconds from epoch.

#ifndef RPCZ_UTC_MS_HPP
#define RPCZ_UTC_MS_HPP

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <rpcz/common.hpp>  // for uint64

namespace rpcz {

inline uint64 utc_ms() {
  static boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
  return (boost::posix_time::microsec_clock::universal_time() - epoch)
      .total_milliseconds();
}

}  // namespace rpcz
#endif  // RPCZ_UTC_MS_HPP
