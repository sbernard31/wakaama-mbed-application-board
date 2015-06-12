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
 *    Simon Bernard
 *******************************************************************************/

#include "liblwm2m.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mbed.h"
#include "dbg.h"
#include "LM75B.h"

#define LWM2M_TEMPERATURE_OBJECT_ID   3303
#define PRV_TEMPERATURE_SENSOR_UNITS    "Cel"

// Resource Id's:
#define RES_SENSOR_VALUE    5700
#define RES_SENSOR_UNITS    5701

LM75B sensor(p28, p27);
bool opened = false;

static uint8_t prv_set_value(lwm2m_tlv_t * tlvP) {
    // a simple switch structure is used to respond at the specified resource asked
    switch (tlvP->id) {
    case RES_SENSOR_VALUE:
        lwm2m_tlv_encode_float((float) sensor, tlvP);
        tlvP->type = LWM2M_TYPE_RESOURCE;
        return COAP_205_CONTENT ;

    case RES_SENSOR_UNITS:
        tlvP->value = (uint8_t*) PRV_TEMPERATURE_SENSOR_UNITS;
        tlvP->length = strlen(PRV_TEMPERATURE_SENSOR_UNITS);
        tlvP->flags = LWM2M_TLV_FLAG_STATIC_DATA;
        tlvP->type = LWM2M_TYPE_RESOURCE;
        tlvP->dataType = LWM2M_TYPE_STRING;
        return COAP_205_CONTENT ;

    default:
        return COAP_404_NOT_FOUND ;
    }
}

static uint8_t prv_temperature_read(uint16_t instanceId, int * numDataP, lwm2m_tlv_t ** dataArrayP,
        lwm2m_object_t * objectP) {
    uint8_t result;
    int i;

    // this is a single instance object
    if (instanceId != 0) {
        return COAP_404_NOT_FOUND ;
    }

    // is the server asking for the full object ?
    if (*numDataP == 0) {

        uint16_t resList[] = { RES_SENSOR_VALUE,
        RES_SENSOR_UNITS, };
        int nbRes = sizeof(resList) / sizeof(uint16_t);

        *dataArrayP = lwm2m_tlv_new(nbRes);
        if (*dataArrayP == NULL)
            return COAP_500_INTERNAL_SERVER_ERROR ;
        *numDataP = nbRes;
        for (i = 0; i < nbRes; i++) {
            (*dataArrayP)[i].id = resList[i];
        }
    }

    i = 0;
    do {
        result = prv_set_value((*dataArrayP) + i);
        i++;
    } while (i < *numDataP && result == COAP_205_CONTENT );

    return result;
}

static void prv_temperature_close(lwm2m_object_t * objectP) {
    if (NULL != objectP->instanceList) {
        lwm2m_free(objectP->instanceList);
        objectP->instanceList = NULL;
    }
}

lwm2m_object_t * get_object_temperature() {
    /*
     * The get_object_tem function create the object itself and return a pointer to the structure that represent it.
     */
    lwm2m_object_t * temperatureObj;

    if (!sensor.open()) {
        ERR("Unable to open temperature sensor.");
        return NULL;
    }
    opened = true;

    temperatureObj = (lwm2m_object_t *) lwm2m_malloc(sizeof(lwm2m_object_t));

    if (NULL != temperatureObj) {
        memset(temperatureObj, 0, sizeof(lwm2m_object_t));

        /*
         * It assigns his unique ID
         * The 3313 is the standard ID for the mandatory object "IPSO Accelerometer".
         */
        temperatureObj->objID = LWM2M_TEMPERATURE_OBJECT_ID;

        /*
         * there is only one instance of accelerometer on the Application Board for mbed NXP LPC1768.
         *
         */
        temperatureObj->instanceList = (lwm2m_list_t *) lwm2m_malloc(sizeof(lwm2m_list_t));
        if (NULL != temperatureObj->instanceList) {
            memset(temperatureObj->instanceList, 0, sizeof(lwm2m_list_t));
        } else {
            lwm2m_free(temperatureObj);
            return NULL;
        }

        /*
         * And the private function that will access the object.
         * Those function will be called when a read/write/execute query is made by the server. In fact the library don't need to
         * know the resources of the object, only the server does.
         */
        temperatureObj->readFunc = prv_temperature_read;
        temperatureObj->writeFunc = NULL;
        temperatureObj->executeFunc = NULL;
        temperatureObj->closeFunc = prv_temperature_close;
        temperatureObj->userData = NULL;

    }

    return temperatureObj;
}

bool isTempSensorOpened() {
    return opened;
}

float getCurrentTemp() {
    return (float) sensor;
}
