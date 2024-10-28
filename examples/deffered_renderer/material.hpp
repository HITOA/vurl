#pragma once


enum class AlphaMode {
    Opaque,
    Transparent
};

struct Material {
    AlphaMode alphaMode = AlphaMode::Opaque;
};