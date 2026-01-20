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
        case Interpolation::BILINEAR: {
            float u = handle_wrap(uv.x) * (data->width() - 1);
            float v = handle_wrap(uv.y) * (data->height() - 1);

            int x0 = static_cast<int>(std::floor(u));
            int y0 = static_cast<int>(std::floor(v));
            int x1 = std::min(x0 + 1, data->width() - 1);
            int y1 = std::min(y0 + 1, data->height() - 1);

            float tx = u - x0;
            float ty = v - y0;

            TGAColor c00 = data->get(x0, y0);
            TGAColor c10 = data->get(x1, y0);
            TGAColor c01 = data->get(x0, y1);
            TGAColor c11 = data->get(x1, y1);

            TGAColor result;
            result.bytespp = c00.bytespp;
            for (int i = 0; i < c00.bytespp; i++) {
                // 水平方向插值
                float color_top    = c00[i] + tx * (c10[i] - c00[i]);
                float color_bottom = c01[i] + tx * (c11[i] - c01[i]);
                // 垂直方向插值
                result[i] = static_cast<uint8_t>(color_top + ty * (color_bottom - color_top));
            }
            return result;
        }
    }
}
