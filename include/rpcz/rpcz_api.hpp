#ifndef RPCZ_API_HPP
#define RPCZ_API_HPP

#if defined(_MSC_VER) && !defined(RPCZ_STATIC)
  #ifdef rpcz_EXPORTS
    #define RPCZ_API __declspec(dllexport)
  #else
    #define RPCZ_API __declspec(dllimport)
  #endif
#else
  #define RPCZ_API
#endif

#endif  // RPCZ_API_HPP