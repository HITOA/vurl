#pragma once

#include <vurl/hash.hpp>
#include <string>
#include <vector>
#include <memory>

namespace Vurl {

    template<typename T>
    class Resource : public HashedObject {
    public:
        Resource() = delete;
        Resource(const std::string& name) : name{ name } {}
        virtual ~Resource() {};

        inline void SetSliceCount(uint32_t count) { slices.resize(count); }
        inline uint32_t GetSliceCount() const { return slices.size(); }
        inline std::shared_ptr<T> GetResourceSlice(uint32_t sliceIdx) const { return slices[sliceIdx % slices.size()]; }
        inline void SetResourceSlice(std::shared_ptr<T> slice, uint32_t sliceIdx) { slices[sliceIdx] = slice; }

        inline void SetTransient(bool transient) { isTransient = transient; }
        inline bool IsTransient() const { return isTransient; }

        inline void SetExternal(bool external) { isExternal = external; }
        inline bool IsExternal() const { return isExternal; }

        inline void SetFirstReadOperationPassIndex(uint32_t idx) { firstReadOperationPassIndex = idx; }
        inline void SetFirstWriteOperationPassIndex(uint32_t idx) { firstWriteOperationPassIndex = idx; }
        inline void SetLastReadOperationPassIndex(uint32_t idx) { lastReadOperationPassIndex = idx; }
        inline void SetLastWriteOperationPassIndex(uint32_t idx) { lastWriteOperationPassIndex = idx; }
        inline uint32_t GetFirstReadOperationPassIndex() const { return firstReadOperationPassIndex; }
        inline uint32_t GetFirstWriteOperationPassIndex() const { return firstWriteOperationPassIndex; }
        inline uint32_t GetLastReadOperationPassIndex() const { return lastReadOperationPassIndex; }
        inline uint32_t GetLastWriteOperationPassIndex() const { return lastWriteOperationPassIndex; }

        inline uint32_t GetHash() const {
            Hasher hasher{};
            for (uint32_t i = 0; i < slices.size(); ++i)
                hasher.Data(slices[i].get(), 1);
            return hasher.Get();
        };

    protected:
        std::string name;
        bool isTransient = false;
        bool isExternal = true;
        std::vector<std::shared_ptr<T>> slices{};
        uint32_t firstReadOperationPassIndex = -1;
        uint32_t firstWriteOperationPassIndex = -1;
        uint32_t lastReadOperationPassIndex = -1;
        uint32_t lastWriteOperationPassIndex = -1;
    };
}