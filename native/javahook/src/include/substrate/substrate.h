//substrate.h

#ifndef _SUBSTRATE_H_
#define _SUBSTRATE_H_

#ifdef __cplusplus
extern "C" {
#endif

void MSHookFunction(void *symbol, void *hook, void **old);
#if defined(__APPLE__)
void MSHookMessageEx(/*Class*/void *_class, /*SEL*/void *message, /*IMP*/void *hook, /*IMP**/void **old);
#endif

#ifdef __cplusplus
}
#endif

#endif
