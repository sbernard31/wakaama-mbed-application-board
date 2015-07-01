/*******************************************************************************
 *
 * Copyright (c) 2015 Intel Corporation and others.
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
 *    Julien Vermillard - initial implementation
 *    Benjamin Cabé - Please refer to git log
 *    
 *******************************************************************************/

typedef struct
{
    char package_url[400];
    uint8_t state;
    uint8_t supported;
    uint8_t result;
    void (* updatefw_function) (lwm2m_object_t *);
} firmware_data_t;

