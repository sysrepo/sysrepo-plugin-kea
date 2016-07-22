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
    /// Specifies the default value of a model.
    const static char* DEFAULT_MODEL_NAME;

    /// @brief Constructor
    ///
    /// @param session a Sysrepo session to be used.
    SysrepoKea(sr_session_ctx_t* session);

    /// @brief Returns the model name.
    std::string getModelName() {
        return (model_name_);
    }

    /// @brief Sets the model name
    ///
    /// If not used, the DEFAULT_MODEL_NAME is used.
    ///
    /// @param name name of the model to be used.
    void setModelName(const std::string& name) {
        model_name_ = name;
    }

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

    /// @brief Retrieves config from Sysrepo and generates Kea config
    ///        in JSON format.
    ///
    /// @param returns Kea config in JSON format.
    std::string getConfig();

private:
    /// @brief Returns a pool specified by xpath as JSON text
    ///
    /// @param xpath XPath to the pool to be returned
    /// @param indent indentation level
    ///
    /// @return string with specified pool as JSON text
    std::string getPool(const char *xpath, int indent);

    /// @brief Returns array of pools specified by xpath as JSON text
    ///
    /// @param xpath XPath to the pools list to be returned
    /// @param indent indentation level
    ///
    /// @return string with specified pools array as JSON text
    std::string getPools(const char *xpath, int indent);

    /// @brief Returns a Subnet specified by xpath as JSON text
    ///
    /// @param xpath XPath to the subnet to be returned
    /// @param indent indentation level
    ///
    /// @return string of JSON text
    std::string getSubnet(const char *xpath, int indent);


    /// @brief Returns array of subnets specified by xpath as JSON text
    ///
    /// @param xpath XPath to the subnets list to be returned
    /// @param indent indentation level
    ///
    /// @return string with specified subnets array as JSON text
    std::string getSubnets(const std::string& xpath, int indent);

    /// @brief Returns a value specified by xpath as JSON text
    ///
    /// @param xpath XPath to the value to be returned
    ///
    /// @return string with specified element as JSON text
    std::string getValue(const std::string& xpath);

    /// @brief Returns a formatted value of element specified by xpath as JSON text
    ///
    /// @param xpath XPath to the element to be returned
    /// @param json_name Name of the JSON parameter to be produced
    /// @param indent indentation level
    /// @param comma whether the JSON structure should end with a comma
    ///
    /// @return string with specified element as JSON text
    std::string getFormattedValue(const std::string& xpath,
                                  const std::string& json_name, int indent,
                                  bool comma = true);

    std::string model_name_; ///< Model name (usually /ietf-kea-dhcpv6:server/)

    /// Sysrepo session (must be valid for the whole time this
    /// object's lifetime)
    sr_session_ctx_t* session_;
};

#endif /* YANG_KEA_H */
