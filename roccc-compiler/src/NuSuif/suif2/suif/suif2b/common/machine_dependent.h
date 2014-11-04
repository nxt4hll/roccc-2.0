#ifndef MACHINE_DEPENDENT_H
#define MACHINE_DEPENDENT_H

// define NULL, size_t
#include <stddef.h>
#include <assert.h>
#include <limits.h>
#ifndef NULL
#define NULL 0
#endif

#ifndef SIZE_T_MAX
#define SIZE_T_MAX ((sizeof(size_t) == sizeof(unsigned long)) ? ULONG_MAX : \
                    UINT_MAX)
#endif

#endif /* MACHINE_DEPENDENT_H */
