#ifndef TWS_TWS_H
#define TWS_TWS_H

#ifdef __cplusplus
#define TWS_EXTERN_C extern "C"
#else
#define TWS_EXTERN_C
#endif

#if defined(TWS_STATIC_LIBRARY)
#define TWS_EXPORT TWS_EXTERN_C
#else
#define TWS_EXPORT TWS_EXTERN_C __attribute__((visibility("default")))
#endif

TWS_EXPORT void tws_printy(void);
TWS_EXPORT int tws_divide(int);

#endif
