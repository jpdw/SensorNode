
#include "webserver.h"

WiFiServer server(80);

void webserver_start(){
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}


/*
  // Return the response
  wifi_client.println("HTTP/1.1 200 OK");
  wifi_client.println("Content-Type: text/html");
  wifi_client.println(""); // do not forget this one
  wifi_client.println("<!DOCTYPE HTML>");
  wifi_client.println("<html>");
*/
/*
  client.print("Led pin is now: ");

  if(value == HIGH) {
    client.println("On");
  } else {
    client.println("Off");
  }

  client.print("Fade Around is now: ");
  if(fade_around == 1) {
    client.println("On");
  } else {
    client.println("Off");
  }
*/
/*
  wifi_client.println("<br><br>");
  wifi_client.println("Click <a href=\"/LED=ON\">here</a> turn the LED on pin 2 ON<br>");
  wifi_client.println("Click <a href=\"/LED=OFF\">here</a> turn the LED on pin 2 OFF<br>");
  wifi_client.println("Click <a href=\"/RGB=RED\">here</a> turn the RGB Red ON<br>");
  wifi_client.println("Click <a href=\"/RGB=GREEN\">here</a> turn the RGB Green ON<br>");
  wifi_client.println("Click <a href=\"/RGB=BLUE\">here</a> turn the RGB Blue ON<br>");
  wifi_client.println("Click <a href=\"/FADE=RED\">here</a> turn start a Red Fade<br>");
  wifi_client.println("Click <a href=\"/AROUND=1\">here</a> to toggle Fade Around<br>");
  wifi_client.println("Click <a href=\"/RGB=RED50\">here</a> to set the RGB Red to 50%<br>");
  wifi_client.println("Click <a href=\"/RGB=RED100\">here</a> to set the RGB Red to 100%<br>");

  wifi_client.println("</html>");

  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
  */
