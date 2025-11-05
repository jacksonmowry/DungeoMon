#include "color.h"

RGB color_blend(RGB base_color, RGBA new_color) {
    double alpha = (double)new_color.a / 255;
    return (RGB){
        .r = ((double)base_color.r * (1 - alpha)) +
             ((double)new_color.r * alpha),
        .g = ((double)base_color.g * (1 - alpha)) +
             ((double)new_color.g * alpha),
        .b = ((double)base_color.b * (1 - alpha)) +
             ((double)new_color.b * alpha),
    };
}
