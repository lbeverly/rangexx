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
#include <stack>

#include <boost/make_shared.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <google/protobuf/message.h>
#include "../graph/graphdb.h"
#include "../graph/node_interface.h"
#include "../graph/graph_interface.h"
#include "../db/pbuff_node.h"
#include "../graph/node_factory.h"

using namespace ::testing;

#include "mock_instance.h"
#include "mock_node.h"
#include "mock_cursor.h"
#include "mock_graph.h"
#include "mock_transaction.h"
#include "mock_instance_lock.h"

//##############################################################################
//##############################################################################
class TestGraphDB: public ::testing::Test {
    //##########################################################################
    public:
};

//##############################################################################
//##############################################################################
TEST_F(TestGraphDB, test_ctor) {
    auto inst = boost::make_shared<MockInstance>();
    range::graph::GraphDB gdb { "primary", inst, range::graph::GraphDB::node_factory_t(new range::graph::NodeIfaceConcreteFactory<MockNode>()) };
}

//##############################################################################
//##############################################################################
TEST_F(TestGraphDB, test_V) {
    auto inst = boost::make_shared<MockInstance>();
    EXPECT_CALL(*inst, n_vertices())
        .Times(1)
        .WillOnce(Return(5));

    range::graph::GraphDB gdb { "primary", inst, range::graph::GraphDB::node_factory_t(new range::graph::NodeIfaceConcreteFactory<MockNode>()) };

    EXPECT_EQ(5, gdb.V());
}

//##############################################################################
//##############################################################################
TEST_F(TestGraphDB, test_E) {
    auto inst = boost::make_shared<MockInstance>();
    EXPECT_CALL(*inst, n_edges())
        .Times(1)
        .WillOnce(Return(9));

    range::graph::GraphDB gdb { "primary", inst, range::graph::GraphDB::node_factory_t(new range::graph::NodeIfaceConcreteFactory<MockNode>()) };

    EXPECT_EQ(9, gdb.E());
}

//##############################################################################
//##############################################################################
TEST_F(TestGraphDB, test_version) {
    auto inst = boost::make_shared<MockInstance>();
    EXPECT_CALL(*inst, version())
        .Times(1)
        .WillOnce(Return(99));

    range::graph::GraphDB gdb { "primary", inst, range::graph::GraphDB::node_factory_t(new range::graph::NodeIfaceConcreteFactory<MockNode>()) };

    EXPECT_EQ(99, gdb.version());
}

//##############################################################################
//##############################################################################
TEST_F(TestGraphDB, test_wanted_version) {
    auto inst = boost::make_shared<MockInstance>();
    EXPECT_CALL(*inst, version())
        .Times(3)
        .WillRepeatedly(Return(99));

    EXPECT_CALL(*inst, n_vertices())
        .Times(1)
        .WillRepeatedly(Return(10));

    std::list<::range::db::GraphInstanceInterface::changelist_t> clist;
    for(int i = 0; i < 99; ++i) {
        clist.push_back(::range::db::GraphInstanceInterface::changelist_t());
    }

    EXPECT_CALL(*inst, get_change_history())
        .Times(1)
        .WillRepeatedly(Return(clist));


    range::graph::GraphDB gdb { "primary", inst, range::graph::GraphDB::node_factory_t(new range::graph::NodeIfaceConcreteFactory<MockNode>()) };

    EXPECT_EQ(true, gdb.set_wanted_version(99));
    EXPECT_EQ(99, gdb.get_wanted_version());
    EXPECT_NE(true, gdb.set_wanted_version(100));
    EXPECT_EQ(99, gdb.get_wanted_version());
}

//##############################################################################
//##############################################################################
TEST_F(TestGraphDB, test_wanted_version_history) {
    auto node = boost::make_shared<MockNode>();


}


