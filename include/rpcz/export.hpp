#ifndef RPCZ_EXPORT_HPP
#define RPCZ_EXPORT_HPP

#if defined(_MSC_VER) && !defined(RPCZ_STATIC)
  #ifdef rpcz_EXPORTS
    #define RPCZ_EXPORT __declspec(dllexport)
  #else
    #define RPCZ_EXPORT __declspec(dllimport)
  #endif
#else
  #define RPCZ_EXPORT
#endif

#endif  // RPCZ_EXPORT_HPP