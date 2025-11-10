/**
 * LLM Integration for ESP32-S3-Touch-LCD-1.85
 * 
 * このファイルは、ESP32-S3でローカルLLMまたはクラウドベースのLLMを
 * 統合するためのインターフェースを提供します。
 * 
 * ESP32-S3のメモリ制約:
 * - SRAM: 512KB
 * - PSRAM: 8MB
 * - Flash: 16MB
 * 
 * オプション:
 * 1. 超軽量ローカルLLM (限定的)
 * 2. クラウドLLM API (OpenAI, Claude, Gemini など)
 * 3. ローカルサーバー上のLLM (Ollama, LM Studio など)
 */

#ifndef LLM_HANDLER_H
#define LLM_HANDLER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// LLM統合タイプ
enum LLMType {
    LLM_NONE,           // LLM無効
    LLM_CLOUD_OPENAI,   // OpenAI API
    LLM_CLOUD_CLAUDE,   // Anthropic Claude
    LLM_CLOUD_GEMINI,   // Google Gemini
    LLM_LOCAL_SERVER,   // ローカルサーバー (Ollama等)
    LLM_MICRO_LOCAL     // ESP32上の超軽量モデル(実験的)
};

class LLMHandler {
private:
    LLMType llm_type;
    String api_key;
    String api_endpoint;
    String model_name;
    
    WiFiClient wifi_client;
    HTTPClient http_client;
    
    // 会話履歴 (メモリ制約のため最近のN件のみ)
    static const int MAX_HISTORY = 5;
    String conversation_history[MAX_HISTORY * 2]; // user, assistant pairs
    int history_count;
    
    // キャラクター設定
    String system_prompt;
    
public:
    LLMHandler();
    ~LLMHandler();
    
    // WiFi接続
    bool connectWiFi(const char* ssid, const char* password);
    bool isConnected();
    
    // LLM設定
    void setLLMType(LLMType type);
    void setAPIKey(const String& key);
    void setEndpoint(const String& endpoint);
    void setModelName(const String& model);
    void setSystemPrompt(const String& prompt);
    
    // 会話処理
    String chat(const String& user_message);
    void clearHistory();
    
    // プリセットプロンプト
    void setupKirbyPersonality();
    void setupCuteAssistant();
    
private:
    String sendCloudRequest(const String& message);
    String sendLocalRequest(const String& message);
    String processMicroLocal(const String& message);
    
    void addToHistory(const String& user_msg, const String& assistant_msg);
    String buildPrompt(const String& current_message);
    
    // JSON処理
    String parseOpenAIResponse(const String& response);
    String parseClaudeResponse(const String& response);
    String parseGeminiResponse(const String& response);
    String parseOllamaResponse(const String& response);
};

// WiFi設定ヘルパー
struct WiFiCredentials {
    const char* ssid;
    const char* password;
};

// 設定例
namespace LLMConfig {
    // WiFi設定 (実際の値に変更してください)
    const WiFiCredentials WIFI_CREDS = {
        "YOUR_WIFI_SSID",
        "YOUR_WIFI_PASSWORD"
    };
    
    // OpenAI設定
    const char* OPENAI_API_KEY = "sk-your-api-key-here";
    const char* OPENAI_ENDPOINT = "https://api.openai.com/v1/chat/completions";
    const char* OPENAI_MODEL = "gpt-3.5-turbo";
    
    // Claude設定
    const char* CLAUDE_API_KEY = "sk-ant-your-api-key-here";
    const char* CLAUDE_ENDPOINT = "https://api.anthropic.com/v1/messages";
    const char* CLAUDE_MODEL = "claude-3-haiku-20240307";
    
    // Gemini設定
    const char* GEMINI_API_KEY = "your-gemini-api-key";
    const char* GEMINI_ENDPOINT = "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent";
    
    // ローカルサーバー (Ollama等)
    const char* LOCAL_ENDPOINT = "http://192.168.1.100:11434/api/generate";
    const char* LOCAL_MODEL = "tinyllama";
    
    // カービィ風システムプロンプト
    const char* KIRBY_SYSTEM_PROMPT = 
        "あなたはカービィのようなかわいいキャラクターです。"
        "短く、明るく、元気に答えてください。"
        "絵文字を使って感情を表現してください。"
        "語尾は「だよ!」「なの!」「ね!」などを使ってください。"
        "返答は1-2文で簡潔に。";
}

#endif
