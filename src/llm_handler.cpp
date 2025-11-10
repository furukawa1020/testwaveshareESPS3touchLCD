#include "llm_handler.h"

LLMHandler::LLMHandler() {
    llm_type = LLM_NONE;
    history_count = 0;
    system_prompt = LLMConfig::KIRBY_SYSTEM_PROMPT;
}

LLMHandler::~LLMHandler() {
    if (http_client.connected()) {
        http_client.end();
    }
}

bool LLMHandler::connectWiFi(const char* ssid, const char* password) {
    Serial.print("WiFi接続中: ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi接続成功!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("\nWiFi接続失敗");
        return false;
    }
}

bool LLMHandler::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void LLMHandler::setLLMType(LLMType type) {
    llm_type = type;
}

void LLMHandler::setAPIKey(const String& key) {
    api_key = key;
}

void LLMHandler::setEndpoint(const String& endpoint) {
    api_endpoint = endpoint;
}

void LLMHandler::setModelName(const String& model) {
    model_name = model;
}

void LLMHandler::setSystemPrompt(const String& prompt) {
    system_prompt = prompt;
}

void LLMHandler::setupKirbyPersonality() {
    system_prompt = LLMConfig::KIRBY_SYSTEM_PROMPT;
}

void LLMHandler::setupCuteAssistant() {
    system_prompt = 
        "あなたはとってもかわいいAIアシスタントです。"
        "短く、楽しく、親しみやすい口調で答えてください。";
}

String LLMHandler::chat(const String& user_message) {
    if (llm_type == LLM_NONE) {
        return "LLMが設定されていません";
    }
    
    if (!isConnected() && llm_type != LLM_MICRO_LOCAL) {
        return "WiFiに接続されていません";
    }
    
    Serial.print("ユーザー: ");
    Serial.println(user_message);
    
    String response;
    
    switch (llm_type) {
        case LLM_CLOUD_OPENAI:
        case LLM_CLOUD_CLAUDE:
        case LLM_CLOUD_GEMINI:
            response = sendCloudRequest(user_message);
            break;
            
        case LLM_LOCAL_SERVER:
            response = sendLocalRequest(user_message);
            break;
            
        case LLM_MICRO_LOCAL:
            response = processMicroLocal(user_message);
            break;
            
        default:
            response = "未対応のLLMタイプです";
            break;
    }
    
    if (response.length() > 0) {
        addToHistory(user_message, response);
    }
    
    Serial.print("アシスタント: ");
    Serial.println(response);
    
    return response;
}

void LLMHandler::clearHistory() {
    history_count = 0;
    for (int i = 0; i < MAX_HISTORY * 2; i++) {
        conversation_history[i] = "";
    }
}

void LLMHandler::addToHistory(const String& user_msg, const String& assistant_msg) {
    if (history_count >= MAX_HISTORY) {
        // 古い履歴を削除
        for (int i = 0; i < (MAX_HISTORY - 1) * 2; i++) {
            conversation_history[i] = conversation_history[i + 2];
        }
        history_count = MAX_HISTORY - 1;
    }
    
    int idx = history_count * 2;
    conversation_history[idx] = user_msg;
    conversation_history[idx + 1] = assistant_msg;
    history_count++;
}

String LLMHandler::buildPrompt(const String& current_message) {
    String prompt = system_prompt + "\n\n";
    
    // 会話履歴を追加
    for (int i = 0; i < history_count; i++) {
        int idx = i * 2;
        prompt += "User: " + conversation_history[idx] + "\n";
        prompt += "Assistant: " + conversation_history[idx + 1] + "\n";
    }
    
    prompt += "User: " + current_message + "\nAssistant: ";
    return prompt;
}

