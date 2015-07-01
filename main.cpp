/*******************************************************************************
 *
 * Copyright (c) 2013, 2014 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Julien Vermillard,
 *    Simon Bernard
 *******************************************************************************/
#include "mbed.h"
#include "EthernetInterface.h"
#include "C12832.h"
#include "SWUpdate.h"

#include "object_accelerometer.cpp"
#include "object_rgb_led.cpp"
#include "object_temperature.cpp"
#include "dbg.h"

extern "C" {
#include "wakaama/liblwm2m.h"
}

#include "wakaama/client_objects/object_firmware.h"

extern "C" {
extern lwm2m_object_t * get_object_device();
extern lwm2m_object_t * get_object_firmware();
extern lwm2m_object_t * get_server_object(int serverId, const char* binding, int lifetime, bool storing);
extern lwm2m_object_t * get_security_object(int serverId, const char* serverUri, bool isBootstrap);
extern char * get_server_uri(lwm2m_object_t * objectP, uint16_t serverID);
}

#ifndef ENDPOINT_NAME
#define ENDPOINT_NAME "lcp1768"
#endif
#ifndef LOOP_TIMEOUT
#define LOOP_TIMEOUT 1000 //ms
#endif
#ifndef SERVER_URI
#define SERVER_URI "coap://5.39.83.206:5683" // leshan sandbox : http://leshan.eclipse.org
#endif

// a linked logical lwm2m session to a server
typedef struct session_t {
    struct session_t * next;
    char * host;
    int port;
    Endpoint ep;
} session_t;

LocalFileSystem local("local");     // Create the local filesystem under the name "local"
                                    // Will be used for firmware upgrades temp files

// the lcd screen
C12832 lcd(p5, p7, p6, p8, p11);

// the server UDP socket
UDPSocket udp;

void ethSetup() {
    EthernetInterface eth;
    eth.init(); // use DHCP
    eth.connect();
    INFO("IP Address is %s", eth.getIPAddress());

    udp.init();
    udp.bind(5683);

    udp.set_blocking(false, LOOP_TIMEOUT);
}

// globals for accessing configuration
lwm2m_context_t * lwm2mH = NULL;
lwm2m_object_t * securityObjP;
lwm2m_object_t * serverObject;
session_t * sessionList;

/* create a new lwm2m session to a server */
static void * prv_connect_server(uint16_t serverID, void * userData) {
    char * host;
    char * portStr;
    int port;
    session_t * sessionP = NULL;

    INFO("Create connection for server %d", serverID);

    // need to created a connection to the server identified by server ID
    char* uri = get_server_uri(securityObjP, serverID);

    if (uri == NULL) {
        ERR("Server %d not found in security object", serverID);
        return NULL;
    }
    INFO("URI for server %d: %s", serverID, uri);

    // parse uri in the form "coap://[host]:[port]"
    if (0 == strncmp(uri, "coap://", strlen("coap://"))) {
        host = uri + strlen("coap://");
    } else {
        ERR("Only coap URI is supported: %s", uri);
        goto exit;
    }

    portStr = strchr(host, ':');
    if (portStr == NULL) {
        ERR("Port must be specify in URI: %s", uri);
        goto exit;
    }

    *portStr = 0;
    portStr++; // jump the ':' character
    char * ptr;
    port = strtol(portStr, &ptr, 10);
    if (*ptr != 0) {
        ERR("Port in URI %s should be a number", uri);
        goto exit;
    }

    INFO("Trying to connect to Server %s at %s:%d", serverID, host, port);

    //  create a connection
    sessionP = (session_t *) malloc(sizeof(session_t));
    if (sessionP == NULL) {
        ERR("Session creation failed (malloc)");
        goto exit;
    } else {
        sessionP->port = port;
        sessionP->host = strdup(host);
        sessionP->next = sessionList;
        sessionP->ep.set_address(sessionP->host, port);

        sessionList = sessionP;
        INFO("Lwm2m session created");
    }

    exit: free(uri);

    return sessionP;
}

