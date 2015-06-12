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
#include "MMA7660.h"

MMA7660 MMA(p28, p27);

#define PRV_MIN_RANGE_VALUE   -1.0f
#define PRV_MAX_RANGE_VALUE   1.0f
#define PRV_SENSOR_UNITS      "g"

#define LWM2M_ACCELEROMETER_OBJECT_ID   3313

// Resource Id's:
#define RES_MIN_RANGE_VALUE 5603
#define RES_MAX_RANCE_VALUE 5604
#define RES_SENSOR_UNITS    5701
#define RES_X_VALUE         5702
#define RES_Y_VALUE         5703
#define RES_Z_VALUE         5704

typedef struct {
    float previous_x;
    float previous_y;
    float previous_z;
} accelometer_data_t;

static float round(float v) {
    float r = v * 10.0f;
    r = (r > (floor(r) + 0.5f)) ? ceil(r) : floor(r);
    r = r / 10.0f;
    return r;
}

static uint8_t prv_set_value(lwm2m_tlv_t * tlvP, accelometer_data_t * devDataP) {
    // a simple switch structure is used to respond at the specified resource asked
    switch (tlvP->id) {
    case RES_MIN_RANGE_VALUE:
        lwm2m_tlv_encode_float(PRV_MIN_RANGE_VALUE, tlvP);
        tlvP->type = LWM2M_TYPE_RESOURCE;
        return COAP_205_CONTENT ;

    case RES_MAX_RANCE_VALUE:
        lwm2m_tlv_encode_float(PRV_MAX_RANGE_VALUE, tlvP);
        tlvP->type = LWM2M_TYPE_RESOURCE;
        return COAP_205_CONTENT ;

    case RES_SENSOR_UNITS:
        tlvP->value = (uint8_t*) PRV_SENSOR_UNITS;
        tlvP->length = strlen(PRV_SENSOR_UNITS);
        tlvP->flags = LWM2M_TLV_FLAG_STATIC_DATA;
        tlvP->type = LWM2M_TYPE_RESOURCE;
        tlvP->dataType = LWM2M_TYPE_STRING;
        return COAP_205_CONTENT ;

    case RES_X_VALUE:
        if (MMA.testConnection()) {
            lwm2m_tlv_encode_float(round(MMA.x()), tlvP);
            tlvP->type = LWM2M_TYPE_RESOURCE;
            if (0 != tlvP->length)
                return COAP_205_CONTENT ;
            else
                return COAP_500_INTERNAL_SERVER_ERROR ;
        } else {
            return COAP_503_SERVICE_UNAVAILABLE ;
        }

    case RES_Y_VALUE:
        if (MMA.testConnection()) {
            lwm2m_tlv_encode_float(round(MMA.y()), tlvP);
            tlvP->type = LWM2M_TYPE_RESOURCE;
            if (0 != tlvP->length)
                return COAP_205_CONTENT ;
            else
                return COAP_500_INTERNAL_SERVER_ERROR ;
        } else {
            return COAP_503_SERVICE_UNAVAILABLE ;
        }

    case RES_Z_VALUE:
        if (MMA.testConnection()) {
            lwm2m_tlv_encode_float(round(MMA.z()), tlvP);
            tlvP->type = LWM2M_TYPE_RESOURCE;
            if (0 != tlvP->length)
                return COAP_205_CONTENT ;
            else
                return COAP_500_INTERNAL_SERVER_ERROR ;
        } else {
            return COAP_503_SERVICE_UNAVAILABLE ;
        }

    default:
        return COAP_404_NOT_FOUND ;
    }
}

static uint8_t prv_accelerometer_read(uint16_t instanceId, int * numDataP, lwm2m_tlv_t ** dataArrayP,
        lwm2m_object_t * objectP) {
    uint8_t result;
    int i;

    // this is a single instance object
    if (instanceId != 0) {
        return COAP_404_NOT_FOUND ;
    }

    // is the server asking for the full object ?
    if (*numDataP == 0) {

        uint16_t resList[] = {
        RES_MIN_RANGE_VALUE,
        RES_MAX_RANCE_VALUE,
        RES_SENSOR_UNITS,
        RES_X_VALUE,
        RES_Y_VALUE,
        RES_Z_VALUE };
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
        result = prv_set_value((*dataArrayP) + i, (accelometer_data_t*) (objectP->userData));
        i++;
    } while (i < *numDataP && result == COAP_205_CONTENT );

    return result;
}

static void prv_accelerometer_close(lwm2m_object_t * objectP) {
    if (NULL != objectP->userData) {
        lwm2m_free(objectP->userData);
        objectP->userData = NULL;
    }
    if (NULL != objectP->instanceList) {
        lwm2m_free(objectP->instanceList);
        objectP->instanceList = NULL;
    }
}

void display_accelerometer_object(lwm2m_object_t * object) {
#ifdef WITH_LOGS
    accelometer_data_t * data = (accelometer_data_t *) object->userData;
    fprintf(stdout, "  /%u: Accelerometer object:\r\n", object->objID);
    if (NULL != data) {
        fprintf(stdout, "    previous x: %f, previous y: %f, previous z: %f\r\n", data->previous_x, data->previous_y,
                data->previous_z);
    }
#endif
}

lwm2m_object_t * get_object_accelerometer() {
    /*
     * The get_object_accelerometer function create the object itself and return a pointer to the structure that represent it.
     */
    lwm2m_object_t * accelerometerObj;

    accelerometerObj = (lwm2m_object_t *) lwm2m_malloc(sizeof(lwm2m_object_t));

    if (NULL != accelerometerObj) {
        memset(accelerometerObj, 0, sizeof(lwm2m_object_t));

        /*
         * It assigns his unique ID
         * The 3313 is the standard ID for the mandatory object "IPSO Accelerometer".
         */
        accelerometerObj->objID = LWM2M_ACCELEROMETER_OBJECT_ID;

        /*
         * there is only one instance of accelerometer on the Application Board for mbed NXP LPC1768.
         *
         */
        accelerometerObj->instanceList = (lwm2m_list_t *) lwm2m_malloc(sizeof(lwm2m_list_t));
        if (NULL != accelerometerObj->instanceList) {
            memset(accelerometerObj->instanceList, 0, sizeof(lwm2m_list_t));
        } else {
            lwm2m_free(accelerometerObj);
            return NULL;
        }

        /*
         * And the private function that will access the object.
         * Those function will be called when a read/write/execute query is made by the server. In fact the library don't need to
         * know the resources of the object, only the server does.
         */
        accelerometerObj->readFunc = prv_accelerometer_read;
        accelerometerObj->writeFunc = NULL;
        accelerometerObj->executeFunc = NULL;
        accelerometerObj->closeFunc = prv_accelerometer_close;
        accelerometerObj->userData = lwm2m_malloc(sizeof(accelometer_data_t));

        /*
         * Also some user data can be stored in the object with a private structure containing the needed variables 
         */
        if (NULL != accelerometerObj->userData) {
            ((accelometer_data_t*) accelerometerObj->userData)->previous_x = -1;
            ((accelometer_data_t*) accelerometerObj->userData)->previous_y = -1;
            ((accelometer_data_t*) accelerometerObj->userData)->previous_z = -1;
        } else {
            lwm2m_free(accelerometerObj);
            accelerometerObj = NULL;
        }
    }

    return accelerometerObj;
}
