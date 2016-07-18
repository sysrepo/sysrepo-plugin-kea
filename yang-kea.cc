// Copyright (C) 2016 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
/// @file yang-kea.cc
///
/// This file contains primary interface for operations
/// needed for Kea YANG interface. It is specifiec to
/// Sysrepo implementation.

#include "yang-kea.h"

#include <sstream>

using namespace std;

std::string
SysrepoKea::srTypeToText(sr_type_t type)
{
    typedef struct {
        sr_type_t type;
        const std::string name;
    } sr_types_text;

    sr_types_text names[] = {
    /* special types that does not contain any data */
        { SR_UNKNOWN_T, "SR_UNKNOWN_T" },     /**< Element unknown to sysrepo (unsupported element). */
        { SR_LIST_T, "SR_LIST_T" },           /**< List instance. ([RFC 6020 sec 7.8](http://tools.ietf.org/html/rfc6020#section-7.8)) */
        { SR_CONTAINER_T, "SR_CONTAINER_T" }, /**< Non-presence container. ([RFC 6020 sec 7.5](http://tools.ietf.org/html/rfc6020#section-7.5)) */
        { SR_CONTAINER_PRESENCE_T, "SR_CONTAINER_PRESENCE_T" }, /**< Presence container. ([RFC 6020 sec 7.5.1](http://tools.ietf.org/html/rfc6020#section-7.5.1)) */
        { SR_LEAF_EMPTY_T, "SR_LEAF_EMPTY_T" },           /**< A leaf that does not hold any value ([RFC 6020 sec 9.11](http://tools.ietf.org/html/rfc6020#section-9.11)) */
        { SR_UNION_T, "SR_UNION_T" },                /**< Choice of member types ([RFC 6020 sec 9.12](http://tools.ietf.org/html/rfc6020#section-9.12)) */

    /* types containing some data */
        { SR_BINARY_T, "SR_BINARY_T" },   /**< Base64-encoded binary data ([RFC 6020 sec 9.8](http://tools.ietf.org/html/rfc6020#section-9.8)) */
        { SR_BITS_T, "SR_BITS_T" },        /**< A set of bits or flags ([RFC 6020 sec 9.7](http://tools.ietf.org/html/rfc6020#section-9.7)) */
        { SR_BOOL_T, "SR_BOOL_T" },         /**< A boolean value ([RFC 6020 sec 9.5](http://tools.ietf.org/html/rfc6020#section-9.5)) */
        { SR_DECIMAL64_T, "SR_DECIMAL64_T" },    /**< 64-bit signed decimal number ([RFC 6020 sec 9.3](http://tools.ietf.org/html/rfc6020#section-9.3)) */
        { SR_ENUM_T, "SR_ENUM_T" },         /**< A string from enumerated strings list ([RFC 6020 sec 9.6](http://tools.ietf.org/html/rfc6020#section-9.6)) */
        { SR_IDENTITYREF_T, "SR_IDENTITYREF_T" },  /**< A reference to an abstract identity ([RFC 6020 sec 9.10](http://tools.ietf.org/html/rfc6020#section-9.10)) */
        { SR_INSTANCEID_T, "SR_INSTANCEID_T" },  /**< References a data tree node ([RFC 6020 sec 9.13](http://tools.ietf.org/html/rfc6020#section-9.13)) */
        { SR_INT8_T, "SR_INT8_T" },         /**< 8-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
        { SR_INT16_T, "SR_INT16_T" },        /**< 16-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
        { SR_INT32_T, "SR_INT32_T" },       /**< 32-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
        { SR_INT64_T, "SR_INT64_T" },        /**< 64-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
        { SR_STRING_T, "SR_STRING_T" },       /**< Human-readable string ([RFC 6020 sec 9.4](http://tools.ietf.org/html/rfc6020#section-9.4)) */
        { SR_UINT8_T, "SR_UINT8_T" },        /**< 8-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
        { SR_UINT16_T, "SR_UINT16_T" },       /**< 16-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
        { SR_UINT32_T, "SR_UINT32_T" },       /**< 32-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
        { SR_UINT64_T, "SR_UINT64_T" }       /**< 64-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    };

    for (int i = 0; i < sizeof(names)/sizeof(names[0]); i++) {
        if (names[i].type == type) {
            return (names[i].name);
        }
    }

    return "unknown";
}

std::string
SysrepoKea::valueToText(sr_val_t *value, bool xpath, bool type)
{
    stringstream tmp;

    if (xpath) {
        tmp << value->xpath;
    }
    if (type) {
        tmp << ",type=" << srTypeToText(value->type) << ":";
    }

    if (xpath || type) {
        tmp <<  ":";
    }
    switch (value->type) {
        case SR_CONTAINER_T:
        case SR_CONTAINER_PRESENCE_T:
        case SR_LIST_T:
            /* do not print */
            break;
        case SR_STRING_T:
            tmp << value->data.string_val;
            break;
        case SR_BOOL_T:
            tmp << value->data.bool_val ? "true" : "false";
            break;
        case SR_UINT8_T:
            // Converted to uint16, because otherwise it will be printed as
            // a single character.
            tmp << static_cast<uint16_t>(value->xpath, value->data.uint8_val);
            break;
        case SR_UINT16_T:
            tmp << static_cast<uint16_t>(value->xpath, value->data.uint16_val);
            break;
        case SR_UINT32_T:
            tmp << static_cast<uint32_t>(value->xpath, value->data.uint16_val);
            break;
        case SR_IDENTITYREF_T:
            tmp << value->data.identityref_val;
            break;
        case SR_ENUM_T:
            tmp << value->data.enum_val;
            break;
        default:
            if (!xpath) {
                // We need to print it only if xpath was not printed
                // already.
                tmp << "xpath=" << value->xpath;
            }
            tmp << "(unprintable)";
    }

    return (tmp.str());
}

std::string
tabs(int level) {
    stringstream tmp;
    const std::string indent("    ");
    for (int i=0; i < level; i++) {
        tmp << indent;
    }
    return (tmp.str());
}
