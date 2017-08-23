#ifndef __PARSEBGP_BGP_UPDATE_EXT_COMMUNITIES_H
#define __PARSEBGP_BGP_UPDATE_EXT_COMMUNITIES_H

#include "parsebgp_bgp_common.h"
#include "parsebgp_error.h"
#include "parsebgp_opts.h"
#include <inttypes.h>
#include <stdlib.h>

/**
 * Extended Community Types (High-byte)
 *
 * See
 * http://www.iana.org/assignments/bgp-extended-communities/bgp-extended-communities.xhtml
 */
typedef enum {

  /* Transitive Types: */

  /** Two-Octet AS-Specific (Transitive) */
  PARSEBGP_BGP_EXT_COMM_TYPE_TRANS_TWO_OCTET_AS = 0x00,

  /** IPv4-Address-Specific (Transitive) */
  PARSEBGP_BGP_EXT_COMM_TYPE_TRANS_IPV4 = 0x01,

  /** Four-Octet AS-Specific (Transitive) */
  PARSEBGP_BGP_EXT_COMM_TYPE_TRANS_FOUR_OCTET_AS = 0x02,

  /** Opaque (Transitive) */
  PARSEBGP_BGP_EXT_COMM_TYPE_TRANS_OPAQUE = 0x03,

  // ...

  /** Non-Transitive Types: */

  /** Two-Octet AS-Specific (Non-Transitive) */
  PARSEBGP_BGP_EXT_COMM_TYPE_NONTRANS_TWO_OCTET_AS = 0x40,

  /** IPv4-Address-Specific (Non-Transitive) */
  PARSEBGP_BGP_EXT_COMM_TYPE_NONTRANS_IPV4 = 0x41,

  /** Four-Octet AS-Specific (Non-Transitive) */
  PARSEBGP_BGP_EXT_COMM_TYPE_NONTRANS_FOUR_OCTET_AS = 0x42,

  /** Opaque (Non-Transitive) */
  PARSEBGP_BGP_EXT_COMM_TYPE_NONTRANS_OPAQUE = 0x43,

} parsebgp_bgp_update_ext_community_type_t;

/**
 * IPv6 Address Specific Extended Community Types (High-byte)
 *
 * See
 * http://www.iana.org/assignments/bgp-extended-communities/bgp-extended-communities.xhtml
 */
typedef enum {

  /* Transitive Types: */

  /** IPv6-Address-Specific (Transitive) */
  PARSEBGP_BGP_EXT_COMM_TYPE_TRANS_IPV6 = 0x00,

  // ...

  /** Non-Transitive Types: */

  /** IPv6-Address-Specific (Non-Transitive) */
  PARSEBGP_BGP_EXT_COMM_TYPE_NONTRANS_IPV6 = 0x40,

} parsebgp_bgp_update_ext_community_ipv6_type_t;

/**
 * Two-Octet AS-Specific Extended Community
 */
typedef struct parsebgp_bgp_update_ext_community_two_octet {

  /** Global Administrator (2-byte ASN) */
  uint16_t global_admin;

  /** Local Administrator (Encoding is sub-type specific) */
  uint32_t local_admin;

} parsebgp_bgp_update_ext_community_two_octet_t;

/**
 * IPv4 and IPv6 Address Specific Extended Community
 */
typedef struct parsebgp_bgp_update_ext_community_ip_addr {

  /** Global Administrator IP AFI */
  uint16_t global_admin_ip_afi;

  /** Global Administrator (IP Address) */
  uint8_t global_admin_ip[16];

  /** Local Administrator (Encoding is sub-type specific) */
  uint16_t local_admin;

} parsebgp_bgp_update_ext_community_ip_addr_t;

/**
 * Four-Octet AS-Specific Extended Community
 */
typedef struct parsebgp_bgp_update_ext_community_four_octet {

  /** Global Administrator (4-byte ASN) */
  uint32_t global_admin;

  /** Local Administrator (Encoding is sub-type specific) */
  uint16_t local_admin;

} parsebgp_bgp_update_ext_community_four_octet_t;

/**
 * Extended Community (also supports IPv6-Extended Community)
 */
typedef struct parsebgp_bgp_update_ext_community {

  /** Type (High-order byte) */
  uint8_t type;

  /* Sub-Type (Low-order byte, optional) */
  uint8_t subtype;

  /** Union of supported Extended Community type data */
  union {

    /** Two-Octet AS-Specific */
    parsebgp_bgp_update_ext_community_two_octet_t two_octet;

    /** IPv4 and IPv6 Address Specific */
    parsebgp_bgp_update_ext_community_ip_addr_t ip_addr;

    /** Four-Octet AS-Specific */
    parsebgp_bgp_update_ext_community_four_octet_t four_octet;

    /** Opaque */
    uint8_t opaque[6];

    /** Unknown */
    uint8_t unknown[7];

  } types;

} parsebgp_bgp_update_ext_community_t;

/**
 * Extended Communities
 *
 * Note that while we parse the community based on the type, we leave sub-type
 * parsing to the user.
 *
 * Since the sub-type field is optional, and its presence is not encoded into
 * the structure of the type, we can only parse communities for types that we
 * know about. If an unknown type is encountered, only the "type" field is
 * populated, and the remainder of the community is stored in the "unknown"
 * field.
 */
typedef struct parsebgp_bgp_update_ext_communities {

  /** Array of (communities_cnt) EXTENDED COMMUNITIES */
  parsebgp_bgp_update_ext_community_t *communities;

  /** (Inferred) number of communities */
  int communities_cnt;

} parsebgp_bgp_update_ext_communities_t;

/** Decode an EXTENDED COMMUNITIES message */
parsebgp_error_t parsebgp_bgp_update_ext_communities_decode(
  parsebgp_opts_t *opts, parsebgp_bgp_update_ext_communities_t *msg,
  uint8_t *buf, size_t *lenp, size_t remain);

/** Decode an IPv6 EXTENDED COMMUNITIES message */
parsebgp_error_t parsebgp_bgp_update_ext_communities_ipv6_decode(
  parsebgp_opts_t *opts, parsebgp_bgp_update_ext_communities_t *msg,
  uint8_t *buf, size_t *lenp, size_t remain);

/**
 * Dump a human-readable version of the message to stdout
 *
 * @param msg           Pointer to the parsed MP_REACH attribute to dump
 * @param depth         Depth of the message within the overall message
 *
 * The output from these functions is designed to help with debugging the
 * library and also includes internal implementation information like the names
 * and sizes of structures. It may be useful to potential users of the library
 * to get a sense of their data.
 */
void parsebgp_bgp_update_ext_communities_dump(
  parsebgp_bgp_update_ext_communities_t *msg, int depth);

/** Destroy an EXTENDED COMMUNITIES message */
void parsebgp_bgp_update_ext_communities_destroy(
  parsebgp_bgp_update_ext_communities_t *msg);

#endif /* __PARSEBGP_BGP_UPDATE_EXT_COMMUNITIES_H */
