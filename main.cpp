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
#ifndef __MODULE__
#define __MODULE__ "main.cpp"
#endif

#include "mbed.h"
#include "EthernetInterface.h"
#include "C12832.h"
#include "object_accelerometer.cpp"
#include "dbg.h"

extern "C" {
#include "wakaama/liblwm2m.h"
}

C12832 lcd(p5, p7, p6, p8, p11);

extern "C" {
extern lwm2m_object_t * get_object_device();
extern lwm2m_object_t * get_object_firmware();
extern lwm2m_object_t * get_server_object(int serverId, const char* binding, int lifetime, bool storing);
extern lwm2m_object_t * get_security_object(int serverId, const char* serverUri, bool isBootstrap);
extern char * get_server_uri(lwm2m_object_t * objectP, uint16_t serverID);
}

// a linked logical connection to a server
typedef struct _connection_t {
    struct _connection_t * next;
    char * host;
    int port;
    Endpoint ep;
} connection_t;

DigitalOut myled(LED1);

// the server UDP socket
UDPSocket udp;

void ethSetup() {
    EthernetInterface eth;
    eth.init(); //Use DHCP
    eth.connect();
    INFO("IP Address is %s", eth.getIPAddress());

    udp.init();
    udp.bind(5683);

    udp.set_blocking(false, 1000);
}

// globals for accessing configuration
lwm2m_context_t * lwm2mH = NULL;
lwm2m_object_t * securityObjP;
lwm2m_object_t * serverObject;
connection_t * connList;

/* create a new connection to a server */
static void * prv_connect_server(uint16_t serverID, void * userData) {
    char * host;
    char * portStr;
    char * ptr;
    int port;
    connection_t * connP = NULL;

    INFO("Create connection for server %d", serverID);

    // need to created a connection to the server identified by server ID
    char* uri = get_server_uri(securityObjP, serverID);

    if (uri == NULL) {
        INFO("server %d not found in security object", serverID);
        return NULL;
    }
    INFO("URI: %s", uri);

    // parse uri in the form "coaps://[host]:[port]"
    if (0 == strncmp(uri, "coap://", strlen("coap://"))) {
        host = uri + strlen("coap://");
    } else {
        goto exit;
    }
    portStr = strchr(host, ':');
    if (portStr == NULL)
        goto exit;
    // split strings
    *portStr = 0;
    portStr++;
    port = strtol(portStr, &ptr, 10);
    if (*ptr != 0) {
        goto exit;
    }

    INFO("Trying to connect to LWM2M Server at %s:%d", host, port);

    //  create a connection
    connP = (connection_t *) malloc(sizeof(connection_t));
    if (connP == NULL) {
        INFO("Connection creation fail (malloc)");
        goto exit;
    } else {
        connP->port = port;
        connP->host = strdup(host);
        connP->next = connList;
        connP->ep.set_address(connP->host, port);

        connList = connP;
        INFO("udp connection created");
    }
    exit: free(uri);

    return connP;
}

/* send a buffer to a session*/
static uint8_t prv_buffer_send(void * sessionH, uint8_t * buffer, size_t length, void * userdata) {
    INFO("sending");
    connection_t * connP = (connection_t*) sessionH;

    if (connP == NULL) {
        INFO("#> failed sending %u bytes, missing connection", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    INFO("sending to %s", connP->ep.get_address());

    INFO("send NO_SEC datagram");
    if (-1 == udp.sendTo(connP->ep, (char*) buffer, length)) {
        INFO("send error");
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }
    return COAP_NO_ERROR ;
}

int main() {
    int result;

    lwm2m_object_t * objArray[5];

    INFO("Start Smart watch");

    ethSetup();
    lcd.cls();

    INFO("Initialazing Wakaama");

    // create objects
    objArray[0] = get_security_object(123, "coap://10.42.0.1:5683", false);
    securityObjP = objArray[0];
    objArray[1] = get_server_object(123, "U", 40, false);
    serverObject = objArray[1];
    objArray[2] = get_object_device();
    objArray[3] = get_object_firmware();
    objArray[4] = get_object_accelerometer();

    /*
     * The liblwm2m library is now initialized with the functions that will be in
     * charge of communication
     */
    lwm2mH = lwm2m_init(prv_connect_server, prv_buffer_send, NULL);
    if (NULL == lwm2mH) {
        fprintf(stderr, "lwm2m_init() failed");
        return -1;
    }

    // configure the liblwm2m lib
    result = lwm2m_configure(lwm2mH, "smart-watch", NULL, NULL, 5, objArray);

    if (result != 0) {
        INFO("lwm2m_configure() failed: 0x%X", result);
        return -1;
    }

    // start

    result = lwm2m_start(lwm2mH);
    if (result != 0) {
        INFO("lwm2m_start() failed: 0x%X", result);
        return -1;
    }

    // main loop

    while (true) {
        char buffer[1024];
        Endpoint server;

        INFO("loop...");
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        time_t seconds = time(NULL);
        char buf[32];
        lcd.locate(0, 10);
        strftime(buf, 32, "%x : %r", localtime(&seconds));
        lcd.printf("%s", buf);

        result = lwm2m_step(lwm2mH, &timeout.tv_sec);
        if (result != 0) {
            INFO("lwm2m_step error %d", result);
        }
        int n = udp.receiveFrom(server, buffer, sizeof(buffer));
        INFO("Received packet from: %s of size %d", server.get_address(), n);
        if (n > 0) {
            // TODO: find connection
            connection_t * connP = connList;
            while (connP != NULL) {
                if (strcmp(connP->host, server.get_address()) == 0) {

                    INFO("found connection");
                    // is it a secure connection?
                    INFO("nosec session");
                    lwm2m_handle_packet(lwm2mH, (uint8_t*) buffer, n, (void*) connP);
                    break;
                }
            }
            if (connP == NULL)
                INFO("no connection");
        }
        lwm2m_uri_t URI1;
        URI1.objectId = 3313;
        //URI1.resourceId = 5702;
        lwm2m_resource_value_changed(lwm2mH, &URI1);

        /*lwm2m_uri_t URI2;
         URI2.objectId = 3313;
         URI2.resourceId = 5703;
         lwm2m_resource_value_changed(lwm2mH, &URI2);

         lwm2m_uri_t URI3;
         URI3.objectId = 3313;
         URI3.resourceId = 5704;
         lwm2m_resource_value_changed(lwm2mH, &URI3);*/
    }
}
