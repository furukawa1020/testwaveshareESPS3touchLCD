/**
 * WiFi and API Configuration Example
 * 
 * このファイルをコピーして、実際のWiFi情報とAPIキーを設定してください。
 * 
 * 手順:
 * 1. このファイルを config.h という名前でコピー
 * 2. 下記の情報を実際の値に変更
 * 3. ビルド＆アップロード
 * 
 * 注意: config.h は .gitignore に含まれているため、
 *      GitHubにプッシュされません（セキュリティのため）
 */

#ifndef CONFIG_EXAMPLE_H
#define CONFIG_EXAMPLE_H

// ===== WiFi設定 =====
// あなたのWiFiのSSIDとパスワードを入力
#define WIFI_SSID     "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"

// ===== LLM設定 =====
// 使用するLLMのタイプを選択（LLM_MICRO_LOCALは無料で使える軽量版）
#define USE_LLM              // この行をコメントアウトするとLLM機能無効
#define DEFAULT_LLM_TYPE  LLM_MICRO_LOCAL

// ----- OpenAI設定 -----
// https://platform.openai.com/api-keys でAPIキーを取得
#define OPENAI_API_KEY    "sk-your-openai-api-key-here"
#define OPENAI_MODEL      "gpt-3.5-turbo"

// ----- Anthropic Claude設定 -----
// https://console.anthropic.com/ でAPIキーを取得
#define CLAUDE_API_KEY    "sk-ant-your-claude-api-key-here"
#define CLAUDE_MODEL      "claude-3-haiku-20240307"

// ----- Google Gemini設定 -----
// https://makersuite.google.com/app/apikey でAPIキーを取得
#define GEMINI_API_KEY    "your-gemini-api-key-here"

// ----- ローカルサーバー（Ollama等）設定 -----
// ローカルでOllamaやLM Studioを実行している場合
#define LOCAL_SERVER_URL  "http://192.168.1.100:11434/api/generate"
#define LOCAL_MODEL       "tinyllama"

// ===== オーディオ設定 =====
#define DEFAULT_VOLUME    15  // 0-21
#define ENABLE_AUDIO          // この行をコメントアウトすると音声機能無効

// ===== ディスプレイ設定 =====
#define DEFAULT_BRIGHTNESS  200  // 0-255

// ===== デバッグ設定 =====
#define DEBUG_MODE            // デバッグメッセージを有効化
#define SERIAL_SPEED  115200

#endif
