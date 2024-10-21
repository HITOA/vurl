#pragma once

#include <volk.h>

namespace Vurl {
    enum QueueIndices {
        QUEUE_INDEX_GRAPHICS = 0,
        QUEUE_INDEX_COMPUTE,
        QUEUE_INDEX_TRANSFER,
        QUEUE_INDEX_VIDEO_DECODE,
        QUEUE_INDEX_VIDEO_ENCODE,
        QUEUE_INDEX_MAX
    };

    struct QueueInfo {
        VkQueue queues[QUEUE_INDEX_MAX];
        uint32_t familyIndices[QUEUE_INDEX_MAX];
        uint32_t counts[QUEUE_INDEX_MAX];
    };
}