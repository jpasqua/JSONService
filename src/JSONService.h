/*
 * JSONService: Utilities for POSTing and GETing JSON
 *
 * NOTES:
 * o This class is able to make https get requests, but the required libraries
 *   are large and the requests consume considerable heap space.
 * o By default https functionality is not enabled on an ESP8266 and is enabled on an
 *   ESP32. If you'd like to enable it on an ESP8266, uncomment the definition
 *   of SSL_SUPPORT in the #ifdef below, but it will consume a large amount of heap
 *   space and program space.
 *
 */

#ifndef JSONService_h
#define JSONService_h


#if defined(ESP32)
  #define SSL_SUPPORT 1
#else
  // #define SSL_SUPPORT 1
#endif

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <WiFiClient.h>
#if defined(SSL_SUPPORT)
  #include <WiFiClientSecure.h>
#endif
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

  /**
      Issue a GET request to the service and return a JSONDocument with the results.
      It is the caller's responsibility to dispose of the JSON object.  
      @param endpoint     The endpoint to invoke
      @param jsonSize     How much space to allocate to hold the JSON results
      @param filterDoc    If not NULL, then an ArduinoJson filter which will filter
                          the returned JSON to only the requested parts
      @param validation   - For ESP32: A CA Certificate (perhaps a root cert)
                          - For ESP8266: A fingerprint
                          - Both: If the validation string is empty OR the port is not 443, 
                          then no validation is performed
      @return A pointer to a Json document with the return data, or NULL if an error occurred.
      and "," as the half bar.
  */
  DynamicJsonDocument *issueGET(
      const String& endpoint, int jsonSize, JsonDocument *filterDoc = NULL,
      const char* validation=nullptr);
  DynamicJsonDocument *issuePOST(const String& endpoint, int jsonSize, const String& payload="");

private:
  ServiceDetails details;
  String _encodedAuth;

  WiFiClient *getRequest(
      const String& endpoint, RequestType type, const String& payload="", const char* validation = nullptr);
  DynamicJsonDocument *getJSON(WiFiClient *client, int jsonSize, JsonDocument *filterDoc);
};

#endif  // JSONService_h