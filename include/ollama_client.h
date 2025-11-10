/**
 * Ollama Local LLM Client for ESP32-S3
 * ローカルPC上のOllamaサーバーと通信
 */

#ifndef OLLAMA_CLIENT_H
#define OLLAMA_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class OllamaClient {
private:
    String server_url;
    String model_name;
    HTTPClient http;
    
    // 会話履歴（メモリ節約のため最大3ターン）
    static const int MAX_HISTORY = 3;
    String history[MAX_HISTORY * 2];
    int history_count = 0;
    
    String system_prompt;
    
public:
    OllamaClient();
    ~OllamaClient();
    
    // 初期化
    bool init(const char* server_ip, int port = 11434, const char* model = "tinyllama");
    bool testConnection();
    
    // チャット
    String chat(const String& message, bool stream = false);
    String chatWithContext(const String& message);
    
    // 履歴管理
    void clearHistory();
    void setSystemPrompt(const String& prompt);
    
    // モデル情報
    bool listModels();
    bool pullModel(const String& model);
    
private:
    String sendRequest(const String& prompt, bool include_history = true);
    void addToHistory(const String& user_msg, const String& ai_msg);
    String buildContextPrompt(const String& current_message);
};

#endif
