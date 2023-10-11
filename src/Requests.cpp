#include "Requests.h"

Requests::Requests(const char *digicert_root_ca)
{
    client.setCACert(digicert_root_ca);
}

void Requests::begin(String url)
{
    if (!http.begin(client, url)) 
    {
        Serial.println("E: HTTP Could not begin");
    }
}

int Requests::send(const char *method, String body)
{
    int responseCode = http.sendRequest(method, body);
    if (responseCode < 0)
    {
        Serial.println("E: HTTP respose malformed or invalid");
        Serial.println(http.errorToString(responseCode).c_str());
        responseCode = 0;
    }
    if (responseCode == 401)
    {
        Serial.println("E: HTTP Invalid Access Token");
    }
    if (responseCode == 403)
    {
        Serial.println("E: HTTP Bad OAuth request");
    }
    if (responseCode == 429)
    {
        Serial.println("E: Rate limits exceeded");
    }
    if ((method == "GET" || method == "POST") && responseCode == 200)
    {
        contentResponse = http.getString();
    }
    return responseCode;
}

void Requests::addAuthHeader(String accessToken)
{
    String auth = "Bearer " + accessToken;
    http.addHeader("Authorization", auth);
    http.addHeader("Content-Length", "0");
}

void Requests::addFetchAuthHeader(String authorization)
{
    http.addHeader("Authorization", "Basic " + authorization);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
}

void Requests::processResponse()
{
    
}

void Requests::acquireAuthToken(const char authorization[66], const char refreshToken[132])
{
    
}

void Requests::clearResponse()
{
    this->contentResponse = "";
}

char* Requests::getResponse()
{
    char *contentResponseChar = &contentResponse[0];
    return contentResponseChar;
}

bool Requests::getFile(String url, const char *filename)
{

  // If it exists then no need to fetch it
  if (LittleFS.exists(filename) == true) {
    Serial.print("Found ");
    Serial.println(filename);
    return 0;
  }

  Serial.print("Downloading ");
  Serial.print(filename);
  Serial.println(" from " + url);

  // Check WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    Serial.print("[HTTP] begin...\n");
    HTTPClient http;
    // Configure server and url
    http.begin(url);

    Serial.print("[HTTP] GET...\n");
    // Start connection and send HTTP header
    int httpCode = http.GET();
    if (httpCode == 200) {
      fs::File f = LittleFS.open(filename, "w+");
      if (!f) {
        Serial.println("file open failed");
        return 0;
      }
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // File found at server
      if (httpCode == HTTP_CODE_OK) {

        // Get length of document (is -1 when Server sends no Content-Length header)
        int total = http.getSize();
        int len = total;

        // Create buffer for read
        uint8_t buff[64] = { 0 };

        // Get tcp stream
        WiFiClient * stream = http.getStreamPtr();

        // Read all data from server
        while (http.connected() && (len > 0 || len == -1)) {
          // Get available data size
          size_t size = stream->available();

          if (size) {
            // Read up to 64 bytes
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

            // Write it to file
            f.write(buff, c);

            // Calculate remaining bytes
            if (len > 0) {
              len -= c;
            }
          }
          yield();
        }
        Serial.println();
        Serial.print("[HTTP] connection closed or file end.\n");
      }
      f.close();
    }
    else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  return 1; // File was fetched from web
}
