/*
 * JSONService: Utilities for POSTing and GETing JSON
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <base64.h>
//                                  Local Includes
#include "JSONService.h"
//--------------- End:    Includes ---------------------------------------------


/*------------------------------------------------------------------------------
 *
 * Private Constants / Types
 *
 *----------------------------------------------------------------------------*/

static const char EndOfHeaders[] = "\r\n\r\n";
static const String RequestTypeNames[2] = {"GET", "PUT"};


/*------------------------------------------------------------------------------
 *
 * Private functions
 *
 *----------------------------------------------------------------------------*/

WiFiClient *JSONService::getRequest(String endpoint, RequestType type, String payload) {
  WiFiClient *client = new WiFiClient();

  if (!client->connect(details.server.c_str(), details.port)) {
    Log.warning("Connection to %s:%d failed", details.server.c_str(), details.port);
    client->stop();
    delete client;
    return NULL;
  }

  // Use HTTP/1.0, not 1.1 to avoid problems with chunking
  client->println(RequestTypeNames[type] + " " + endpoint + " HTTP/1.0");
  client->println("Host: " + details.server + ":" + String(details.port));
  client->println("Connection: close");
  if (!details.apiKey.isEmpty()) { client->println(details.apiKeyName + ": " + details.apiKey); }
  if (!_encodedAuth.isEmpty()) { client->print("Authorization: "); client->println("Basic " + _encodedAuth); }

  client->println("User-Agent: ArduinoWiFi/1.1");
  if (type == POST && !payload.isEmpty()) {
    client->println("Content-Type: application/json ");
    client->print("Content-Length: "); client->println(payload.length());
    client->println();
    client->println(payload);
  }

  if (client->println() == 0) {
    Log.warning("Headers not accepted by %s:%d", details.server.c_str(), details.port);
    delete client;
    return NULL;
  }

  // Wait for a response
  while (client->connected() && !client->available()) delay(5);

  // Check HTTP status
  char status[32] = {0};
  client->readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0 && strcmp(status, "HTTP/1.1 409 CONFLICT") != 0) {
    Log.warning("Unexpected response: %s, for %s", status, endpoint.c_str());
    Log.warning("Connecting to %s:%d", details.server.c_str(), details.port);
    if (!details.user.isEmpty())
      Log.warning("user/pass = %s:%s", details.user.c_str(), details.pass.c_str());
    if (!details.apiKeyName.isEmpty())
      Log.warning("API Key Name / API KEY = %s:%s", details.apiKeyName.c_str(), details.apiKey.c_str());
    if (!_encodedAuth.isEmpty())
      Log.warning("Auth: %s", _encodedAuth.c_str());

    while (client->connected() || client->available()) {
      if (client->available()) {
        String line = client->readStringUntil('\n');
        Serial.println(line);
      }
    }

    delete client;
    return NULL;
  }

  // Skip past HTTP headers
  if (!client->find(EndOfHeaders)) {
    Log.warning("Invalid response");
    delete client;
    return NULL;
  }

  return client;
}

DynamicJsonDocument *JSONService::getJSON(WiFiClient *client, int jsonSize, JsonDocument *filterDoc) {
  DeserializationError error;
  DynamicJsonDocument *root = new DynamicJsonDocument(jsonSize);

  if (filterDoc != NULL) { error = deserializeJson(*root, *client, DeserializationOption::Filter(*filterDoc)); }
  else { error = deserializeJson(*root, *client); }
  client->stop();

  if (error) {
    Log.warning("Unable to deserialize JSON from %s:%d", details.server.c_str(), details.port);
    Log.warning("Error = %s", error.c_str());
    Log.warning("Requested buffer size = %d, actual buffer size = %d", jsonSize, root->capacity());
    delete root;
    return NULL;
  }
  return root;
}


/*------------------------------------------------------------------------------
 *
 * Public functions
 *
 *----------------------------------------------------------------------------*/

JSONService::JSONService(ServiceDetails details) : details(details) {
  if (!details.user.isEmpty()) {
    base64 b64;
    _encodedAuth = b64.encode(details.user + ":" + details.pass, true);
  }
}

DynamicJsonDocument *JSONService::issueGET(String endpoint, int jsonSize, JsonDocument *filterDoc) {
  WiFiClient *client = getRequest(endpoint, GET);
  DynamicJsonDocument *root = NULL;
  if (client) {
    root = getJSON(client, jsonSize, filterDoc);
    delete client;
  }
  return root;
}

DynamicJsonDocument *JSONService::issuePOST(String endpoint, int jsonSize, String payload) {
  WiFiClient *client = getRequest(endpoint, POST, payload);
  DynamicJsonDocument *root = NULL;
  if (client) {
    root = getJSON(client, jsonSize, NULL);
    delete client;
  }
  return root;
}










