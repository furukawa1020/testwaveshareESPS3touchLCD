/**
 * LVGL Configuration for ESP32-S3-Touch-LCD-1.85
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* Color depth: 16bit (RGB565) */
#define LV_COLOR_DEPTH 16

/* Swap the 2 bytes of RGB565 color. Useful if the display has an 8-bit interface (e.g. SPI) */
#define LV_COLOR_16_SWAP 0

/* Memory settings */
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (128U * 1024U)  /* 128KB for LVGL */

/* Display settings */
#define LV_HOR_RES_MAX 360
#define LV_VER_RES_MAX 360

/* DPI (Dots Per Inch) */
#define LV_DPI_DEF 130

/* Font settings */
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_28 1

/* Japanese fonts (if needed) */
#define LV_FONT_CUSTOM_DECLARE

/* Animations */
#define LV_USE_ANIMATION 1
#define LV_ANIM_TIME_DEFAULT 200

/* Asserts */
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ 0

/* Logging */
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
#define LV_LOG_PRINTF 1

/* Performance */
#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0

/* Circle drawing */
#define LV_DRAW_COMPLEX 1
#define LV_SHADOW_CACHE_SIZE 0

/* Gradient support */
#define LV_GRAD_CACHE_DEF_SIZE 0

/* Widgets */
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_BTN 1
#define LV_USE_BTNMATRIX 1
#define LV_USE_CANVAS 1
#define LV_USE_CHECKBOX 1
#define LV_USE_DROPDOWN 1
#define LV_USE_IMG 1
#define LV_USE_LABEL 1
#define LV_USE_LINE 1
#define LV_USE_ROLLER 1
#define LV_USE_SLIDER 1
#define LV_USE_SWITCH 1
#define LV_USE_TEXTAREA 1
#define LV_USE_TABLE 0

/* Extended widgets */
#define LV_USE_ANIMIMG 1
#define LV_USE_CALENDAR 0
#define LV_USE_CHART 0
#define LV_USE_COLORWHEEL 0
#define LV_USE_IMGBTN 1
#define LV_USE_KEYBOARD 0
#define LV_USE_LED 1
#define LV_USE_LIST 0
#define LV_USE_MENU 0
#define LV_USE_METER 0
#define LV_USE_MSGBOX 1
#define LV_USE_SPAN 0
#define LV_USE_SPINBOX 0
#define LV_USE_SPINNER 1
#define LV_USE_TABVIEW 0
#define LV_USE_TILEVIEW 0
#define LV_USE_WIN 0

/* Themes */
#define LV_USE_THEME_DEFAULT 1
#define LV_USE_THEME_BASIC 1

/* Layouts */
#define LV_USE_FLEX 1
#define LV_USE_GRID 0

/* Others */
#define LV_USE_SNAPSHOT 0
#define LV_USE_MONKEY 0

/* Examples */
#define LV_BUILD_EXAMPLES 0

#endif /* LV_CONF_H */
