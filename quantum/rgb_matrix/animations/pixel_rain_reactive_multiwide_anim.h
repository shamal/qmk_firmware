// Copyright 2024
// SPDX-License-Identifier: GPL-2.0+
//
// Combined effect: Pixel Rain (background) + Reactive Wide Multi (on keypress)

#ifdef ENABLE_RGB_MATRIX_PIXEL_RAIN_REACTIVE_MULTIWIDE
#    ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
RGB_MATRIX_EFFECT(PIXEL_RAIN_REACTIVE_MULTIWIDE)
#        ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#            include "effect_runner_reactive_splash.h"

// Static storage for pixel rain colors (one RGB per LED)
static RGB pixel_rain_colors[RGB_MATRIX_LED_COUNT];
static uint32_t pixel_rain_reactive_wait_timer = 0;

// Helper function to add two RGB colors (additive blending with clamping)
static RGB add_rgb_colors(RGB a, RGB b) {
    RGB result;
    result.r = qadd8(a.r, b.r);
    result.g = qadd8(a.g, b.g);
    result.b = qadd8(a.b, b.b);
    return result;
}

void PIXEL_RAIN_REACTIVE_MULTIWIDE_init(void) {
    pixel_rain_reactive_wait_timer = 0;
    // Initialize all pixel rain colors to black
    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        pixel_rain_colors[i].r = 0;
        pixel_rain_colors[i].g = 0;
        pixel_rain_colors[i].b = 0;
    }
}

// Reactive math function for colorful multi-wide effect
// Each keypress gets a different hue, creating a rainbow splash effect
static HSV PIXEL_RAIN_REACTIVE_MULTIWIDE_math(HSV hsv, int16_t dx, int16_t dy, uint8_t dist, uint16_t tick, uint8_t keypress_index) {
    // Wider spread: use dist * 3 instead of 5 to make waves spread further
    uint16_t effect = tick + dist * 3;
    if (effect > 255) effect = 255;
    
    // Each keypress gets a different base hue (rainbow effect)
    // Use keypress index to create different colors for each press
    // Multiply by 43 to get ~6 distinct colors across the spectrum
    uint8_t base_hue = scale8(keypress_index * 43, 255);
    
    // Add hue variation based on distance for gradient/rainbow effect
    // This creates a colorful gradient that spreads outward
    hsv.h = base_hue + (dist * 4) + scale8(tick, 24);
    
    // Brightness based on distance and time (wider spread with smoother falloff)
    hsv.v = qadd8(hsv.v, 255 - effect);
    hsv.s = 255; // Full saturation for vibrant colors
    
    return hsv;
}

bool PIXEL_RAIN_REACTIVE_MULTIWIDE(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Update pixel rain: randomly set one pixel at intervals
    inline uint32_t interval(void) {
        return 500 / scale16by8(qadd8(rgb_matrix_config.speed, 16), 16);
    }

    if (g_rgb_timer > pixel_rain_reactive_wait_timer) {
        uint8_t led_index = random8_max(RGB_MATRIX_LED_COUNT);
        if (HAS_ANY_FLAGS(g_led_config.flags[led_index], params->flags)) {
            HSV hsv = (random8() & 2) ? (HSV){0, 0, 0} : (HSV){random8(), random8_min_max(127, 255), rgb_matrix_config.hsv.v};
            pixel_rain_colors[led_index] = rgb_matrix_hsv_to_rgb(hsv);
        }
        pixel_rain_reactive_wait_timer = g_rgb_timer + interval();
    }

    // Render combined effect: pixel rain (base) + reactive (overlay)
    uint8_t count = g_last_hit_tracker.count;
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        // Start with pixel rain color as base
        RGB base_rgb = pixel_rain_colors[i];

        // Calculate reactive color with colorful multi-wide effect
        // Use additive blending: convert each keypress to RGB and add them
        RGB reactive_rgb = (RGB){0, 0, 0};
        for (uint8_t j = 0; j < count; j++) {
            int16_t  dx   = g_led_config.point[i].x - g_last_hit_tracker.x[j];
            int16_t  dy   = g_led_config.point[i].y - g_last_hit_tracker.y[j];
            uint8_t  dist = sqrt16(dx * dx + dy * dy);
            uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], qadd8(rgb_matrix_config.speed, 1));
            
            // Get the HSV color for this keypress (each gets different hue)
            HSV keypress_hsv = (HSV){0, 255, 0};
            keypress_hsv = PIXEL_RAIN_REACTIVE_MULTIWIDE_math(keypress_hsv, dx, dy, dist, tick, j);
            keypress_hsv.v = scale8(keypress_hsv.v, rgb_matrix_config.hsv.v);
            
            // Convert to RGB and additively blend
            RGB keypress_rgb = rgb_matrix_hsv_to_rgb(keypress_hsv);
            reactive_rgb = add_rgb_colors(reactive_rgb, keypress_rgb);
        }

        // Blend: add reactive color on top of pixel rain
        RGB final_rgb = add_rgb_colors(base_rgb, reactive_rgb);

        // Set the final color
        rgb_matrix_region_set_color(params->region, i, final_rgb.r, final_rgb.g, final_rgb.b);
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#        endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#    endif     // RGB_MATRIX_KEYREACTIVE_ENABLED
#endif         // ENABLE_RGB_MATRIX_PIXEL_RAIN_REACTIVE_MULTIWIDE

