// Copyright (C) 2016 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
/// @file yang-kea.h

#ifndef YANG_KEA_H
#define YANG_KEA_H

extern "C" {
#include "sysrepo.h"
};

#include <string>

/// @brief convenient funtion that generates spaces for specified
///        indentation level
///
/// For example for 1 it generates string("    "), for 2
/// it is string("        ") and so on.
///
/// @param int identation level
///
/// @return a string with appropriate number of spaces.
std::string tabs(int level);

class SysrepoKea {
public:
    /// @brief converts sr_type_t to textual form
    ///
    /// @param type type to be converted
    /// @return printable representation of the type
    static std::string srTypeToText(sr_type_t type);

    /// @brief Converts sysrepo value into a string representation.
    ///
    /// @param value pointer to the sr_var_t object to be represented
    /// @param xpath should the xpath information be printed?
    /// @param type should the type be printed?
    ///
    /// @return string representing the value
    static std::string
    valueToText(sr_val_t *value, bool xpath = false,
                bool type = false);

    std::string
    getConfig(sr_session_ctx_t* session);

private:
    std::string
    get_pool(sr_session_ctx_t* session, const char *xpath,int indent);

    std::string
    get_subnet(sr_session_ctx_t* session, const char *xpath, int indent);

    std::string
    get_pools(sr_session_ctx_t* session, const char *xpath, int indent);
};

#endif /* YANG_KEA_H */
