#include "texture.h"

float Texture::handle_wrap(float v) const {
    switch (wrap) {
        case WrapMode::REPEAT: {
            return v - std::floor(v);
        }
        case WrapMode::CLAMP: {
            return std::clamp(v, 0.0f, 1.0f);
        }
        case WrapMode::MIRROR: {
            int i = (int)std::floor(v);
            float frac = v - i;
            return (std::abs(i) % 2 == 0) ? frac : (1.0f - frac);
        }
    }
    return v;
}

TGAColor Texture::sample_uv(vec2 uv) const {
    switch(mode) {
        case Interpolation::NEAREST: {
            int x = static_cast<int>(handle_wrap(uv.x) * data->width());
            int y = static_cast<int>(handle_wrap(uv.y) * data->height());
            return data->get(x, y);
        }
    }
}
