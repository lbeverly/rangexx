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

#ifndef _RANGE_DB_CONFIG_INTERFACE_H
#define _RANGE_DB_CONFIG_INTERFACE_H

#include <string>

namespace range {
namespace db {

class ConfigIface {
    public:
        ConfigIface() : db_home_("/var/lib/rangexx"), cache_size_(67108864) { }
        ConfigIface(std::string db_home, size_t cache_size) 
            : db_home_(db_home), cache_size_(cache_size)
        {
        }
        virtual ~ConfigIface() = default;

        virtual const std::string& db_home() const { return db_home_; }
        virtual size_t cache_size() const { return cache_size_; }

    private:
        std::string db_home_;
        size_t cache_size_;

};


} // namespace db
} // namespace range

#endif

