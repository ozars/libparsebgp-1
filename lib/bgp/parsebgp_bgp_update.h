/*
 * Copyright (C) 2017 The Regents of the University of California.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __PARSEBGP_BGP_UPDATE_H
#define __PARSEBGP_BGP_UPDATE_H

#include "parsebgp_bgp_common.h"
#include "parsebgp_bgp_update_ext_communities.h"
#include "parsebgp_bgp_update_mp_reach.h"
#include <inttypes.h>

/**
 * BGP ORIGIN Path Attribute values
 */
typedef enum {

  /** IGP - Network Layer Reachability Information is interior to the
      originating AS */
  PARSEBGP_BGP_UPDATE_ORIGIN_IGP = 0,

  /** EGP - Network Layer Reachability Information learned via the EGP protocol
      [RFC904] */
  PARSEBGP_BGP_UPDATE_ORIGIN_EGP = 1,

  /** INCOMPLETE - Network Layer Reachability Information learned by some other
      means */
  PARSEBGP_BGP_UPDATE_ORIGIN_INCOMPLETE = 2,

} parsebgp_bgp_update_origin_type_t;

/**
 * AS Path Segment Types
 */
typedef enum {

  /** AS_SET: Unordered set of ASes a route in the UPDATE message has
      traversed */
  PARSEBGP_BGP_UPDATE_AS_PATH_SEG_AS_SET = 1,

  /** AS_SEQ: Ordered set of ASes a route in the UPDATE message has traversed */
  PARSEBGP_BGP_UPDATE_AS_PATH_SEG_AS_SEQ = 2,

  /** AS Path Segment Confederation Sequence (RFC 5065) */
  PARSEBGP_BGP_UPDATE_AS_PATH_SEG_CONFED_SEQ = 3,

  /** AS Path Segment Confederation Set (RFC 5065) */
  PARSEBGP_BGP_UPDATE_AS_PATH_SEG_CONFED_SET = 4,

} parsebgp_bgp_update_as_path_seg_type_t;

/**
 * AS Path Segment (supports both 2 and 4-byte ASNs)
 */
typedef struct parsebgp_bgp_update_as_path_seg {

  /** Segment Type (parsebgp_bgp_update_as_path_seg_type_t) */
  uint8_t type;

  /** Number of ASNs in the segment */
  uint8_t asns_cnt;

  /** Array of (asn_cnt) ASNs */
  uint32_t *asns;

  /** Number of allocated ASNs (INTERNAL) */
  uint8_t _asns_alloc_cnt;

} __attribute__((packed)) parsebgp_bgp_update_as_path_seg_t;

/**
 * AS Path (supports both 2 and 4-byte ASNs)
 */
typedef struct parsebgp_bgp_update_as_path {

  /** Array of AS Path Segments (may be NULL if shallow parsing is enabled) */
  parsebgp_bgp_update_as_path_seg_t *segs;

  /** Number of allocated segments (INTERNAL) */
  uint8_t _segs_alloc_cnt;

  /** Number of Segments in the AS Path */
  uint8_t segs_cnt;

  /** Number of ASNs in the AS Path
   *
   * Note: this uses the definition in Section 9.1.2.2 of [RFC4271] and Section
   * 5.3 of [RFC5065] which treats AS_SETs as a single ASN, and does not count
   * CONFED_* segments at all.
   */
  uint8_t asns_cnt;

  /** Does the path contain 4-byte ASNs (instead of 2-byte)? */
  uint8_t asn_4_byte;

  /** Pointer to the a copy of the raw AS Path data */
  uint8_t *raw;

  /** Allocated length of the raw data (INTERNAL) */
  uint16_t _raw_alloc_len;

} __attribute__((packed)) parsebgp_bgp_update_as_path_t;

/**
 * AGGREGATOR (supports both 2- and 4-byte ASNs)
 */
typedef struct parsebgp_bgp_update_aggregator {

  /** ASN */
  uint32_t asn;

  /** IP Address */
  uint8_t addr[4];

} parsebgp_bgp_update_aggregator_t;

/**
 * COMMUNITIES
 */
typedef struct parsebgp_bgp_update_communities {

  /** Set of communities (may be NULL if shallow parsing is enabled) */
  uint32_t *communities;

  /** Allocated number of communities (INTERNAL) */
  int _communities_alloc_cnt;

  /** (Inferred) Number of communities in the array */
  int communities_cnt;

  /** Pointer to a copy of the raw communities data */
  uint8_t *raw;

  /** Allocated length of the raw data (INTERNAL) */
  int _raw_alloc_len;

} parsebgp_bgp_update_communities_t;

/**
 * CLUSTER_LIST
 */
typedef struct parsebgp_bgp_update_cluster_list {

  /** Array of CLUSTER_IDs */
  uint32_t *cluster_ids;

  /** Number of allocated CLUSTER_IDs (INTERNAL) */
  int _cluster_ids_alloc_cnt;

  /** (Inferred) Number of CLUSTER_IDs in the array */
  int cluster_ids_cnt;

} parsebgp_bgp_update_cluster_list_t;