//##############################################################################
//##############################################################################
#define UNUSED(x) (void)(x)
struct MockNodeFactory : public range::graph::NodeIfaceAbstractFactory {
    MockNodeFactory(std::stack<boost::shared_ptr<MockNode>> return_nodes) : return_nodes_(return_nodes) { }
    virtual node_t createNode(const std::string& name, instance_t instance) override
    {
        UNUSED(name);
        UNUSED(instance);

        auto ret = return_nodes_.top();
        return_nodes_.pop();
        return ret;
    }

    std::stack<boost::shared_ptr<MockNode>> return_nodes_;
};



//##############################################################################
//##############################################################################
TEST_F(TestGraphDB, test_create) {
    auto inst = boost::make_shared<MockInstance>();
    auto txn = boost::make_shared<MockTransaction>();
    auto cur = boost::make_shared<MockCursor>();

    std::vector<range::graph::NodeIface::node_t> MockNodes;

    boost::shared_ptr<MockNode> thisnode = boost::make_shared<MockNode>();

    EXPECT_CALL(*txn, flush())
        .Times(AtLeast(1));

    EXPECT_CALL(*thisnode, name())
        .Times(AtLeast(0))
        .WillRepeatedly(Return("foobar"));

    EXPECT_CALL(*thisnode, commit())
        .Times(1)
        .WillOnce(Return(true));

    EXPECT_CALL(*thisnode, version())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(0));

    EXPECT_CALL(*thisnode, graph_versions())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(std::vector<uint64_t>()));
    
    for (int i = 0; i < 10; ++i) {
        boost::shared_ptr<MockNode> node = boost::make_shared<MockNode>();

        EXPECT_CALL(*node, add_graph_version(100))
            .Times(AtLeast(1));

        EXPECT_CALL(*node, name())
            .Times(AtLeast(0))
            .WillRepeatedly(Return("Node" + std::to_string(i)));

        MockNodes.push_back(node);
    }

    EXPECT_CALL(*cur, first())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(MockNodes[0]));

    EXPECT_CALL(*cur, next())
        .Times(10)
        .WillOnce(Return(MockNodes[1]))
        .WillOnce(Return(MockNodes[2]))
        .WillOnce(Return(MockNodes[3]))
        .WillOnce(Return(MockNodes[4]))
        .WillOnce(Return(MockNodes[5]))
        .WillOnce(Return(MockNodes[6]))
        .WillOnce(Return(MockNodes[7]))
        .WillOnce(Return(MockNodes[8]))
        .WillOnce(Return(MockNodes[9]))
        .WillOnce(Return(nullptr))
        .RetiresOnSaturation();

    EXPECT_CALL(*cur, next(MockNodes[9]))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(nullptr));


    EXPECT_CALL(*cur, fetch("Node0"))
        .Times(1)
        .WillOnce(Return(MockNodes[0]));

    EXPECT_CALL(*cur, last())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(MockNodes[9]));

    EXPECT_CALL(*inst, version())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(99));

    EXPECT_CALL(*inst, start_txn())
        .Times(1)
        .WillOnce(Return(txn));

    EXPECT_CALL(*inst, get_cursor())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(cur));

    std::stack<boost::shared_ptr<MockNode>> nodestack { {thisnode} };
    range::graph::GraphDB gdb { "primary", inst, range::graph::GraphDB::node_factory_t(new MockNodeFactory(nodestack)) };
    gdb.create("foobar");
}

