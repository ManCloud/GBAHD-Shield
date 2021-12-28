#include <Arduino.h>
#include <WiFi.h>
#include <SD_MMC.h>
#include <cJSON.h>

#include "wifi_handler.h"
#include "preferences_handler.h"

#define WIFI_SSID_SIZE    ( 32U )  // 32 length
#define WIFI_PW_SIZE      ( 64U )     // 64 length
#define WIFI_CREDS_FILE     ( "/wifi_credentials.json" )


void Wifi_Handler_Class::getSTACredentials(String& ssid, String& password)
{
    Preferences_Handler.restoreWifiCredentials(ssid, password);
    Serial.println(ssid);

    if (SD_MMC.begin())
    {
        String newSSID, newPassword;

        if (SD_MMC.exists(WIFI_CREDS_FILE))
        {
            File wifi_creds = SD_MMC.open(WIFI_CREDS_FILE, "r");
            if (wifi_creds.size() < 1024U)
            {
                cJSON *creds_json = NULL;

                {
                    char buffer[1024U] = { 0 };
                    wifi_creds.readBytes(buffer, (sizeof(buffer)/sizeof(buffer[0])));
                    
                    creds_json = cJSON_Parse(buffer);
                }

                if (NULL != creds_json)
                {
                    {
                        cJSON* ssid = cJSON_GetObjectItem(creds_json, "ssid");
                        if (cJSON_IsString(ssid) && (ssid->valuestring != NULL))
                        {
                            newSSID = ssid->valuestring;
                        }
                    }
                    {
                        cJSON* password = cJSON_GetObjectItem(creds_json, "password");
                        if (cJSON_IsString(password) && (password->valuestring != NULL))
                        {
                            newPassword = password->valuestring;
                        }
                    }
                    cJSON_Delete(creds_json);

                }
                else
                {
                    Serial.println("WiFi Creds JSON was invalid");
                }
            }
            else
            {
                Serial.println("WiFi Creds JSON too big");
            }
            wifi_creds.close();
        }

        if ((ssid != newSSID) || (password != newPassword))
        {
            if ((newSSID.length() < WIFI_SSID_SIZE) && (newPassword.length() < WIFI_PW_SIZE))
            {
                Preferences_Handler.saveWifiCredentials(newSSID, newPassword);
                ssid = newSSID;
                password = newPassword;
            }
            else
            {
                Serial.println("Error: Entered SSID or Password is too long and is potentially invalid.");
            }
        }
    }
    else
    {
        Serial.println("Error: SD_MMC not available. Skip getting new Credentials...");
    }

}



void Wifi_Handler_Class::connectWifiSTA(String& ssid, String& password)
{
    WiFi.begin(ssid.c_str(), password.c_str());
}

void Wifi_Handler_Class::init()
{
    String ssid;
    String password;

    WiFi.setHostname("gbaHD");

    getSTACredentials(ssid, password);

    connectWifiSTA(ssid, password);
}

void Wifi_Handler_Class::update()
{
    static wl_status_t old_status = WL_NO_SHIELD;
    wl_status_t new_status = WiFi.status();
    if (old_status != new_status)
    {
        switch(new_status)
        {
            case WL_IDLE_STATUS:
            case WL_CONNECTED:
                Serial.println("Connection established.");
                Serial.print("IP:\t");
                Serial.println(WiFi.localIP());
                break;
            case WL_NO_SSID_AVAIL:
            case WL_CONNECT_FAILED:
            case WL_CONNECTION_LOST:
                Serial.println("Connection not available.");
                break;
            case WL_SCAN_COMPLETED:
            case WL_DISCONNECTED:
            default:
            break;
        }

        old_status = new_status;
    }
}



Wifi_Handler_Class Wifi_Handler;