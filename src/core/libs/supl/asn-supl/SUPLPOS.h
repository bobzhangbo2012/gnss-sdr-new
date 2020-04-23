/*
 * SPDX-FileCopyrightText: (c) 2003, 2004 Lev Walkin <vlm@lionet.info>. All rights reserved.
 * SPDX-License-Identifier: BSD-1-Clause
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "SUPL-POS"
 *     found in "../supl-pos.asn"
 */

#ifndef _SUPLPOS_H
#define _SUPLPOS_H

#include <asn_application.h>

/* Including external dependencies */
#include "PosPayLoad.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* Forward declarations */
    struct Velocity;

    /* SUPLPOS */
    typedef struct SUPLPOS
    {
        PosPayLoad_t posPayLoad;
        struct Velocity *velocity /* OPTIONAL */;
        /*
         * This type is extensible,
         * possible extensions are below.
         */

        /* Context for parsing across buffer boundaries */
        asn_struct_ctx_t _asn_ctx;
    } SUPLPOS_t;

    /* Implementation */
    extern asn_TYPE_descriptor_t asn_DEF_SUPLPOS;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "Velocity.h"

#endif /* _SUPLPOS_H_ */
#include <asn_internal.h>
