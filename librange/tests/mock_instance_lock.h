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

#ifndef _RANGE_TESTS_MOCK_INSTANCE_LOCK_H
#define _RANGE_TESTS_MOCK_INSTANCE_LOCK_H


#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../db/db_interface.h"

//##############################################################################
//##############################################################################
class MockInstanceLock : public range::db::GraphInstanceLock {
    public:
        MOCK_METHOD0(unlock, void(void));
        MOCK_METHOD0(readonly, bool(void));

        virtual ~MockInstanceLock() noexcept override {
            try {
                unlock();
            }
            catch(...) {
                return;
            }
        }
};

#endif
