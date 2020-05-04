/*
 * JSONService: Utilities for POSTing and GETing JSON
 *
 */

#ifndef JSONService_h
#define JSONService_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <WiFiClient.h>
//                                  Third Party Libraries
#include <ArduinoJson.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------


class ServiceDetails {
public:
  ServiceDetails() { }
  ServiceDetails(
        String theServer, int thePort = 80,
        String theUser = "", String thePass = "",
        String theApiKey = "", String theApiKeyName = "") :
    server(theServer), port(thePort), user(theUser), pass(thePass),
    apiKey(theApiKey), apiKeyName(theApiKeyName) { };

  String server = "";
  int port = 80;
  String user = "";
  String pass = "";
  String apiKey = "";
  String apiKeyName = "";
};

typedef enum requestType {GET=0, POST=1} RequestType;

class JSONService {
public:
  JSONService(ServiceDetails sd);

  DynamicJsonDocument *issueGET(String endpoint, int jsonSize, JsonDocument *filterDoc = NULL);
  DynamicJsonDocument *issuePOST(String endpoint, int jsonSize, String payload="");

private:
  ServiceDetails details;
  String _encodedAuth;

  WiFiClient *getRequest(String endpoint, RequestType type, String payload="");
  DynamicJsonDocument *getJSON(WiFiClient *client, int jsonSize, JsonDocument *filterDoc);
};

#endif  // JSONService_h