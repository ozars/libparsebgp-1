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

#include "parsebgp_bgp.h"
#include "parsebgp_error.h"
#include "parsebgp_utils.h"
#include "parsebgp_bgp_open_impl.h"
#include "parsebgp_bgp_update_impl.h"
#include "parsebgp_bgp_notification_impl.h"
#include "parsebgp_bgp_route_refresh_impl.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define BGP_HDR_LEN 19

static parsebgp_error_t parse_common_hdr(parsebgp_opts_t *opts,
                                         parsebgp_bgp_msg_t *msg, const uint8_t *buf,
                                         size_t *lenp)
{
  size_t len = *lenp, nread = 0;

  // Marker
  if (opts->bgp.marker_omitted == 0) {
    if ((len - nread) < sizeof(msg->marker)) {
      return PARSEBGP_PARTIAL_MSG;
    }
    if (opts->bgp.marker_copy != 0) {
      memcpy(&msg->marker, buf, sizeof(msg->marker));
    }
    nread += sizeof(msg->marker);
    buf += sizeof(msg->marker);
  }

  // Length
  PARSEBGP_DESERIALIZE_UINT16(buf, len, nread, msg->len);

  // Type
  PARSEBGP_DESERIALIZE_UINT8(buf, len, nread, msg->type);

  *lenp = nread;
  return PARSEBGP_OK;
}

parsebgp_error_t parsebgp_bgp_decode_ext(parsebgp_opts_t *opts,
                                         parsebgp_bgp_msg_t *msg,
                                         const uint8_t *buf,
                                         size_t *len, int allow_truncation)
{
  parsebgp_error_t err;
  size_t slen = 0, nread = 0, remain = 0;

  /* First, parse the message header */
  slen = *len;
  if ((err = parse_common_hdr(opts, msg, buf, &slen)) != PARSEBGP_OK) {
    return err;
  }
  nread += slen;
  buf += slen;

  remain = msg->len - nread; // number of bytes left in the message
  slen = *len - nread;       // number of bytes left in the buffer

  if (remain > slen && !allow_truncation) {
    // we already know that the message will be longer than what we have in the
    // buffer, give up now
    return PARSEBGP_PARTIAL_MSG;
  }

  switch (msg->type) {
  case PARSEBGP_BGP_TYPE_OPEN:
    PARSEBGP_MAYBE_MALLOC_ZERO(msg->types.open);
    err = parsebgp_bgp_open_decode(opts, msg->types.open, buf, &slen, remain);
    break;

  case PARSEBGP_BGP_TYPE_UPDATE:
    PARSEBGP_MAYBE_MALLOC_ZERO(msg->types.update);
    err =
      parsebgp_bgp_update_decode(opts, msg->types.update, buf, &slen, remain);
    if (err == PARSEBGP_PARTIAL_MSG && allow_truncation) {
      // leave *len unchanged; i.e., we consumed everything available
      return PARSEBGP_TRUNCATED_MSG;
    }
    break;

  case PARSEBGP_BGP_TYPE_NOTIFICATION:
    PARSEBGP_MAYBE_MALLOC_ZERO(msg->types.notification);
    err = parsebgp_bgp_notification_decode(opts, msg->types.notification, buf,
                                           &slen, remain);
    break;

  case PARSEBGP_BGP_TYPE_KEEPALIVE:
    // no data
    err = PARSEBGP_OK;
    slen = 0;
    break;

  case PARSEBGP_BGP_TYPE_ROUTE_REFRESH:
    PARSEBGP_MAYBE_MALLOC_ZERO(msg->types.route_refresh);
    err = parsebgp_bgp_route_refresh_decode(opts, msg->types.route_refresh, buf,
                                            &slen, remain);
    break;

  default:
    PARSEBGP_RETURN_INVALID_MSG_ERR;
  }
  if (err != PARSEBGP_OK) {
    // parser failed
    return err;
  }
  nread += slen;

  assert(msg->len == nread);
  *len = nread;
  return PARSEBGP_OK;
}

parsebgp_error_t parsebgp_bgp_decode(parsebgp_opts_t *opts,
                                     parsebgp_bgp_msg_t *msg,
                                     const uint8_t *buf,
                                     size_t *len)
{
  return parsebgp_bgp_decode_ext(opts, msg, buf, len, 0);
}

void parsebgp_bgp_destroy_msg(parsebgp_bgp_msg_t *msg)
{
  if (msg == NULL) {
    return;
  }

  // no dynamic memory in common header

  parsebgp_bgp_open_destroy(msg->types.open);
  parsebgp_bgp_update_destroy(msg->types.update);
  parsebgp_bgp_notification_destroy(msg->types.notification);
  parsebgp_bgp_route_refresh_destroy(msg->types.route_refresh);

  free(msg);
}

void parsebgp_bgp_clear_msg(parsebgp_bgp_msg_t *msg)
{
  if (msg == NULL) {
    return;
  }

  // no dynamic memory in common header

  // clear based on message type
  switch (msg->type) {
  case PARSEBGP_BGP_TYPE_OPEN:
    parsebgp_bgp_open_clear(msg->types.open);
    break;

  case PARSEBGP_BGP_TYPE_UPDATE:
    parsebgp_bgp_update_clear(msg->types.update);
    break;

  case PARSEBGP_BGP_TYPE_NOTIFICATION:
    parsebgp_bgp_notification_clear(msg->types.notification);
    break;

  case PARSEBGP_BGP_TYPE_KEEPALIVE:
    // nothing to clear
    break;

  case PARSEBGP_BGP_TYPE_ROUTE_REFRESH:
    parsebgp_bgp_route_refresh_clear(msg->types.route_refresh);
    break;

  default:
    break;
  }
}

void parsebgp_bgp_dump_msg(const parsebgp_bgp_msg_t *msg, int depth)
{
  PARSEBGP_DUMP_STRUCT_HDR(parsebgp_bgp_msg_t, depth);

  PARSEBGP_DUMP_DATA(depth, "Marker", msg->marker, sizeof(msg->marker));
  PARSEBGP_DUMP_INT(depth, "Length", msg->len);
  PARSEBGP_DUMP_INT(depth, "Type", msg->type);

  depth++;

  switch (msg->type) {
  case PARSEBGP_BGP_TYPE_OPEN:
    parsebgp_bgp_open_dump(msg->types.open, depth);
    break;

  case PARSEBGP_BGP_TYPE_UPDATE:
    parsebgp_bgp_update_dump(msg->types.update, depth);
    break;

  case PARSEBGP_BGP_TYPE_NOTIFICATION:
    parsebgp_bgp_notification_dump(msg->types.notification, depth);
    break;

  case PARSEBGP_BGP_TYPE_KEEPALIVE:
    PARSEBGP_DUMP_INFO(depth, "KEEPALIVE\n");
    break;

  case PARSEBGP_BGP_TYPE_ROUTE_REFRESH:
    parsebgp_bgp_route_refresh_dump(msg->types.route_refresh, depth);
    break;

  default:
    break;
  }
}
