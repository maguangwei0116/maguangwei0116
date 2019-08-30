/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#include "PrepareDownloadResponseOk.h"

asn_TYPE_member_t asn_MBR_PrepareDownloadResponseOk_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PrepareDownloadResponseOk, euiccSigned2),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_EUICCSigned2,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"euiccSigned2"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PrepareDownloadResponseOk, euiccSignature2),
		(ASN_TAG_CLASS_APPLICATION | (55 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"euiccSignature2"
		},
};
static const ber_tlv_tag_t asn_DEF_PrepareDownloadResponseOk_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_PrepareDownloadResponseOk_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 0, 0, 0 }, /* euiccSigned2 */
    { (ASN_TAG_CLASS_APPLICATION | (55 << 2)), 1, 0, 0 } /* euiccSignature2 */
};
asn_SEQUENCE_specifics_t asn_SPC_PrepareDownloadResponseOk_specs_1 = {
	sizeof(struct PrepareDownloadResponseOk),
	offsetof(struct PrepareDownloadResponseOk, _asn_ctx),
	asn_MAP_PrepareDownloadResponseOk_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	2,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_PrepareDownloadResponseOk = {
	"PrepareDownloadResponseOk",
	"PrepareDownloadResponseOk",
	&asn_OP_SEQUENCE,
	asn_DEF_PrepareDownloadResponseOk_tags_1,
	sizeof(asn_DEF_PrepareDownloadResponseOk_tags_1)
		/sizeof(asn_DEF_PrepareDownloadResponseOk_tags_1[0]), /* 1 */
	asn_DEF_PrepareDownloadResponseOk_tags_1,	/* Same as above */
	sizeof(asn_DEF_PrepareDownloadResponseOk_tags_1)
		/sizeof(asn_DEF_PrepareDownloadResponseOk_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_PrepareDownloadResponseOk_1,
	2,	/* Elements count */
	&asn_SPC_PrepareDownloadResponseOk_specs_1	/* Additional specs */
};

