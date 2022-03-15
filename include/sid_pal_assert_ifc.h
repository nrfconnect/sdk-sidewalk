#ifndef SID_PAL_ASSERT_IFC_H
#define SID_PAL_ASSERT_IFC_H

#if defined(SID_PAL_ASSERT_DISABLED)
#   define SID_PAL_ASSERT(expression) (void)(expression)
#else
#   define SID_PAL_ASSERT(expression) \
    ((expression) ? \
        ((void)0) : \
        sid_pal_assert(__LINE__, __FILE__))

void sid_pal_assert(int line, const char * file) __attribute__((__noreturn__));

#endif


#endif // SID_PAL_ASSERT_IFC_H
