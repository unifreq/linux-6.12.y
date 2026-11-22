/* minimal vanilla stub: BSP headers reference Android GKI KABI macros */
#ifndef _LINUX_ANDROID_KABI_H
#define _LINUX_ANDROID_KABI_H
#define __ANDROID_KABI_CHECK_SIZE_HELPER(_type, _orig_struct)
#define ANDROID_KABI_RESIZE(_type) _type ANDROID_KABI_RESERVE(1); _type ANDROID_KABI_RESERVE(2)
#define ANDROID_KABI_USE(_n, _type)
#define ANDROID_KABI_DECLSIZE(_sz)
#define ANDROID_KABI_COMPARE_ERROR(_member1, _member2)
#define ANDROID_KABI_ASSERT_EQUAL(_member1, _member2)
#define ANDROID_KABI_REPLACE(_type) _type
#define _ANDROID_KABI_REPLACE(_type) _type
#define ANDROID_KABI_REPLACE_NONE
#define _ANDROID_KABI_REPLACE_NONE
#define ANDROID_KABI_RESERVE(n)
#define ANDROID_KABI_CROCEND
#endif
