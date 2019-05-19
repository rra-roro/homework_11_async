// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define OS64
#else
#define OS32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define OS64
#else
#define OS32
#endif
#endif
