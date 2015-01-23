#ifndef RPCZ_API_HPP
#define RPCZ_API_HPP

#if defined(_WIN32)
  #ifdef rpcz_EXPORTS
    #define RPCZ_API __declspec(dllexport)
  #else
    #define RPCZ_API __declspec(dllimport)
  #endif
#else
  #define RPCZ_API
#endif

#endif  // RPCZ_API_HPP