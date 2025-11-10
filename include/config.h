/**
 * Default Configuration
 * config.h が存在しない場合の デフォルト設定
 */

#ifndef CONFIG_H
#define CONFIG_H

// WiFi設定（ビルドエラー回避用のダミー）
#define WIFI_SSID     "dummy"
#define WIFI_PASSWORD "dummy"

// LLM無効（WiFi不要で動作）
// #define USE_LLM  // コメントアウト = LLM無効

// デフォルト設定
#define DEFAULT_BRIGHTNESS  200
#define DEFAULT_VOLUME      15
#define SERIAL_SPEED        115200

// デバッグモード
#define DEBUG_MODE

#endif
