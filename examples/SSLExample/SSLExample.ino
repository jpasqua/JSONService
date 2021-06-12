#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#else
  #error "Must be an ESP8266 or ESP32"
#endif
#include <ArduinoLog.h>
#include <JSONService.h>
#include "SSLExample.h"

// The Station SSID (STASSID) and the Station Password (STAPSK)
// may be defined in SSLExample.h, which is not checked into
// source control.

#ifndef STASSID
#define STASSID "MySSID"
#define STAPSK  "MyPassword"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

const char* host = "coinbase.com";
const uint16_t port = 443;
static const String JobStateEndpoint = "/v2/prices/BTC-USD/buy";
static const String validation = 
#if defined(ESP8266)
  // Use a fingerprint for coinbase
  "11 7A 9E 53 1A 1A 84 1A 04 0A B8 9E A5 40 95 87 7A 3B 43 4D";
#else // ESP32
  // Use a cert for the coinbase root certificate (Baltimore Trust)
  // Using the root makes the certicate more robust over time
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n" \
  "RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n" \
  "VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n" \
  "DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n" \
  "ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n" \
  "VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n" \
  "mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n" \
  "IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n" \
  "mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n" \
  "XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n" \
  "dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n" \
  "jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n" \
  "BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n" \
  "DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n" \
  "9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n" \
  "jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n" \
  "Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n" \
  "ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n" \
  "R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n" \
  "-----END CERTIFICATE-----\n";
#endif


static const uint32_t ReplyJSONSize = 256;
ServiceDetails details("api.coinbase.com", 443);
JSONService* service;

void flushSerial(Print *p) { p->print(CR); Serial.flush(); }

void prepLogging() {
  Serial.begin(115200);
  while (!Serial) yield();
  Serial.println(); // Separate out from the normal garbage that starts the output

  // NOTE: Log level will be adjusted when settings are read, right now it is
  // whatever is in the default settings object
  Log.begin(LOG_LEVEL_VERBOSE, &Serial, false);
  Log.setSuffix(flushSerial);
}

void setup() {
  prepLogging();

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  service = new JSONService(details);
}

void fetch() {
Serial.println("fetch: 1");
  DynamicJsonDocument *root = service->issueGET(JobStateEndpoint, ReplyJSONSize, NULL, validation);
  if (!root) {
    Serial.println("The issueGet() call failed!");
    return;
  }

  serializeJsonPretty(*root, Serial); Serial.println();
  delete root;  // It is the caller's job to delete the JSON object
}

void loop() {
  fetch();
  delay(30 * 1000L); // execute once every 30 seconds, don't flood remote service
}