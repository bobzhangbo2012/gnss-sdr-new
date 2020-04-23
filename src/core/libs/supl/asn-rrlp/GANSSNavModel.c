/*
 * SPDX-FileCopyrightText: (c) 2003, 2004 Lev Walkin <vlm@lionet.info>. All rights reserved.
 * SPDX-License-Identifier: BSD-1-Clause
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "RRLP-Components"
 *     found in "../rrlp-components.asn"
 */

#include "GANSSNavModel.h"

static int memb_nonBroadcastIndFlag_constraint_1(
    asn_TYPE_descriptor_t *td, const void *sptr,
    asn_app_constraint_failed_f *ctfailcb, void *app_key)
{
    long value;

    if (!sptr)
        {
            _ASN_CTFAIL(app_key, td, sptr, "%s: value not given (%s:%d)",
                td->name, __FILE__, __LINE__);
            return -1;
        }

    value = *(const long *)sptr;

    if ((value >= 0 && value <= 1))
        {
            /* Constraint check succeeded */
            return 0;
        }
    else
        {
            _ASN_CTFAIL(app_key, td, sptr, "%s: constraint failed (%s:%d)",
                td->name, __FILE__, __LINE__);
            return -1;
        }
}

static int memb_toeMSB_constraint_1(asn_TYPE_descriptor_t *td, const void *sptr,
    asn_app_constraint_failed_f *ctfailcb,
    void *app_key)
{
    long value;

    if (!sptr)
        {
            _ASN_CTFAIL(app_key, td, sptr, "%s: value not given (%s:%d)",
                td->name, __FILE__, __LINE__);
            return -1;
        }

    value = *(const long *)sptr;

    if ((value >= 0 && value <= 31))
        {
            /* Constraint check succeeded */
            return 0;
        }
    else
        {
            _ASN_CTFAIL(app_key, td, sptr, "%s: constraint failed (%s:%d)",
                td->name, __FILE__, __LINE__);
            return -1;
        }
}

static int memb_eMSB_constraint_1(asn_TYPE_descriptor_t *td, const void *sptr,
    asn_app_constraint_failed_f *ctfailcb,
    void *app_key)
{
    long value;

    if (!sptr)
        {
            _ASN_CTFAIL(app_key, td, sptr, "%s: value not given (%s:%d)",
                td->name, __FILE__, __LINE__);
            return -1;
        }

    value = *(const long *)sptr;

    if ((value >= 0 && value <= 127))
        {
            /* Constraint check succeeded */
            return 0;
        }
    else
        {
            _ASN_CTFAIL(app_key, td, sptr, "%s: constraint failed (%s:%d)",
                td->name, __FILE__, __LINE__);
            return -1;
        }
}

static int memb_sqrtAMBS_constraint_1(asn_TYPE_descriptor_t *td,
    const void *sptr,
    asn_app_constraint_failed_f *ctfailcb,
    void *app_key)
{
    long value;

    if (!sptr)
        {
            _ASN_CTFAIL(app_key, td, sptr, "%s: value not given (%s:%d)",
                td->name, __FILE__, __LINE__);
            return -1;
        }

    value = *(const long *)sptr;

    if ((value >= 0 && value <= 63))
        {
            /* Constraint check succeeded */
            return 0;
        }
    else
        {
            _ASN_CTFAIL(app_key, td, sptr, "%s: constraint failed (%s:%d)",
                td->name, __FILE__, __LINE__);
            return -1;
        }
}

