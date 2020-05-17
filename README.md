# JSONService
A small utlitiy library for the ESP8266 which makes it easy to issue GET or POST requests and return a JSON response which is parsed using the [ArduinoJson](https://github.com/bblanchon/ArduinoJson) library.

## Dependencies

### Libraries
The following third party libraries are used by this library:

* [Arduino-Log](https://github.com/thijse/Arduino-Log)
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) You must use at least version 6.15 which added filtering. 

#### Services
There are no dependencies on external services

## Usage Model

### Basic model
The basic model is that you provide an endpoint URL to etiehr `issueGet` or `issuePOST` and you are handed back a pointer to a `DynamicJsonDocument` representing the returned JSON payload.

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

### Filtering

Yu may wish to use a service that returns a very large JSON result - one that is too large to manage on an ESP8266. ArduinoJson added a facility to deal with some large JSON results - filtering. Please see the [description](https://arduinojson.org/news/2020/03/22/version-6-15-0/) for details.

You can apply filtering to an `issueGet` request by providing a pointer to a filter document. This parameter is normally defaulted to `NULL`, meaning no filtering.