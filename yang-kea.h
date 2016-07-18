// Copyright (C) 2016 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
/// @file yang-kea.h

#ifndef YANG_KEA_H
#define YANG_KEA_H

#include "sysrepo.h"
#include <string>


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
    std::string
    valueToText(sr_val_t *value, bool xpath = false,
                bool type = false);
};

#endif /* YANG_KEA_H */
