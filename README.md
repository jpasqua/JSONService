# JSONService
A small utlitiy library for the ESP8266 or ESP32 which makes it easy to issue GET or POST requests which return a JSON response. That response is parsed using the [ArduinoJson](https://github.com/bblanchon/ArduinoJson) library.

## Dependencies

### Libraries
The following third party libraries are used by this library:

* [Arduino-Log](https://github.com/thijse/Arduino-Log)
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) You must use at least version 6.15 which added filtering. 

#### Services
There are no dependencies on external services

## Usage Model

### Basic model
The basic model is that you provide an endpoint URL to either `issueGet` or `issuePOST` and you are handed back a pointer to a `DynamicJsonDocument` representing the returned JSON payload.

In addition to the URL, you must also provide the size of the expected JSON result as you would in other uses of the ArduinoJson library. It should be computed the same way. Here is an example of a use of `issueGet`:

````
  static const String JobStateEndpoint = "/api/job";
  static const uint32_t JobStateJSONSize =
      JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) +
      2*JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + 710;

  DynamicJsonDocument *root = service->issueGET(JobStateEndpoint, JobStateJSONSize);
  if (!root) {
    Log.warning("issueGET failed!");
    return false;
  }
  // To print the entire JSON object to the Serial port, uncomment the line below
  //serializeJsonPretty(*root, Serial); Serial.println();

  // Extract the fields of interest from the JSON object
  jobState.valid = true;
  jobState.state = (*root)["state"].as<String>();
  jobState.file.name = (*root)["job"]["file"]["name"].as<String>();
  jobState.file.size = (*root)["job"]["file"]["size"];
  
  delete root;	// It is the caller's job to delete the JSON object
````
The `issuePost` function is similar, but you may also provide a JSON payload to be delivered to the server.

### https Support

You may issue https GET requests using this library if `SSL_SUPPORT` is defined in `JSONService.h`. It is defined by default on ESP32 and not defined by default on ESP8266 due to the more limited memory available.

To use `https`, you must use port `443` in your `ServiceDetails` object. There is an additional optional `String` parameter in the `issueGet` method nameed `validation`. It is used to validate the target host. An empty `String` may be passed (which is the default). In this case, no validation of the target host is performed. If you want to validate the host, you must provide:

* ESP8266:
	* A fingerprint for the site. If an empty fingerprint is provided, the connection will be established, but with no validation. This effectively means you have no security guarantees.
	* See [this note](https://arduino-esp8266.readthedocs.io/en/2.4.0/esp8266wifi/client-secure-examples.html#get-the-fingerprint) on how to find the fingerprint
* ESP32:
	* A CA Cert for the site (you may provide the root cert). If you provide an empty cert, the connection will fail.
	* You can use this [incredibly convenient site](https://projects.petrucci.ch/esp32/?page=ssl.php) to get the site certificate (or root cert) for any site of interest.

Take a look at SSLExample.ino in the examples directory to see this in action.

### Filtering

You may wish to use a service that returns a very large JSON result - one that is too large to manage on an ESP8266. ArduinoJson added a facility to deal with some large JSON results - filtering. Please see the [description](https://arduinojson.org/news/2020/03/22/version-6-15-0/) for details.

You can apply filtering to an `issueGet` request by providing a pointer to a filter document. This parameter is normally defaulted to `NULL`, meaning no filtering.