String LLMHandler::sendCloudRequest(const String& message) {
    if (!http_client.begin(api_endpoint)) {
        return "HTTP接続エラー";
    }
    
    http_client.setTimeout(15000); // 15秒タイムアウト
    
    // ヘッダー設定
    http_client.addHeader("Content-Type", "application/json");
    
    if (llm_type == LLM_CLOUD_OPENAI) {
        http_client.addHeader("Authorization", "Bearer " + api_key);
    } else if (llm_type == LLM_CLOUD_CLAUDE) {
        http_client.addHeader("x-api-key", api_key);
        http_client.addHeader("anthropic-version", "2023-06-01");
    }
    
    // リクエストボディ作成
    DynamicJsonDocument doc(4096);
    
    if (llm_type == LLM_CLOUD_OPENAI) {
        doc["model"] = model_name;
        JsonArray messages = doc.createNestedArray("messages");
        
        JsonObject system_msg = messages.createNestedObject();
        system_msg["role"] = "system";
        system_msg["content"] = system_prompt;
        
        // 履歴追加
        for (int i = 0; i < history_count; i++) {
            int idx = i * 2;
            JsonObject user_msg = messages.createNestedObject();
            user_msg["role"] = "user";
            user_msg["content"] = conversation_history[idx];
            
            JsonObject asst_msg = messages.createNestedObject();
            asst_msg["role"] = "assistant";
            asst_msg["content"] = conversation_history[idx + 1];
        }
        
        JsonObject current_msg = messages.createNestedObject();
        current_msg["role"] = "user";
        current_msg["content"] = message;
        
        doc["max_tokens"] = 150;
        doc["temperature"] = 0.8;
    }
    
    String request_body;
    serializeJson(doc, request_body);
    
    Serial.println("リクエスト送信中...");
    int http_code = http_client.POST(request_body);
    
    String response;
    if (http_code > 0) {
        if (http_code == HTTP_CODE_OK) {
            response = http_client.getString();
            
            // レスポンス解析
            if (llm_type == LLM_CLOUD_OPENAI) {
                response = parseOpenAIResponse(response);
            } else if (llm_type == LLM_CLOUD_CLAUDE) {
                response = parseClaudeResponse(response);
            } else if (llm_type == LLM_CLOUD_GEMINI) {
                response = parseGeminiResponse(response);
            }
        } else {
            response = "HTTPエラー: " + String(http_code);
        }
    } else {
        response = "接続エラー";
    }
    
    http_client.end();
    return response;
}

String LLMHandler::sendLocalRequest(const String& message) {
    if (!http_client.begin(api_endpoint)) {
        return "ローカルサーバー接続エラー";
    }
    
    http_client.setTimeout(30000); // 30秒タイムアウト
    http_client.addHeader("Content-Type", "application/json");
    
    // Ollama形式のリクエスト
    DynamicJsonDocument doc(2048);
    doc["model"] = model_name;
    doc["prompt"] = buildPrompt(message);
    doc["stream"] = false;
    
    String request_body;
    serializeJson(doc, request_body);
    
    int http_code = http_client.POST(request_body);
    
    String response;
    if (http_code > 0) {
        if (http_code == HTTP_CODE_OK) {
            response = http_client.getString();
            response = parseOllamaResponse(response);
        } else {
            response = "サーバーエラー: " + String(http_code);
        }
    } else {
        response = "接続エラー";
    }
    
    http_client.end();
    return response;
}

String LLMHandler::processMicroLocal(const String& message) {
    // 超軽量なルールベースの応答
    // ESP32-S3のメモリ制約のため、完全なLLMは不可能
    
    String msg_lower = message;
    msg_lower.toLowerCase();
    
    if (msg_lower.indexOf("こんにちは") >= 0 || msg_lower.indexOf("hello") >= 0) {
        return "やっほー! 元気だよ! 🎀";
    } else if (msg_lower.indexOf("元気") >= 0) {
        return "うん! とっても元気だよ! ✨";
    } else if (msg_lower.indexOf("ありがとう") >= 0) {
        return "どういたしまして! 💕";
    } else if (msg_lower.indexOf("かわいい") >= 0) {
        return "えへへ、ありがとう! (*´▽`*) 💗";
    } else if (msg_lower.indexOf("名前") >= 0) {
        return "ぼくの名前はカビちゃんだよ! 🌸";
    } else if (msg_lower.indexOf("好き") >= 0) {
        return "わーい! ぼくも大好きだよ! 💖";
    } else if (msg_lower.indexOf("遊") >= 0) {
        return "遊ぼう遊ぼう! 何して遊ぶ? 🎮";
    } else if (msg_lower.indexOf("さようなら") >= 0 || msg_lower.indexOf("bye") >= 0) {
        return "またね! バイバイ! 👋✨";
    } else {
        return "ふむふむ、なるほどね! 😊";
    }
}

String LLMHandler::parseOpenAIResponse(const String& response) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        return "JSON解析エラー";
    }
    
    if (doc.containsKey("choices")) {
        return doc["choices"][0]["message"]["content"].as<String>();
    }
    
    return "応答の解析に失敗しました";
}

String LLMHandler::parseClaudeResponse(const String& response) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        return "JSON解析エラー";
    }
    
    if (doc.containsKey("content")) {
        return doc["content"][0]["text"].as<String>();
    }
    
    return "応答の解析に失敗しました";
}

String LLMHandler::parseGeminiResponse(const String& response) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        return "JSON解析エラー";
    }
    
    if (doc.containsKey("candidates")) {
        return doc["candidates"][0]["content"]["parts"][0]["text"].as<String>();
    }
    
    return "応答の解析に失敗しました";
}

String LLMHandler::parseOllamaResponse(const String& response) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        return "JSON解析エラー";
    }
    
    if (doc.containsKey("response")) {
        return doc["response"].as<String>();
    }
    
    return "応答の解析に失敗しました";
}
