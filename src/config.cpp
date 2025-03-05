//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#include "config.h"
#include <Preferences.h>
//-----------------------------------------------------------------------------

static Preferences prefs;
config_t config;

//-----------------------------------------------------------------------------
void config_init() {
    prefs.begin("ESPNOW_RX");
    
    // Check if initialized
    if (!prefs.getUChar("initialized", 0)) {
        // First time initialization
        config_commit();
    } else {
        // Load existing configuration
        size_t readSize = prefs.getBytes("config", nullptr, sizeof(config_t));
        if (readSize != sizeof(config_t)) {
            // Error in reading, restore defaults
            config_commit();
        }
    }
}

//-----------------------------------------------------------------------------
void config_task() {
    // Your periodic config tasks here
}

//-----------------------------------------------------------------------------
void config_commit() {
    prefs.putBytes("config", &config, sizeof(config_t));
    prefs.putUChar("initialized", 1);
}

//-----------------------------------------------------------------------------
void config_set_reset_flag() {
    prefs.putUChar("initialized", 0);
}