/**
 * AS_PATHLIMIT
 */
typedef struct parsebgp_bgp_update_as_pathlimit {

  /** Upper bound on the number of ASes in the AS_PATH attribute */
  uint8_t max_asns;

  /** AS Number */
  uint32_t asn;

} parsebgp_bgp_update_as_pathlimit_t;

/**
 * LARGE COMMUNITY
 */
typedef struct parsebgp_bgp_update_large_community {

  /** Global Administrator (ASN) */
  uint32_t global_admin;

  /** Local Data Part 1 */
  uint32_t local_1;

  /** Local Data Part 2 */
  uint32_t local_2;

} parsebgp_bgp_update_large_community_t;

/**
 * LARGE COMMUNITIES
 */
typedef struct parsebgp_bgp_update_large_communities {

  /** Array of (communities_cnt) LARGE COMMUNITIES */
  parsebgp_bgp_update_large_community_t *communities;

  /** Number of allocated communities (INTERNAL) */
  int _communities_alloc_cnt;

  /** (Inferred) number of communities */
  int communities_cnt;

} parsebgp_bgp_update_large_communities_t;

typedef enum {

  /** ORIGIN (Type Code 1) */
  PARSEBGP_BGP_PATH_ATTR_TYPE_ORIGIN = 1, // DONE, TESTED

  /** AS_PATH (Type Code 2) */
  PARSEBGP_BGP_PATH_ATTR_TYPE_AS_PATH = 2, // DONE, TESTED

  /** NEXT_HOP (Type Code 3) */
  PARSEBGP_BGP_PATH_ATTR_TYPE_NEXT_HOP = 3, // DONE, TESTED

  /** MULTI_EXIT_DISC (Type Code 4) */
  PARSEBGP_BGP_PATH_ATTR_TYPE_MED = 4, // DONE, TESTED

  /** LOCAL_PREF (Type Code 5) */
  PARSEBGP_BGP_PATH_ATTR_TYPE_LOCAL_PREF = 5, // DONE

  /** ATOMIC_AGGREGATE (Type Code 6) */
  PARSEBGP_BGP_PATH_ATTR_TYPE_ATOMIC_AGGREGATE = 6, // DONE, TESTED

  /** AGGREGATOR (Type Code 7) */
  PARSEBGP_BGP_PATH_ATTR_TYPE_AGGREGATOR = 7, // DONE, TESTED

  /** COMMUNITY (Type Code 8) [RFC1997] */
  PARSEBGP_BGP_PATH_ATTR_TYPE_COMMUNITIES = 8, // DONE, TESTED

  /** ORIGINATOR_ID (Type Code 9) [RFC4456] */
  PARSEBGP_BGP_PATH_ATTR_TYPE_ORIGINATOR_ID = 9, // DONE

  /** CLUSTER_LIST (Type Code 10) [RFC4456] */
  PARSEBGP_BGP_PATH_ATTR_TYPE_CLUSTER_LIST = 10, // DONE

  // ...

  /** MP_REACH_NLRI (Type Code 14) [RFC4760] */
  PARSEBGP_BGP_PATH_ATTR_TYPE_MP_REACH_NLRI = 14, // DONE-PARTLY, TESTED

  /** MP_UNREACH_NLRI (Type Code 15) [RFC4760] */
  PARSEBGP_BGP_PATH_ATTR_TYPE_MP_UNREACH_NLRI = 15, // DONE, TESTED

  /** EXTENDED COMMUNITIES (Type Code 16) [RFC4360] */
  PARSEBGP_BGP_PATH_ATTR_TYPE_EXT_COMMUNITIES = 16, // DONE-PARTLY, TESTED

  /** AS4_PATH (Type Code 17) [RFC6793] */
  PARSEBGP_BGP_PATH_ATTR_TYPE_AS4_PATH = 17, // DONE, TESTED

  /** AS4_AGGREGATOR (Type Code 18) [RFC6793] */
  PARSEBGP_BGP_PATH_ATTR_TYPE_AS4_AGGREGATOR = 18, // DONE, TESTED

  // ...

  /** AS_PATHLIMIT (deprecated) (Type Code 21) [draft-ietf-idr-as-pathlimit-03]
   */
  PARSEBGP_BGP_PATH_ATTR_TYPE_AS_PATHLIMIT = 21, // DONE, TESTED

  // ...

  /** IPv6 Address Specific Extended Community (Type Code 25) [RFC5701] */
  PARSEBGP_BGP_PATH_ATTR_TYPE_IPV6_EXT_COMMUNITIES = 25, // DONE, TESTED

  // ...

  /** BGP-LS Attribute (Type Code 29) [RFC7752] */
  PARSEBGP_BGP_PATH_ATTR_TYPE_BGP_LS = 29,

  // ...

  /** LARGE_COMMUNITY (Type Code 32) [RFC8092] */
  PARSEBGP_BGP_PATH_ATTR_TYPE_LARGE_COMMUNITIES = 32, // DONE, TESTED

  /** Length of Path Attributes array (use when iterating over all path
      attributes) */
  PARSEBGP_BGP_PATH_ATTRS_LEN,

} parsebgp_bgp_update_path_attr_type_t;

