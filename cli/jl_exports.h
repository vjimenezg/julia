// This file is a part of Julia. License is MIT: https://julialang.org/license
//
// Bring in the curated lists of exported data and function symbols, then
// perform C preprocessor magic upon them to generate lists of declarations and
// functions to re-export our function symbols from libjulia-internal to libjulia.
#include "../src/jl_exported_data.inc"
#include "../src/jl_exported_funcs.inc"


// Build assembler-dependent jump definitions for our function tramplite
#if defined(_CPU_X86_64_)
#define JL_JUMPTO(x)   __asm__("jmpq *%0" : : "a"(x) :)
#elif defined(_CPU_X86_)
#define JL_JUMPTO(x)   __asm__("jmpl *%0" : : "a"(x) :)
#elif defined(_CPU_AARCH64_)
#define JL_JUMPTO(x)   __asm__("br *%0" : : (x) :)
#elif defined(_CPU_ARM_)
#define JL_JUMPTO(x)   __asm__("br *%0" : : (x) :)
#elif defined(_ARCH_PPC_)
#define JL_JUMPTO(x)   __asm__("ljmp *%0" : : (x) :)
#elif defined(__wasm__)
#define JL_JUMPTO(x)   __asm__("longjmp *%0" : : (x) :)
#else
#warning "Symbol re-export trampoline support must be defined for this CPU to enable C linking against libjulia"
#undef JL_EXPORTED_DATA
#undef JL_EXPORTED_FUNCS
#define JL_EXPORTED_DATA(x)
#define JL_EXPORTED_FUNCS(x)
#endif


// Define data symbols as `const void * $(name);`
#define XX(name)    JL_DLLEXPORT const void * name;
JL_EXPORTED_DATA(XX)
#undef XX

// Define holder locations for function addresses as `const void * $(name)_addr`
#define XX(name)    JL_HIDDEN const void * name##_addr; \
                    JL_DLLEXPORT JL_NAKED void name(void) { JL_JUMPTO(name##_addr); }
JL_EXPORTED_FUNCS(XX)
#undef XX

// Generate lists of function names and addresses
#define XX(name)    #name,
static const char *const jl_exported_func_names[] = {
    JL_EXPORTED_FUNCS(XX)
    NULL
};
#undef XX

#define XX(name)    &name##_addr,
static const void ** jl_exported_func_addrs[] = {
    JL_EXPORTED_FUNCS(XX)
    NULL
};
#undef XX