/* send a buffer for a given session*/
static uint8_t prv_buffer_send(void * sessionH, uint8_t * buffer, size_t length, void * userdata) {
    INFO("Sending data");

    session_t * session = (session_t*) sessionH;
    if (session == NULL) {
        ERR("Failed sending %u bytes, missing lwm2m session", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    INFO("Sending %u bytes to %s", length, session->ep.get_address());
    int err = udp.sendTo(session->ep, (char*) buffer, length);
    if (err < 0) {
        ERR("Failed sending %u bytes to %s, error %d", length, session->ep.get_address(), err);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }
    return COAP_NO_ERROR ;
}

void update_firmware(lwm2m_object_t* obj) {
    INFO("*** EXEC FW UPDATE *** \n");

    char* package_url = ((firmware_data_t *) (obj->userData) )->package_url;

    INFO("*** PACKAGE URL *** %s \n", package_url);
    char file[200];
    char rootUrl[200];
    memset(file, '\0', 200);
    memset(rootUrl, '\0', 200);
    
    // Split URL in two bits: root path, and software package root filename
    if(strlen(package_url) > 0) {
        for(int i = strlen(package_url) ; i > 0 ; i--) {
            if(package_url[i-1] == '/') {
                strncpy(file, & package_url[i], strlen(& package_url[i]));
                memcpy(rootUrl, package_url, i-1);
                rootUrl[i] = '\0';
                
                INFO("[[ %s --- %s ]] \n", rootUrl, file);

                if(strlen(file) > 0) {
                    if (SWUP_OK == SoftwareUpdate(rootUrl, file));
                }

                break;
            }
        }
    }
    
}

int main() {
    INFO("Start");
    lcd.cls();
    lcd.locate(0, 10);
    lcd.printf("Starting ...");

    INFO("Ethernet Setup");
    ethSetup();

    INFO("Initializing Wakaama");
    // create objects
    lwm2m_object_t * objArray[7];
    objArray[0] = get_security_object(123, SERVER_URI, false);
    securityObjP = objArray[0];
    objArray[1] = get_server_object(123, "U", 40, false);
    serverObject = objArray[1];
    objArray[2] = get_object_device();
    objArray[3] = get_object_firmware();
    ((firmware_data_t*)objArray[3]->userData)->updatefw_function = update_firmware;

    objArray[4] = get_object_accelerometer();
    objArray[5] = get_object_rgb_led();
    objArray[6] = get_object_temperature();

    // initialize Wakaama library with the functions that will be in
    // charge of communication
    lwm2mH = lwm2m_init(prv_connect_server, prv_buffer_send, NULL);
    if (NULL == lwm2mH) {
        ERR("Wakaama initialization failed");
        return -1;
    }

    // configure wakaama
    int result;
    result = lwm2m_configure(lwm2mH, ENDPOINT_NAME, NULL, NULL, 7, objArray);
    if (result != 0) {
        ERR("Wakaama configuration failed: 0x%X", result);
        return -1;
    }

    // start Wakaama
    result = lwm2m_start(lwm2mH);
    if (result != 0) {
        INFO("Wakaama start failed: 0x%X", result);
        return -1;
    }

    INFO("Start main loop");

    // clear LCD
    lcd.cls();

    while (true) {

        INFO("loop...");

        // display current time
        time_t seconds = time(NULL);
        char buf[32];
        lcd.locate(10, 10);
        strftime(buf, 32, "%x : %r", localtime(&seconds));
        lcd.printf("%s", buf);

        // display current temperature
        lcd.locate(45, 20);
        lcd.printf("%4.2f C", getCurrentTemp());

        // perform any required pending operation
        time_t timeout = 10;
        result = lwm2m_step(lwm2mH, &timeout);
        if (result != 0) {
            INFO("Wakaama step failed : error 0x%X", result);
        }

        // read on socket
        char buffer[1024];
        Endpoint server;
        int n = udp.receiveFrom(server, buffer, sizeof(buffer));
        if (n > 0) {
            INFO("Received packet from: %s of size %d", server.get_address(), n);
            INFO("Search corresponding session...");
            session_t * session = sessionList;
            while (session != NULL) {
                if (strcmp(session->host, server.get_address()) == 0) {
                    INFO("Session found, handle packet");
                    lwm2m_handle_packet(lwm2mH, (uint8_t*) buffer, n, (void*) session);
                    break;
                }
            }
            if (session == NULL)
                INFO("No Session found, ignore packet");
        }else{
            INFO("Fire object accelerometer, temperature and time changed");
            lwm2m_uri_t accelerometer_URI;
            lwm2m_stringToUri("/3313",5, &accelerometer_URI);
            lwm2m_resource_value_changed(lwm2mH, &accelerometer_URI);

            lwm2m_uri_t temperature_URI;
            lwm2m_stringToUri("/3303/0/5700",12, &temperature_URI);
            lwm2m_resource_value_changed(lwm2mH, &temperature_URI);

            lwm2m_uri_t device_time_URI;
            lwm2m_stringToUri("/3/0/13",7, &device_time_URI);
            lwm2m_resource_value_changed(lwm2mH, &device_time_URI);

            lwm2m_uri_t switch_light_URI;
            lwm2m_stringToUri("/3311/0/5850",12, &switch_light_URI);
            lwm2m_resource_value_changed(lwm2mH, &switch_light_URI);
        }
    }
}
