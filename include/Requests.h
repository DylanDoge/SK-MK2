#ifndef REQUESTS_H
#define REQUESTS_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "Filesystem.h"

class Requests
{
private:
    HTTPClient http;
    WiFiClientSecure client;
    String contentResponse = "";
    char *contentResponseChar;
public:
    Requests() {}

    Requests(const char *digicert_root_ca);

    void begin(String url);

    int send(const char *method, String body);

    void addAuthHeader(String accessToken);

    void addFetchAuthHeader(String authorization);

    void clearResponse();

    char* getResponse();

    String getResponseString();

    bool getFile(String url, const char *filename);

};

#endif