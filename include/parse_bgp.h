/*
 * Copyright (c) 2013-2015 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 *
 */

#ifndef PARSEBGP_H_
#define PARSEBGP_H_

#include <vector>
#include <list>
#include "bgp_common.h"
#include "update_msg.h"
#include "open_msg.h"
#include "notification_msg.h"


using namespace std;
/**
 * \class   parseBGP
 *
 * \brief   Parser for BGP messages
 * \details This class can be used as needed to parse a complete BGP message. This
 *          class will read directly from the socket to read the BGP message.
 */
/**
 * Below defines the common BGP header per RFC4271
 */
enum bgp_msg_types { BGP_MSG_OPEN=1, BGP_MSG_UPDATE, BGP_MSG_NOTIFICATION, BGP_MSG_KEEPALIVE,
    BGP_MSG_ROUTE_REFRESH
};

typedef struct libparsebgp_common_bgp_hdr {
    /**
     * 16-octet field is included for compatibility
     * All ones (required).
     */
    uint8_t   marker[16];

    /**
     * Total length of the message, including the header in octets
     *
     * min length is 19, max is 4096
     */
    unsigned short   len;
    /**
     * type code of the message
     *
     * 1 - OPEN
     * 2 - UPDATE
     * 3 - NOTIFICATION
     * 4 - KEEPALIVE
     * 5 - ROUTE-REFRESH
     */
    unsigned char    type;
}libparsebgp_common_bgp_hdr;

typedef struct libparsebgp_parse_bgp_parsed_data {
    libparsebgp_common_bgp_hdr c_hdr;
    //union needed
    struct parsed_bgp_data {
        libparsebgp_open_msg_data open_msg;
        libparsebgp_update_msg_data update_msg;
        libparsebgp_notify_msg notification_msg;
    }parsed_data;
    bool has_end_of_rib_marker;
    /**
     * data_bytes_remaining is a counter that starts at the message size and then is
     * decremented as the message is read.
     *
     * This is used to ensure no buffer overruns on bgp data buffer
     */
    int data_bytes_remaining;

    /**
     * BGP data buffer for the raw BGP message to be parsed
     */
    unsigned char *data;                             ///< Pointer to the data buffer for the raw BGP message
}libparsebgp_parse_bgp_parsed_data;

    uint32_t libparsebgp_parse_bgp_parse_msg(libparsebgp_parse_bgp_parsed_data &bgp_parsed_data, u_char *&data, size_t size,
                                                  bool is_local_msg = true);


    /**
     * handle BGP update message and store in DB
     *
     * \details Parses the bgp update message and store it in the DB.
     *
     * \param [in]     data             Pointer to the raw BGP message header
     * \param [in]     size             length of the data buffer (used to prevent overrun)

     *
     * \returns True if error, false if no error.
     */
    int libparsebgp_parse_bgp_handle_update(libparsebgp_parse_bgp_parsed_data &update_msg, u_char *data, size_t size);

    /**
     * handle BGP notify event - updates the down event with parsed data
     *
     * \details
     *  The notify message does not directly add to Db, so the calling
     *  method/function must handle that.
     *
     * \param [in]     data             Pointer to the raw BGP message header
     * \param [in]     size             length of the data buffer (used to prevent overrun)
     * \param [out]    down_event       Reference to the down event/notification storage buffer
     *
     * \returns True if error, false if no error.
     */
    bool libparsebgp_parse_bgp_handle_down_event(libparsebgp_parse_bgp_parsed_data &bgp_parsed_data, u_char *data, size_t size);

    /**
     * Parses the BGP common header
     *
     * \details
     *      This method will parse the bgp common header and will upload the global
     *      c_hdr structure, instance data pointer, and remaining bytes of message.
     *      The return value of this method will be the BGP message type.
     *
     * \param [in]      data            Pointer to the raw BGP message header
     * \param [in]      size            length of the data buffer (used to prevent overrun)
     *
     * \returns BGP message type
     */
    u_char libparsebgp_parse_bgp_parse_header(libparsebgp_parse_bgp_parsed_data &bgp_parsed_data, u_char *data, size_t size);
//uint32_t libparsebgp_parse_bgp_parse_msg(libparsebgp_parse_bgp_parsed_data &parsed_bgp_msg, unsigned char *data, uint32_t size);

#endif /* PARSEBGP_H_ */