//##############################################################################
//##############################################################################
TEST_F(TestGraphDB, test_remove) {
    auto inst = boost::make_shared<MockInstance>();
    auto txn = boost::make_shared<MockTransaction>();
    auto cur = boost::make_shared<MockCursor>();
    auto thisnode = boost::make_shared<MockNode>();

    std::vector<range::graph::NodeIface::node_t> MockNodes;

    EXPECT_CALL(*txn, flush())
        .Times(AtLeast(1));

    EXPECT_CALL(*thisnode, name())
        .Times(AtLeast(0))
        .WillRepeatedly(Return("foobar"));

    EXPECT_CALL(*thisnode, graph_versions())
        .Times(1)
        .WillOnce(Return(std::vector<uint64_t>({ 99, 100 })));

    std::vector<boost::shared_ptr<range::graph::NodeIface>> thisnode_forwardedges;
    std::vector<boost::shared_ptr<range::graph::NodeIface>> thisnode_reverseedges;
    
    for (int i = 0; i < 10; ++i) {
        boost::shared_ptr<MockNode> node = boost::make_shared<MockNode>();

        EXPECT_CALL(*node, add_graph_version(100))
            .Times(AtLeast(1));

         EXPECT_CALL(*node, name())
            .Times(AtLeast(0))
            .WillRepeatedly(Return("Node" + std::to_string(i)));
   
        if ( i % 3 == 0 ) { 
            thisnode_forwardedges.push_back(node);
            EXPECT_CALL(*node, remove_reverse_edge(Matcher<range::graph::NodeIface::node_t>(thisnode), false))
                .Times(AtLeast(1))
                .WillRepeatedly(Return(true));
        }

        if ( i != 0 && i % 4 == 0 ) { 
            thisnode_reverseedges.push_back(node);
            EXPECT_CALL(*node, remove_forward_edge(Matcher<range::graph::NodeIface::node_t>(thisnode), false))
                .Times(AtLeast(1))
                .WillRepeatedly(Return(true));
        }

        MockNodes.push_back(node);
    }

    EXPECT_CALL(*thisnode, forward_edges())
        .Times(1)
        .WillOnce(Return(thisnode_forwardedges));

    EXPECT_CALL(*thisnode, reverse_edges())
        .Times(1)
        .WillOnce(Return(thisnode_reverseedges));

    EXPECT_CALL(*cur, first())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(MockNodes[0]));

    EXPECT_CALL(*cur, next())
        .Times(10)
        .WillOnce(Return(MockNodes[1]))
        .WillOnce(Return(MockNodes[2]))
        .WillOnce(Return(MockNodes[3]))
        .WillOnce(Return(MockNodes[4]))
        .WillOnce(Return(MockNodes[5]))
        .WillOnce(Return(MockNodes[6]))
        .WillOnce(Return(MockNodes[7]))
        .WillOnce(Return(MockNodes[8]))
        .WillOnce(Return(MockNodes[9]))
        .WillOnce(Return(nullptr))
        .RetiresOnSaturation();

    EXPECT_CALL(*cur, next(MockNodes[9]))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(nullptr));

    EXPECT_CALL(*cur, fetch("Node0"))
        .Times(1)
        .WillOnce(Return(MockNodes[0]));

    EXPECT_CALL(*cur, last())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(MockNodes[9]));

    EXPECT_CALL(*inst, version())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(99));

    /* EXPECT_CALL(*inst, get_record(range::db::GraphInstanceInterface::record_type::NODE, "foobar"))
        .Times(AtLeast(1))
        .WillRepeatedly(Return("")); */

    EXPECT_CALL(*inst, start_txn())
        .Times(1)
        .WillOnce(Return(txn));

    EXPECT_CALL(*inst, get_cursor())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(cur));


    range::graph::GraphDB gdb { "primary", inst, range::graph::GraphDB::node_factory_t(new range::graph::NodeIfaceConcreteFactory<MockNode>()) };
    gdb.remove(thisnode);

    std::for_each(std::begin(MockNodes), std::end(MockNodes), [](boost::shared_ptr<range::graph::NodeIface> p) { Mock::VerifyAndClearExpectations(p.get()); });
    Mock::VerifyAndClearExpectations(thisnode.get());
}















int
main(int argc, char **argv)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    ::testing::InitGoogleTest(&argc, argv);
    range::db::ProtobufNode::s_shutdown();
    return RUN_ALL_TESTS();
}

