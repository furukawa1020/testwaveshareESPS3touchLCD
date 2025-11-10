// Speech Recognition and Audio Handler for ESP32-S3-Touch-LCD-1.85
// Based on ESP-SR (Espressif Speech Recognition)

#ifndef VOICE_HANDLER_H
#define VOICE_HANDLER_H

#include <Arduino.h>
#include "esp_sr_iface.h"
#include "esp_sr_models.h"
#include "Audio.h"

// Wake word: "Hi ESP"
// Commands: various cute responses

class VoiceHandler {
private:
    Audio* audio;
    bool is_awake;
    uint32_t awake_time;
    
    // Speech recognition model
    model_iface_data_t *model_data;
    const esp_mn_iface_t *multinet;
    
public:
    VoiceHandler(Audio* audioPtr);
    ~VoiceHandler();
    
    bool init();
    void loop();
    bool isAwake();
    void wake();
    void sleep();
    
    // Cute voice responses
    void playGreeting();
    void playHappy();
    void playSurprise();
    void playYes();
    void playNo();
    void playGoodbye();
    
    // Callback for recognized commands
    typedef void (*CommandCallback)(const char* command);
    void setCommandCallback(CommandCallback callback);
    
private:
    CommandCallback command_callback;
    void processCommand(const char* command);
};

// Cute sound generation
void generateCuteBeep(Audio* audio, int frequency, int duration);
void generateCuteChirp(Audio* audio);

#endif