typedef enum {

  /** Optional (i.e., not well-known) */
  PARSEBGP_BGP_PATH_ATTR_FLAG_OPTIONAL = 0x80,

  /** Transitive */
  PARSEBGP_BGP_PATH_ATTR_FLAG_TRANSITIVE = 0x40,

  /** Partial */
  PARSEBGP_BGP_PATH_ATTR_FLAG_PARTIAL = 0x20,

  /** Extended */
  PARSEBGP_BGP_PATH_ATTR_FLAG_EXTENDED = 0x10,

} parsebgp_bgp_update_path_attr_flag_t;

/**
 * BGP UPDATE Path Attribute
 */
typedef struct parsebgp_bgp_update_path_attr {

  /** Attribute Flags */
  uint8_t flags;

  /** Attribute Type (parsebgp_bgp_update_path_attr_type_t) */
  uint8_t type;

  /** Attribute Length (in bytes) */
  uint16_t len;

  /** Union of all support Path Attribute data */
  union {

    /** ORIGIN (parsebgp_bgp_update_origin_type_t) */
    uint8_t origin;

    /** AS_PATH or AS4_PATH
     *
     * An AS4_PATH should be merged with the AS_PATH attribute using the method
     * outlined in RFC6793 section 4.2.3.
     */
    parsebgp_bgp_update_as_path_t *as_path;

    /** NEXT_HOP */
    uint8_t next_hop[4];

    /** MULIT_EXIT_DISC (MED) */
    uint32_t med;

    /** LOCAL_PREF */
    uint32_t local_pref;

    /** AGGREGATOR */
    parsebgp_bgp_update_aggregator_t aggregator;

    /** COMMUNITIES */
    parsebgp_bgp_update_communities_t *communities;

    /** ORIGINATOR_ID */
    uint32_t originator_id;

    /** CLUSTER_LIST */
    parsebgp_bgp_update_cluster_list_t *cluster_list;

    /** MP_REACH */
    parsebgp_bgp_update_mp_reach_t *mp_reach;

    /** MP_UNREACH */
    parsebgp_bgp_update_mp_unreach_t *mp_unreach;

    /** EXT_COMMUNITIES and IPV6_EXT_COMMUNITIES */
    parsebgp_bgp_update_ext_communities_t *ext_communities;

    /** AS_PATHLIMIT */
    parsebgp_bgp_update_as_pathlimit_t as_pathlimit;

    /** LARGE COMMUNITIES */
    parsebgp_bgp_update_large_communities_t *large_communities;

  } data;

} parsebgp_bgp_update_path_attr_t;

/**
 * BGP Path Attributes
 */
typedef struct parsebgp_bgp_update_path_attrs {

  /** Length of the (raw) Path Attributes data (in bytes) */
  uint16_t len;

  /** Array of Path Attributes
   *
   * Attributes are stored at attrs[ATTR_TYPE] to allow access to specific
   * attributes without having to do a linear search. Users should check that
   * attrs[ATTR_TYPE].type is set (to ATTR_TYPE) before accessing any other
   * fields of the attribute.
   */
  parsebgp_bgp_update_path_attr_t attrs[PARSEBGP_BGP_PATH_ATTRS_LEN];

  /** Array of attrs_cnt ATTR_TYPE values that give indices to attrs.
   * Facilitates fast iteration over all in-use attributes */
  uint8_t *attrs_used;

  /** Allocated length of the attrs_used array (INTERNAL) */
  int _attrs_used_alloc_cnt;

  /** Number of populated Path Attributes in the attrs field */
  int attrs_cnt;

} parsebgp_bgp_update_path_attrs_t;

/**
 * BGP UPDATE NLRIs
 */
typedef struct parsebgp_bgp_update_nlris {

  /** Length of the (raw) NLRI data (in bytes) */
  uint16_t len;

  /** Array of (prefixes_cnt) prefixes */
  parsebgp_bgp_prefix_t *prefixes;

  /** Number of allocated prefixes (INTERNAL) */
  int _prefixes_alloc_cnt;

  /** (Inferred) number of prefixes in the prefixes field */
  int prefixes_cnt;

} parsebgp_bgp_update_nlris_t;

/**
 * BGP UPDATE Message
 */
typedef struct parsebgp_bgp_update {

  /** Withdrawn NLRIs */
  parsebgp_bgp_update_nlris_t withdrawn_nlris;

  /** Path Attributes */
  parsebgp_bgp_update_path_attrs_t path_attrs;

  /** Announced NLRIs (Note that the len field is inferred) */
  parsebgp_bgp_update_nlris_t announced_nlris;

} parsebgp_bgp_update_t;

#endif /* __PARSEBGP_BGP_UPDATE_H */
