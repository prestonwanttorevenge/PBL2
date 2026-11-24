#ifndef ARC4RANDOM_H_INCLUDED
#define ARC4RANDOM_H_INCLUDED

#include <cstddef>
#include <cstdint>
#include <random>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bcrypt.h>
#ifndef BCRYPT_SUCCESS
#define BCRYPT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif
#ifndef BCRYPT_USE_SYSTEM_PREFERRED_RNG
#define BCRYPT_USE_SYSTEM_PREFERRED_RNG 0x00000002
#endif
#endif

inline void arc4random_buf(void *buf, size_t nbytes)
{
    if (buf == nullptr || nbytes == 0) {
        return;
    }

#ifdef _WIN32
    const NTSTATUS status = BCryptGenRandom(
        nullptr,
        static_cast<PUCHAR>(buf),
        static_cast<ULONG>(nbytes),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (BCRYPT_SUCCESS(status)) {
        return;
    }
#endif

    std::random_device rd;
    auto *out = static_cast<unsigned char *>(buf);
    for (size_t i = 0; i < nbytes; ++i) {
        out[i] = static_cast<unsigned char>(rd() & 0xFFu);
    }
}

inline void arc4random_init(void)
{
}

#endif // ARC4RANDOM_H_INCLUDED