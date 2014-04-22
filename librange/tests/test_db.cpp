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
//#include <cstdlib>

#include <boost/make_shared.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <google/protobuf/message.h>

#include "mock_db_config.h"
#include "../db/db.h"
#include "../db/config_interface.h"

using namespace ::testing;

//##############################################################################
//##############################################################################
class TestDB : public ::testing::Test {
    public:
        static void SetUpTestCase() {
            char p[] = "/tmp/db_test_env.XXXXXXXXXX"; 
            if(!mkdtemp(p)) {
                throw "AAAAGGGGHHHH";
            }
            path = p;
        }

        virtual void SetUp() override {
            EXPECT_CALL(cfg, db_home())
                .Times(AtLeast(0))
                .WillRepeatedly(ReturnRef(path));

            EXPECT_CALL(cfg, cache_size())
                .Times(AtLeast(0))
                .WillRepeatedly(Return(67108864));
        }

        MockDbConfig cfg;
        static std::string path;
};

std::string TestDB::path = "";
   
//##############################################################################
//##############################################################################
TEST_F(TestDB, test_db_ctor) {
    range::db::BerkeleyDB db { cfg };
}
     
//##############################################################################
//##############################################################################
int
main(int argc, char **argv)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    ::testing::InitGoogleTest(&argc, argv);
    int rval = RUN_ALL_TESTS();
    range::db::BerkeleyDB::s_shutdown();
 return rval;
}