static asn_per_constraints_t ASN_PER_MEMB_NON_BROADCAST_IND_FLAG_CONSTR_2 = {
    {APC_CONSTRAINED, 1, 1, 0, 1} /* (0..1) */,
    {APC_UNCONSTRAINED, -1, -1, 0, 0},
    0,
    0 /* No PER value map */
};
static asn_per_constraints_t ASN_PER_MEMB_TOE_MSB_CONSTR_3 = {
    {APC_CONSTRAINED, 5, 5, 0, 31} /* (0..31) */,
    {APC_UNCONSTRAINED, -1, -1, 0, 0},
    0,
    0 /* No PER value map */
};
static asn_per_constraints_t ASN_PER_MEMB_E_MSB_CONSTR_4 = {
    {APC_CONSTRAINED, 7, 7, 0, 127} /* (0..127) */,
    {APC_UNCONSTRAINED, -1, -1, 0, 0},
    0,
    0 /* No PER value map */
};
static asn_per_constraints_t ASN_PER_MEMB_SQRT_AMBS_CONSTR_5 = {
    {APC_CONSTRAINED, 6, 6, 0, 63} /* (0..63) */,
    {APC_UNCONSTRAINED, -1, -1, 0, 0},
    0,
    0 /* No PER value map */
};
static asn_TYPE_member_t asn_MBR_GANSSNavModel_1[] = {
    {ATF_NOFLAGS, 0, offsetof(struct GANSSNavModel, nonBroadcastIndFlag),
        (ASN_TAG_CLASS_CONTEXT | (0 << 2)), -1, /* IMPLICIT tag at current level */
        &asn_DEF_NativeInteger, memb_nonBroadcastIndFlag_constraint_1,
        &ASN_PER_MEMB_NON_BROADCAST_IND_FLAG_CONSTR_2, 0, "nonBroadcastIndFlag"},
    {ATF_POINTER, 3, offsetof(struct GANSSNavModel, toeMSB),
        (ASN_TAG_CLASS_CONTEXT | (1 << 2)), -1, /* IMPLICIT tag at current level */
        &asn_DEF_NativeInteger, memb_toeMSB_constraint_1,
        &ASN_PER_MEMB_TOE_MSB_CONSTR_3, 0, "toeMSB"},
    {ATF_POINTER, 2, offsetof(struct GANSSNavModel, eMSB),
        (ASN_TAG_CLASS_CONTEXT | (2 << 2)), -1, /* IMPLICIT tag at current level */
        &asn_DEF_NativeInteger, memb_eMSB_constraint_1,
        &ASN_PER_MEMB_E_MSB_CONSTR_4, 0, "eMSB"},
    {ATF_POINTER, 1, offsetof(struct GANSSNavModel, sqrtAMBS),
        (ASN_TAG_CLASS_CONTEXT | (3 << 2)), -1, /* IMPLICIT tag at current level */
        &asn_DEF_NativeInteger, memb_sqrtAMBS_constraint_1,
        &ASN_PER_MEMB_SQRT_AMBS_CONSTR_5, 0, "sqrtAMBS"},
    {ATF_NOFLAGS, 0, offsetof(struct GANSSNavModel, ganssSatelliteList),
        (ASN_TAG_CLASS_CONTEXT | (4 << 2)), -1, /* IMPLICIT tag at current level */
        &asn_DEF_SeqOfGANSSSatelliteElement,
        0, /* Defer constraints checking to the member type */
        0, /* No PER visible constraints */
        0, "ganssSatelliteList"},
};
static int asn_MAP_GANSSNavModel_oms_1[] = {1, 2, 3};
static ber_tlv_tag_t asn_DEF_GANSSNavModel_tags_1[] = {
    (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))};
static asn_TYPE_tag2member_t asn_MAP_GANSSNavModel_tag2el_1[] = {
    {(ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0,
        0},                                        /* nonBroadcastIndFlag at 1228 */
    {(ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0}, /* toeMSB at 1229 */
    {(ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0}, /* eMSB at 1230 */
    {(ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0}, /* sqrtAMBS at 1231 */
    {(ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0,
        0} /* ganssSatelliteList at 1233 */
};
static asn_SEQUENCE_specifics_t asn_SPC_GANSSNavModel_specs_1 = {
    sizeof(struct GANSSNavModel),
    offsetof(struct GANSSNavModel, _asn_ctx),
    asn_MAP_GANSSNavModel_tag2el_1,
    5,                           /* Count of tags in the map */
    asn_MAP_GANSSNavModel_oms_1, /* Optional members */
    3,
    0,  /* Root/Additions */
    -1, /* Start extensions */
    -1  /* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_GANSSNavModel = {
    "GANSSNavModel",
    "GANSSNavModel",
    SEQUENCE_free,
    SEQUENCE_print,
    SEQUENCE_constraint,
    SEQUENCE_decode_ber,
    SEQUENCE_encode_der,
    SEQUENCE_decode_xer,
    SEQUENCE_encode_xer,
    SEQUENCE_decode_uper,
    SEQUENCE_encode_uper,
    0, /* Use generic outmost tag fetcher */
    asn_DEF_GANSSNavModel_tags_1,
    sizeof(asn_DEF_GANSSNavModel_tags_1) /
        sizeof(asn_DEF_GANSSNavModel_tags_1[0]), /* 1 */
    asn_DEF_GANSSNavModel_tags_1,                /* Same as above */
    sizeof(asn_DEF_GANSSNavModel_tags_1) /
        sizeof(asn_DEF_GANSSNavModel_tags_1[0]), /* 1 */
    0,                                           /* No PER visible constraints */
    asn_MBR_GANSSNavModel_1,
    5,                             /* Elements count */
    &asn_SPC_GANSSNavModel_specs_1 /* Additional specs */
};
