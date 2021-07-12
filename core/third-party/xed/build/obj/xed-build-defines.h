#if !defined(XED_BUILD_DEFINES_H)
#  define XED_BUILD_DEFINES_H

#  if !defined(XED_AVX)
#    define XED_AVX
#  endif
#  if !defined(XED_CET)
#    define XED_CET
#  endif
#  if !defined(XED_DECODER)
#    define XED_DECODER
#  endif
#  if !defined(XED_GIT_VERSION)
#    define XED_GIT_VERSION "12.0.1-64-g428712c"
#  endif
#  if !defined(XED_MPX)
#    define XED_MPX
#  endif
#  if !defined(XED_SUPPORTS_AVX512)
#    define XED_SUPPORTS_AVX512
#  endif
#  if !defined(XED_SUPPORTS_LZCNT_TZCNT)
#    define XED_SUPPORTS_LZCNT_TZCNT
#  endif
#  if !defined(XED_SUPPORTS_SHA)
#    define XED_SUPPORTS_SHA
#  endif
#  if !defined(XED_SUPPORTS_WBNOINVD)
#    define XED_SUPPORTS_WBNOINVD
#  endif
#endif
