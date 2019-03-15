/* Arduino Includes */

#include <Keyboard.h>
#include <AStar32U4.h>
#include <MFRC522.h>

/* RAAT Includes */

#include "raat.hpp"

#include "string-param.hpp"

#include "adafruit-neopixel-raat.hpp"
#include "rfid-rc522.hpp"

/* Application Includes */

#include "leds.h"

/* Defines, typedefs, constants */
enum rfid_check_result
{
    NO_RFID,
    RFID_MATCH,
    RFID_NO_MATCH
};

static const char CHARACTERS[] = "abcdefghijklmnopqrstuvwxyz";
static const char NO_MATCH_CHARACTER = 'Z';

/* Private Variables */

/* Private Functions */

static bool check_rfid(RFID_RC522 * pRFIDDevice, StringParam * pRFIDParam, uint8_t i)
{
    char uuid[20] = {NULL};
    int len1;

    len1 = pRFIDDevice->get(uuid);
    return pRFIDParam->strncmp(uuid, len1) == 0;
}

static void check_program_flag(
    RFID_RC522 * pRFIDDevice,
    StringParam * pStoredRFIDParams[NUMBER_OF_RFID_TAGS],
    IntegerParam * pRFIDToProgramParam,
    uint8_t i)
{
    int32_t to_program = pRFIDToProgramParam->get();
    char uuid[20];
    uint8_t uuid_length = 0;

    if (to_program == (i+1))
    {
        raat_logln(LOG_APP, "Waiting for RFID %d", to_program);
        while(uuid_length == 0)
        {

            uuid_length = pRFIDDevice->get(uuid);
            if (uuid_length)
            {
                raat_logln(LOG_APP, "Saved RFID %lu: <%s>", to_program, uuid);
                pStoredRFIDParams[i]->set(uuid);
                pStoredRFIDParams[i]->save();
            }
        }
        pRFIDToProgramParam->set(0);
    }
}

/* RAAT Functions */

void raat_custom_setup(const raat_devices_struct& devices, const raat_params_struct& params)
{
    char uuid[20];
    for (uint8_t i=0; i<NUMBER_OF_RFID_TAGS; i++)
    {
        params.pSavedRFID[i]->get(uuid);
        if (strlen(uuid))
        {
            raat_logln(LOG_APP, "Saved RFID %u: <%s>", i+1, uuid);
        }
        else
        {
            raat_logln(LOG_APP, "No saved RFID %u", i+1);
        }
    }
    leds_setup(devices.pLEDs);
}

void raat_custom_loop(const raat_devices_struct& devices, const raat_params_struct& params)
{
    bool analyze_button_pressed = devices.pAnalyzeButton->check_low_and_clear();
    for (uint8_t i=0; i<NUMBER_OF_RFID_TAGS; i++)
    {
        if (analyze_button_pressed)
        {
            switch (check_rfid(devices.pRFID_Device, params.pSavedRFID[i], i))
            {
            case RFID_MATCH:
                Keyboard.press(CHARACTERS[i]);
                Keyboard.release(CHARACTERS[i]);
                leds_start_match_animation();
                break;
            case RFID_NO_MATCH:
                Keyboard.press(NO_MATCH_CHARACTER);
                Keyboard.release(NO_MATCH_CHARACTER);
                leds_start_no_match_animation();
                break;
            case NO_RFID:
                break;
            default:
                break;            
            }
        }
    }

    leds_run(devices.pLEDs);
}