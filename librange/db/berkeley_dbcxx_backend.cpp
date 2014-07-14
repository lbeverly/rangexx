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
#include "berkeley_dbcxx_backend.h"
#include "graph_list.pb.h"

namespace range { namespace db {

//##############################################################################
//##############################################################################
BerkeleyDB::BerkeleyDB(const db::ConfigIface &db_config)
    : db_config_(db_config), env_(BerkeleyDBCXXEnv::get(db_config_)), 
        info_(BerkeleyDBCXXDb::get("graph_info", db_config_, env_)), 
        log("BerkeleyDB")
{
}

//##############################################################################
//##############################################################################
BerkeleyDB::graph_instance_t
BerkeleyDB::getGraphInstance(const std::string& name)
{
    this->listGraphInstances();
    auto it = graph_instances_.find(name);
    if(it == graph_instances_.end()) {
        return nullptr;
    }
    return BerkeleyDBCXXDb::get(name, db_config_, env_);
}

//##############################################################################
//##############################################################################
BerkeleyDB::graph_instance_t
BerkeleyDB::createGraphInstance(const std::string& name)
{
    info_->write_lock(record_type::GRAPH_META, "graph_list");

    this->listGraphInstances();
    auto it = graph_instances_.find(name);
    if(it != graph_instances_.end()) {
        return nullptr;
    }

    std::string buf = info_->get_record(record_type::GRAPH_META, "graph_list");
    GraphList listbuf;
    if(!buf.empty()) {
        listbuf.ParseFromString(buf);
    }

    std::string * n = listbuf.add_name();
    n->assign(name);
    info_->commit_record(std::make_tuple(record_type::GRAPH_META, "graph_list", 0, listbuf.SerializeAsString()));
    return this->getGraphInstance(name);
}

//##############################################################################
//##############################################################################
std::vector<std::string>
BerkeleyDB::listGraphInstances() const
{
    info_->read_lock(record_type::GRAPH_META, "graph_list");
    std::string buf = info_->get_record(record_type::GRAPH_META, "graph_list");
    GraphList listbuf;
    if(!buf.empty()) {
        listbuf.ParseFromString(buf);
    }
    std::vector<std::string> instance_names;
    for(int n = 0; n < listbuf.name_size(); ++n) {
        graph_instances_.insert(listbuf.name(n));
        instance_names.push_back(listbuf.name(n));
    }

    return instance_names;
}

//##############################################################################
//##############################################################################
uint64_t
BerkeleyDB::range_version() const
{
    std::string buf = info_->get_record(record_type::GRAPH_META,
            "range_changelist");
    if(!buf.empty()) {
        ChangeList changes;
        changes.ParseFromString(buf);
        return changes.current_version();
    }
    return 0;
}

//##############################################################################
//##############################################################################
void
BerkeleyDB::set_wanted_version(uint64_t version) {
    std::string buf = info_->get_record(record_type::GRAPH_META,
            "range_changelist");
    ChangeList changes;
    if(!buf.empty()) {
        changes.ParseFromString(buf);
    }


    for (uint32_t c_idx = changes.change_size() - 1; c_idx >= version; --c_idx) {
        for (int i_idx = 0; i_idx < changes.change(c_idx).items_size(); ++i_idx) {
            auto item = changes.change(c_idx).items(i_idx);
            graph_wanted_version_map_[item.key()] = item.version();
        }
    }
}

//##############################################################################
//##############################################################################
BerkeleyDB::range_changelist_t
BerkeleyDB::get_changelist()
{
    std::string buf = info_->get_record(record_type::GRAPH_META,
            "range_changelist");

    ChangeList changes;
    if(!buf.empty()) {
        changes.ParseFromString(buf);
    } else {
        return range_changelist_t();
    }

    range_changelist_t changelist;
    for (int c_idx = 0; c_idx < changes.change_size(); ++c_idx) {
        range_change_t c;
        std::time_t t;
        for (int i_idx = 0; i_idx < changes.change(c_idx).items_size(); ++i_idx) {
            auto item = changes.change(c_idx).items(i_idx);
            c[item.key()] = item.version();
            auto ts = changes.change(c_idx).timestamp();
            t = ts.seconds();
        }
        changelist.push_back(std::make_tuple(t, c_idx, c));
    }

    return changelist;
}

//##############################################################################
//##############################################################################
uint64_t
BerkeleyDB::get_graph_wanted_version(const std::string &graph_name) const
{
    auto it = graph_wanted_version_map_.find(graph_name);
    if(it != graph_wanted_version_map_.end()) {
        return it->second;
    }
    return -1;
}

//##############################################################################
//##############################################################################
void
BerkeleyDB::register_thread() const
{
    env_->register_thread();
}

//##############################################################################
//##############################################################################
void
BerkeleyDB::shutdown()
{
    env_->shutdown();
}



} /* namespace db */ } /* namespace range */