/*
  PsychoClock
  ESP8266 based alarm clock with automatic time setting and music!
  
  Copyright (C) 2017  Earle F. Philhower, III

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <FS.h>

#include "webspiffs.h"

static ESP8266HTTPUpdateServer httpUpdater;
static ESP8266WebServer webSpiffsServer(8000);
static File fsUploadFile;

static void HandleIndex();
static bool handleFileRead(String path);
static void handleFileUpload();

void SetupWebSPIFFS()
{
  httpUpdater.setup(&webSpiffsServer);

  SPIFFS.begin();

  webSpiffsServer.on("/index.html", HTTP_GET, []() {
    HandleIndex();
  });
  webSpiffsServer.on("/", HTTP_GET, []() {
    HandleIndex();
  });

  webSpiffsServer.on("/format", HTTP_POST, []() {
    SPIFFS.format();
    webSpiffsServer.sendHeader("Location","/index.html");      // Redirect the client to the success page
    webSpiffsServer.send(303);
  });

  webSpiffsServer.on("/upload", HTTP_POST,                       // if the client posts to the upload page
    [](){ webSpiffsServer.send(200); },                          // Send status 200 (OK) to tell the client we are ready to receive
    handleFileUpload                                    // Receive and save the file
  );

  webSpiffsServer.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(webSpiffsServer.uri()))                  // send it if it exists
      webSpiffsServer.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });
    
  webSpiffsServer.begin();
  
}


void HandleWebSPIFFS()
{
  webSpiffsServer.handleClient();
}

static void HandleIndex()
{
  String page= "";
  page += "<html><head><title>SPIFFS Index</title></head>";
  page += "<body>" ;
  FSInfo info;
  SPIFFS.info(info);
  page += "Filesystem Information<br>";
  page += "Total Bytes: ";
  page += info.totalBytes;
  page += "<br>";
  page += "Used Bytes: ";
  page += info.usedBytes;
  page += "<br><hr><br>";
 
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    File f = dir.openFile("r");
    page += "<a href=\"" + dir.fileName() + "\">" + dir.fileName() + " (" + f.size() + ")</a><br>"; 
    f.close();
  }
  page += "<br><hr><br>";
  page += "<form method=\"post\" action=\"format\"><button type=\"submit\">Format</button></form><br>";
  page += "<form method=\"post\" action=\"upload\" enctype=\"multipart/form-data\">Upload File:  <input type=\"file\" name=\"name\"><input class=\"button\" type=\"submit\" value=\"Upload\"></form><br>";
  page += "<a href=\"/update\">Upload Firmware</a><br>";
  page += "</body></html>";
  webSpiffsServer.send(200, "text/html", page);
}


static String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

static bool handleFileRead(String path) { // send the right file to the client (if it exists)
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    webSpiffsServer.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    return true;
  }
  return false;
}

static void handleFileUpload(){ // upload a new file to the SPIFFS
  HTTPUpload& upload = webSpiffsServer.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      webSpiffsServer.sendHeader("Location","/index.html");      // Redirect the client to the success page
      webSpiffsServer.send(303);
    } else {
      webSpiffsServer.send(500, "text/plain", "500: couldn't create file");
    }
  }
}
