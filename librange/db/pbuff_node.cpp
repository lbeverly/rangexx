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

#include <iostream>

#include <boost/make_shared.hpp>

#include "db_exceptions.h"
#include "pbuff_node.h"
#include "../util/crc32.h"

namespace range {
namespace db {

typedef boost::shared_ptr<ProtobufNode> pbuffnode_t;

//##############################################################################
//##############################################################################
static inline bool
write_record(const std::string& name, NodeInfo& info, ProtobufNode::instance_t instance)
{
    info.set_crc32(0);
    uint32_t crc = range::util::crc32(info.SerializeAsString());
    info.set_crc32(crc);

    auto rectype = GraphInstanceInterface::record_type::NODE;
    if(!instance->write_record(rectype, name, info.list_version(), info.SerializeAsString())) {
        return false;
    }
    return true;
}

//##############################################################################
//##############################################################################
template <typename Type>
static inline void
add_unique_new_version(Type item, uint64_t new_version)
{
    for(int vv_idx = item->versions_size() - 1; vv_idx >= 0; --vv_idx) {
        if (new_version == item->versions(vv_idx)) {
            return;
        }
    }
    item->add_versions(new_version);
}


//##############################################################################
//##############################################################################
template <typename Type>
static inline void
update_unique_new_version(Type item, uint64_t cmp_version, uint64_t new_version)
{
    for(int vv_idx = item->versions_size() - 1; vv_idx >= 0; --vv_idx) {
        if (new_version == item->versions(vv_idx)) {
            return;
        }
        if (cmp_version == item->versions(vv_idx)) {
            add_unique_new_version(item, new_version);
        }
    }
}


//##############################################################################
//##############################################################################
static inline range::db::NodeInfo_Tags_KeyValue_Values *
find_value(const std::string& value, NodeInfo_Tags_KeyValue * kv)
{
    for (int val_idx = 0; val_idx < kv->values_size(); ++val_idx) {
        if (value == kv->values(val_idx).data() ) {
            return kv->mutable_values(val_idx);
        }
    }
    return nullptr;
}


//##############################################################################
// Well that's horrifyingly terrible... 
//##############################################################################
static inline void
update_tag_versions(range::db::NodeInfo& info, uint64_t cmp_version, uint64_t new_version) {
    int keys_size = info.tags().keys_size();
    for (int key_idx = 0; key_idx < keys_size; ++key_idx) {
        update_unique_new_version(info.mutable_tags()->mutable_keys(key_idx), cmp_version, new_version);
    }
}


//##############################################################################
//##############################################################################
static inline void
init_default_nodeinfo(NodeInfo& info, uint64_t graph_version)
{
    info.set_node_type(static_cast<int>(graph::NodeIface::node_type::UNKNOWN));
    info.set_list_version(0);
    info.set_crc32(0);
    info.mutable_tags();
    info.mutable_forward();
    info.mutable_reverse();
    info.mutable_graph_versions()->Clear();                                     // We're creating a new one, but we may read_record -> "" twice,
    info.add_graph_versions(graph_version);                                     //   and I don't want (0,0) in the list (I think this can
                                                                                //   only really happen in the unit-tests, but just in case)
}


//##############################################################################
//##############################################################################
static inline void
update_all_edge_versions(NodeInfo& info, uint64_t cmp_version, uint64_t new_version)
{
    for (auto direction : { info.mutable_forward(), info.mutable_reverse() }) {
        for (int i = 0; i < direction->edges_size(); ++i) {
            update_unique_new_version(direction->mutable_edges(i), cmp_version, new_version);
        }
    }
}


//##############################################################################
//##############################################################################
static inline std::vector<std::string>
get_map_value(uint64_t cmp_version, const NodeInfo_Tags_KeyValue& key) 
{
    std::vector<std::string> values;

    for (int values_idx = 0; values_idx < key.values_size(); ++values_idx) {
        auto& value = key.values(values_idx);
        for (int val_idx = value.versions_size() - 1; val_idx >= 0; --val_idx) {
            if (cmp_version == value.versions(val_idx)) {
                values.push_back( value.data() );
            }
            break;
        }
    }
    return values;
}


//##############################################################################
//##############################################################################
inline void
ProtobufNode::init_info() const
{
    if (instance_) {
        if (!info_initialized) {
            std::string buffer;

            {
                auto lock = instance_->read_lock(rectype, name_);
                buffer = instance_->get_record(rectype, name_);
            }

            if (buffer.length() > 0) {
                info.ParseFromString(buffer);
                type_ = node_type(info.node_type());
            } else {                                                            ///< New node
                init_default_nodeinfo(info, instance_->version());
            }
            info_initialized = true;
        }
    }
    else {
        throw InstanceUnitializedException("Instance not initialized for node");
    }
}


//##############################################################################
//##############################################################################
GraphInstanceInterface::lock_t
ProtobufNode::info_lock(bool writable)
{
    if (instance_) {
        auto lock = (writable) ? instance_->write_lock(rectype, name_) : instance_->read_lock(rectype, name_);

        std::string buffer = instance_->get_record(rectype, name_);
        if (buffer.length() > 0) {
            info.ParseFromString(buffer);
            type_ = node_type(info.node_type());
        } else {                                                            ///< New node
            init_default_nodeinfo(info, instance_->version());
        }
        info_initialized = true;
        return lock;
    }
    else {
        throw InstanceUnitializedException("Instance not initialized for node");
    }
}


//##############################################################################
//##############################################################################
inline std::vector<ProtobufNode::node_t>
ProtobufNode::get_edges(const NodeInfo_Edges& direction) const
{
    std::vector<node_t> found_edges;

    uint64_t cmp_version = (wanted_version_ == static_cast<uint64_t>(-1)) ? info.list_version() : wanted_version_;

    for (int i = 0; i < direction.edges_size(); ++i) {
        size_t ver_size = direction.edges(i).versions_size();
        for (int ver_idx = ver_size - 1; ver_idx >= 0; --ver_idx) {
            if (direction.edges(i).versions(ver_idx) == cmp_version) {
                pbuffnode_t n = boost::make_shared<ProtobufNode>( 
                        direction.edges(i).id(), instance_, wanted_version_
                        );
                found_edges.push_back(n);
                break;
            }
        }
    }
    return found_edges;
}


//##############################################################################
//##############################################################################
std::vector<ProtobufNode::node_t>
ProtobufNode::forward_edges() const
{
    init_info();

    if (info.has_forward()) {
        return get_edges(info.forward());
    }
    return std::vector<node_t>();
}


//##############################################################################
//##############################################################################
std::vector<ProtobufNode::node_t>
ProtobufNode::reverse_edges() const
{
    init_info();

    if (info.has_reverse()) {
        return get_edges(info.reverse());
    }
    return std::vector<node_t>();
}


//##############################################################################
//##############################################################################
std::string
ProtobufNode::name() const
{
    return name_;
}


//##############################################################################
//##############################################################################
graph::NodeIface::node_type
ProtobufNode::type() const
{
    init_info();

    return node_type(info.node_type());
}


//##############################################################################
//##############################################################################
uint64_t
ProtobufNode::version() const
{
    init_info();
    return info.list_version();
}

//##############################################################################
//##############################################################################
uint64_t
ProtobufNode::get_wanted_version() const
{
    init_info();
    return info.list_version();
}


//##############################################################################
//##############################################################################
uint32_t
ProtobufNode::crc32() const
{
    init_info();

    return info.crc32();
}


//##############################################################################
//##############################################################################
std::unordered_map<std::string, std::vector<std::string>>
ProtobufNode::tags() const
{
    init_info();
    std::unordered_map<std::string, std::vector<std::string>> tagtable;

    uint64_t cmp_version = (wanted_version_ == static_cast<uint64_t>(-1)) ? info.list_version() : wanted_version_;

    for (int key_idx = 0; key_idx < info.tags().keys_size(); ++key_idx) {
        const auto& key = info.tags().keys(key_idx);
        for (int ver_idx = key.versions_size() - 1; ver_idx >= 0; --ver_idx) {
            uint64_t key_ver = key.versions(ver_idx);
            if (cmp_version == key_ver) {
                tagtable[key.key()] = get_map_value(key.key_version(), key);
            }
        }
    }
    return tagtable;
}


//##############################################################################
//##############################################################################
bool
ProtobufNode::add_forward_edge(node_t other, bool update_other_reverse_edge)
{
    {
        auto lock = info_lock(true);

        for (int edge_idx = 0; edge_idx < info.forward().edges_size(); ++edge_idx) {
            if (other->name() == info.forward().edges(edge_idx).id()) {
                return false;
            }
        }

        uint64_t cmp_version = info.list_version();
        uint64_t new_version = cmp_version + 1;

        info.set_list_version(new_version);

        auto edge = info.mutable_forward()->add_edges();
        edge->set_id(other->name());
        edge->add_versions(new_version);

        update_all_edge_versions(info, cmp_version, new_version);

        if(!write_record(name_, info, instance_)) {
            return false;
        }

        update_tag_versions(info, cmp_version, new_version);
    }

    if (update_other_reverse_edge) {
        other->add_reverse_edge(shared_from_this(), false);
    }
    return true;
}


//##############################################################################
//##############################################################################
bool
ProtobufNode::add_reverse_edge(node_t other, bool update_other_forward_edge)
{
    {
        auto lock = info_lock(true);

        for (int edge_idx = 0; edge_idx < info.reverse().edges_size(); ++edge_idx) {
            if (other->name() == info.reverse().edges(edge_idx).id()) {
                return false;
            }
        }


        uint64_t cmp_version = info.list_version();
        uint64_t new_version = cmp_version + 1;

        info.set_list_version(new_version);

        auto edge = info.mutable_reverse()->add_edges();
        edge->set_id(other->name());
        edge->add_versions(new_version);

        update_all_edge_versions(info, cmp_version, new_version);

        if(!write_record(name_, info, instance_)) {
            return false;
        }

        update_tag_versions(info, cmp_version, new_version);
    }

    if (update_other_forward_edge) { 
        other->add_forward_edge(shared_from_this(), false);
    }
    return true;
}


//##############################################################################
//##############################################################################
bool
ProtobufNode::remove_forward_edge(node_t other, bool update_other_reverse_edge)
{
    {
        auto lock = info_lock(true);

        int edge_idx = 0;
        for (edge_idx = 0; edge_idx < info.forward().edges_size(); ++edge_idx) {
            if (other->name() == info.forward().edges(edge_idx).id()) {
                break;
            }
        }
        if (edge_idx == info.forward().edges_size()) {
            return false;
        }

        uint64_t cmp_version = info.list_version();
        uint64_t new_version = cmp_version + 1;

        int last_ver = info.forward().edges(edge_idx).versions_size();
        int ver_idx;
        for (ver_idx = last_ver - 1; ver_idx >= 0; --ver_idx) {
            if (cmp_version == info.forward().edges(edge_idx).versions(ver_idx)) {
                break;
            }
        }
        if (ver_idx == last_ver) {
            return false;
        }

        info.set_list_version(new_version);
        update_all_edge_versions(info, cmp_version, new_version);

        info.mutable_forward()->mutable_edges(edge_idx)->mutable_versions()->RemoveLast(); // We can safely assume that the last element is new_version, because we
                                                                                           // aleady determined that this element was in the current version (we would
                                                                                           // have returned false earlier otherwise

        if(!write_record(name_, info, instance_)) {
            return false;
        }

        update_tag_versions(info, cmp_version, new_version);
    }

    if (update_other_reverse_edge) {
        other->remove_reverse_edge(shared_from_this(), false);
    }

    return true;
}


//##############################################################################
//##############################################################################
bool
ProtobufNode::remove_reverse_edge(node_t other, bool update_other_forward_edge)
{
    {
        auto lock = info_lock(true);

        int edge_idx = 0;
        for (edge_idx = 0; edge_idx < info.reverse().edges_size(); ++edge_idx) {
            if (other->name() == info.reverse().edges(edge_idx).id()) {
                break;
            }
        }
        if (edge_idx == info.reverse().edges_size()) {
            return false;
        }

        uint64_t cmp_version = info.list_version();
        uint64_t new_version = cmp_version + 1;

        int last_ver = info.reverse().edges(edge_idx).versions_size();
        int ver_idx;
        for (ver_idx = last_ver - 1; ver_idx >= 0; --ver_idx) {
            if (cmp_version == info.reverse().edges(edge_idx).versions(ver_idx)) {
                break;
            }
        }
        if (ver_idx == last_ver) {
            return false;
        }

        info.set_list_version(new_version);
        update_all_edge_versions(info, cmp_version, new_version);

        info.mutable_reverse()->mutable_edges(edge_idx)->mutable_versions()->RemoveLast(); // We can safely assume that the last element is new_version, because we
                                                                                           // aleady determined that this element was in the current version (we would
                                                                                           // have returned false earlier otherwise
        if(!write_record(name_, info, instance_)) {
            return false;
        }

        update_tag_versions(info, cmp_version, new_version);
    }

    if (update_other_forward_edge) {
        other->remove_forward_edge(shared_from_this(), false);
    }
    return true;
}


//##############################################################################
// FIXME: doing more than one thing; refactor
//##############################################################################
bool
ProtobufNode::update_tag(const std::string& key, const std::vector<std::string>& values)
{
    auto lock = info_lock(true);
    int key_idx = 0;
    NodeInfo_Tags_KeyValue * kv = nullptr;

    uint64_t cmp_list_version = info.list_version();
    uint64_t new_list_version = cmp_list_version + 1;

    for (key_idx = 0; key_idx < info.tags().keys_size(); ++key_idx) {
        if (key == info.tags().keys(key_idx).key()) {
            kv = info.mutable_tags()->mutable_keys(key_idx);
            for (int v_idx = kv->versions_size() - 1; v_idx >= 0; --v_idx) {
                if (cmp_list_version == kv->versions(v_idx)) {
                    break;
                }
            }
            break;
        }
    }

    if(!kv) {
        kv = info.mutable_tags()->add_keys();
        kv->set_key_version(-1);
        kv->add_versions(new_list_version);
        kv->set_key(key);
    }

    info.set_list_version(new_list_version);
    update_tag_versions(info, cmp_list_version, new_list_version);
    update_all_edge_versions(info, cmp_list_version, new_list_version);

    uint64_t new_version = kv->key_version() + 1;
    kv->set_key_version(new_version);
    for (auto value : values) {
        auto vptr = find_value(value, kv);

        if (!vptr) { 
            vptr = kv->add_values();
            vptr->set_data(value);
        }
        add_unique_new_version(vptr, new_version);
    }

    if(!write_record(name_, info, instance_)) {
        return false;
    }
    return true;
}


//##############################################################################
//##############################################################################
bool
ProtobufNode::delete_tag(const std::string& key)
{
    auto lock = info_lock(true);

    int key_idx;

    uint64_t cmp_version = info.list_version();
    uint64_t new_version = cmp_version + 1;

    for (key_idx = 0; key_idx < info.tags().keys_size(); ++key_idx) {
        if (key == info.tags().keys(key_idx).key()) {
            break;
        }
    }
    if (key_idx == info.tags().keys_size()) {
        return false;
    }

    info.set_list_version(new_version);

    update_tag_versions(info, cmp_version, new_version);
    info.mutable_tags()->mutable_keys(key_idx)->mutable_versions()->RemoveLast();

    if(!write_record(name_, info, instance_)) {
        return false;
    }

    return true;
}


//##############################################################################
//##############################################################################
bool
ProtobufNode::set_wanted_version(uint64_t version)
{
    wanted_version_ = version;
    return true;
}


//##############################################################################
//##############################################################################
ProtobufNode::node_type
ProtobufNode::set_type(node_type type)
{
    auto lock = info_lock(true);

    uint64_t cmp_version = info.list_version();
    uint64_t new_version = cmp_version + 1;
    info.set_list_version(new_version);

    node_type old_type = node_type(info.node_type());
    info.set_node_type(static_cast<int>(type));

    update_tag_versions(info, cmp_version, new_version);
    update_all_edge_versions(info, cmp_version, new_version);

    write_record(name_, info, instance_);
    return old_type;
}


//##############################################################################
// MUST BE LOCKED WHEN CALLING
//##############################################################################
bool
ProtobufNode::commit()
{
    //auto lock = instance_->write_lock(rectype, name_);
    return write_record(name_, info, instance_);
}

//##############################################################################
//##############################################################################
bool
ProtobufNode::is_valid() const
{
    auto copy = info;
    copy.set_crc32(0);
    uint32_t crc = range::util::crc32(copy.SerializeAsString());

    return info.crc32() == crc;
}

//##############################################################################
//##############################################################################
void
ProtobufNode::add_graph_version(uint64_t version)
{
    auto lock = info_lock(true);
    for(int i = info.graph_versions_size() - 1; i >= 0; --i) {
        if (version == info.graph_versions(i)) {
            return;
        }
    }

    info.add_graph_versions(version);
    write_record(name_, info, instance_);
    return;
}

//##############################################################################
//##############################################################################
std::vector<uint64_t>
ProtobufNode::graph_versions() const
{
    std::vector<uint64_t> vers;
    for (int i = 0; i < info.graph_versions_size(); ++i) {
        vers.push_back(info.graph_versions(i));
    }
    return vers;
}


//##############################################################################
//##############################################################################
ProtobufNode::instance_t
ProtobufNode::set_instance(instance_t instance)
{
    instance_t old_instance = instance_;
    instance_ = instance;
    info_initialized = false;
    return old_instance;
}


//##############################################################################
//##############################################################################
ProtobufNode::instance_t
ProtobufNode::get_instance() const
{
    return instance_;
}
 

} // namespace db
} // namespace range
