/*
 * SPDX-FileCopyrightText: (c) 2003, 2004 Lev Walkin <vlm@lionet.info>. All rights reserved.
 * SPDX-License-Identifier: BSD-1-Clause
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "RRLP-Messages"
 *     found in "../rrlp-messages.asn"
 */

#include "PDU.h"

static int memb_referenceNumber_constraint_1(
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

    if ((value >= 0 && value <= 7))
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

static asn_per_constraints_t ASN_PER_MEMB_REFERENCE_NUMBER_CONSTR_2 = {
    {APC_CONSTRAINED, 3, 3, 0, 7} /* (0..7) */,
    {APC_UNCONSTRAINED, -1, -1, 0, 0},
    0,
    0 /* No PER value map */
};
static asn_TYPE_member_t asn_MBR_PDU_1[] = {
    {ATF_NOFLAGS, 0, offsetof(struct PDU, referenceNumber),
        (ASN_TAG_CLASS_CONTEXT | (0 << 2)), -1, /* IMPLICIT tag at current level */
        &asn_DEF_NativeInteger, memb_referenceNumber_constraint_1,
        &ASN_PER_MEMB_REFERENCE_NUMBER_CONSTR_2, 0, "referenceNumber"},
    {ATF_NOFLAGS, 0, offsetof(struct PDU, component),
        (ASN_TAG_CLASS_CONTEXT | (1 << 2)), +1, /* EXPLICIT tag at current level */
        &asn_DEF_RRLP_Component,
        0, /* Defer constraints checking to the member type */
        0, /* No PER visible constraints */
        0, "component"},
};
static ber_tlv_tag_t asn_DEF_PDU_tags_1[] = {
    (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))};
static asn_TYPE_tag2member_t asn_MAP_PDU_tag2el_1[] = {
    {(ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0}, /* referenceNumber at 16 */
    {(ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0}  /* component at 18 */
};
static asn_SEQUENCE_specifics_t asn_SPC_PDU_specs_1 = {
    sizeof(struct PDU),
    offsetof(struct PDU, _asn_ctx),
    asn_MAP_PDU_tag2el_1,
    2, /* Count of tags in the map */
    0,
    0,
    0,  /* Optional elements (not needed) */
    -1, /* Start extensions */
    -1  /* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_PDU = {
    "PDU",
    "PDU",
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
    asn_DEF_PDU_tags_1,
    sizeof(asn_DEF_PDU_tags_1) / sizeof(asn_DEF_PDU_tags_1[0]), /* 1 */
    asn_DEF_PDU_tags_1,                                         /* Same as above */
    sizeof(asn_DEF_PDU_tags_1) / sizeof(asn_DEF_PDU_tags_1[0]), /* 1 */
    0,                                                          /* No PER visible constraints */
    asn_MBR_PDU_1,
    2,                   /* Elements count */
    &asn_SPC_PDU_specs_1 /* Additional specs */
};
