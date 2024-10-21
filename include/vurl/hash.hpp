#pragma once

#include <cstddef>
#include <cstdint>

class Hasher {
public:
    inline void U32(uint32_t v) {
        hash = (hash * 805306457u) ^ v;
    }

    inline void S32(int32_t v) {
        U32((uint32_t)v);
    }

    inline void F32(float v) {
        union {
            float f32;
            uint32_t u32;
        } u;
        u.f32 = v;
        U32(u.u32);
    }

    inline void Ptr(void* ptr) {
        union {
            void* ptr;
            uint32_t u32[2];
        } u;
        u.ptr = ptr;
        U32(u.u32[0]);
        U32(u.u32[1]);
    }

    inline void String(const char* data, size_t size) {
        for (size_t i = 0; i < size; ++i) {
            hash ^= data[i];
            hash *= 16777619u;
        }
    }

    template<typename T>
    inline void Data(const T* data, size_t size) {
        String((const char*)data, size * sizeof(T));
    }

    inline uint32_t Get() const { return hash; }

private:
    uint32_t hash = 01610612741u;
};

class HashedObject {
public:
    virtual uint32_t GetHash() const = 0;
};