/*
 * This file is part of range++.
 *
 * range++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * range++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with range++.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _RANGE_CORE_EXCEPTION_H
#define _RANGE_CORE_EXCEPTION_H

#include <stdexcept>
#include "log.h"
#include "../util/demangle.h"

namespace range {

extern EmitterModuleRegistration ExceptionLogModule;

//##############################################################################
//##############################################################################
struct Exception : public std::runtime_error::runtime_error {
    Exception(const std::string& what,
        const std::string &event="Exception")
        : std::runtime_error::runtime_error(what) 
    { 
        try {
            auto log = range::Emitter(ExceptionLogModule);
            log.notice(event, what);
        } catch(...) { }
    }
    virtual int vtable(void) { return 1; }
};

//##############################################################################
//##############################################################################
struct InvalidTimespecException : public Exception {
    InvalidTimespecException(const std::string &what, 
            const std::string &event="InvalidTimespecException") 
        : Exception(what, event)
        { }
};

//##############################################################################
//##############################################################################
struct IncorrectNumberOfArguments : public Exception {
    IncorrectNumberOfArguments(const std::string &what, 
            const std::string &event="IncorrectNumberOfArguments")
        : Exception(what, event)
        { }
};

//##############################################################################
//##############################################################################
struct NodeExistsException : public Exception {
    NodeExistsException(const std::string &what, 
            const std::string &event="NodeExistsException")
        : Exception(what, event)
        { }
};

//##############################################################################
//##############################################################################
struct CreateNodeException : public Exception {
    CreateNodeException(const std::string &what, 
            const std::string &event="CreateNodeException")
        : Exception(what, event)
        { }
};

//##############################################################################
//##############################################################################
struct InvalidEnvironmentException : public Exception {
    InvalidEnvironmentException(const std::string &what, 
            const std::string &event="NodeExistsException")
        : Exception(what, event)
        { }
};


namespace stored {

//##############################################################################
//##############################################################################
struct MqueueException : public ::range::Exception {
    MqueueException(const std::string& what,
        const std::string &event="stored.MqueueException") 
        : Exception(what, event)
    { }
};

} /* namespace stored */

} // namespace range

#endif
