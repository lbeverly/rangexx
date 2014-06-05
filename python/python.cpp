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

#include <mutex>

#include <boost/python.hpp>

#include <dbstl_exception.h>

#include "../librange/core/api.h"
#include "python_visitor.h"

using namespace boost::python;
using namespace range;
using node_type = range::graph::NodeIface::node_type;

//##############################################################################
//##############################################################################
class APIWrap
{
    public:
        //######################################################################
        APIWrap(std::string configfile) : api_(new RangeAPI_v1(configfile)) { }

        //######################################################################
        object all_clusters(const std::string &env_name, uint64_t version=-1) const
        {
            RangeStruct top = api_->all_clusters(env_name, version);
            return boost::apply_visitor(PythonVisitor(), top);
        }

        //######################################################################
        object all_environments(uint64_t version=-1) const
        {
            RangeStruct top = api_->all_environments(version);
            return boost::apply_visitor(PythonVisitor(), top);
        }

        //######################################################################
        object all_hosts(uint64_t version=-1) const
        {
            RangeStruct top = api_->all_hosts(version);
            return boost::apply_visitor(PythonVisitor(), top);
        }

        //######################################################################
        object expand_range_expression(const std::string &env_name,
                const std::string &expression, uint64_t version=-1) const
        {
            RangeStruct top = api_->expand_range_expression(env_name, expression, version);
            return boost::apply_visitor(PythonVisitor(), top);
        }

        //######################################################################
        object simple_expand(const std::string &env_name, const std::string &node_name,
                uint64_t version=-1, node_type type=node_type::UNKNOWN) const
        {
            RangeStruct top = api_->simple_expand(env_name, node_name, version, type);
            return boost::apply_visitor(PythonVisitor(), top);
        }

        //######################################################################
        object simple_expand_cluster(const std::string &env_name, const std::string &cluster_name,
                uint64_t version=-1) const
        {
            RangeStruct top = api_->simple_expand_cluster(env_name, cluster_name, version);
            return boost::apply_visitor(PythonVisitor(), top);
        }
        
        //######################################################################
        object simple_expand_env(const std::string &env_name, uint64_t version=-1) const
        {
            RangeStruct top = api_->simple_expand_env(env_name, version);
            return boost::apply_visitor(PythonVisitor(), top);
        }
        
        //######################################################################
        object get_keys(const std::string &env_name, const std::string &node_name,
                uint64_t version=-1) const
        {
            RangeStruct top = api_->get_keys(env_name, node_name, version);
            return boost::apply_visitor(PythonVisitor(), top);
        }
        
        //######################################################################
        object fetch_key(const std::string &env_name, const std::string &node_name,
                const std::string &key, uint64_t version=-1) const
        {
            RangeStruct top = api_->fetch_key(env_name, node_name, key, version);
            return boost::apply_visitor(PythonVisitor(), top);
        }

        //######################################################################
        object fetch_all_keys(const std::string &env_name,
                const std::string &node_name, uint64_t version=-1) const
        {
            RangeStruct top = api_->fetch_all_keys(env_name, node_name, version);
            return boost::apply_visitor(PythonVisitor(), top);
        }

        //######################################################################
        object expand(const std::string &env_name,
                const std::string &node_name,
                uint64_t version=-1,
                size_t depth=std::numeric_limits<size_t>::max()) const 
        {
            RangeStruct top = api_->expand(env_name, node_name, version, depth);
            return boost::apply_visitor(PythonVisitor(), top);
        }
        
        //######################################################################
        object expand_cluster(const std::string &env_name,
                const std::string &cluster_name,
                uint64_t version=-1,
                size_t depth=std::numeric_limits<size_t>::max()) const 
        {
            RangeStruct top = api_->expand_cluster(env_name, cluster_name, version, depth);
            return boost::apply_visitor(PythonVisitor(), top);
        }

        //######################################################################
        object expand_env( const std::string &env_name, uint64_t version=-1,
                size_t depth=std::numeric_limits<size_t>::max()) const
        {
            RangeStruct top = api_->expand_env(env_name, version, depth);
            return boost::apply_visitor(PythonVisitor(), top);
        }

        //######################################################################
        object get_clusters(const std::string &env_name,
                            const std::string &node_name,
                            uint64_t version=-1) const
        {
            RangeStruct top = api_->get_clusters(env_name, node_name, version);
            return boost::apply_visitor(PythonVisitor(), top);
        }

        //######################################################################
        object bfs_search_parents_for_first_key(const std::string &env_name,
                                             const std::string &node_name,
                                             const std::string &key,
                                             uint64_t version=-1) const
        {
            RangeStruct top = api_->bfs_search_parents_for_first_key(env_name, node_name, key, version);
            return boost::apply_visitor(PythonVisitor(), top);
        }

        //######################################################################
        object dfs_search_parents_for_first_key(const std::string &env_name,
                                             const std::string &node_name,
                                             const std::string &key,
                                             uint64_t version=-1) const
        {
            RangeStruct top = api_->dfs_search_parents_for_first_key(env_name, node_name, key, version);
            return boost::apply_visitor(PythonVisitor(), top);
        }

        //######################################################################
        object nearest_common_ancestor( const std::string &env_name,
                                    const std::string &node1_name,
                                    const std::string &node2_name,
                                    uint64_t version=-1) const
        {
            RangeStruct top = api_->nearest_common_ancestor(env_name, node1_name, node2_name, version);
            return boost::apply_visitor(PythonVisitor(), top);
        }

        //######################################################################
        object environment_topological_sort(const std::string &env_name,
                                            uint64_t version=-1) const
        {
            RangeStruct top = api_->environment_topological_sort(env_name, version);
            return boost::apply_visitor(PythonVisitor(), top);
        }

        //######################################################################
        object find_orphaned_nodes(uint64_t version=-1) const
        {
            RangeStruct top = api_->find_orphaned_nodes(version);
            return boost::apply_visitor(PythonVisitor(), top);
        }

        
        //######################################################################
        //######################################################################

        //######################################################################
        bool create_env(const std::string &env_name) 
        {
            return api_->create_env(env_name);
        }

        //######################################################################
        bool remove_env(const std::string &env_name)
        {
            return api_->remove_env(env_name);
        }

        //######################################################################
        bool add_cluster_to_env(const std::string &env_name,
                                        const std::string &cluster_name)
        {
            return api_->add_cluster_to_env(env_name, cluster_name);
        }

        //######################################################################
        bool remove_cluster_from_env(const std::string &env_name,
                                        const std::string &cluster_name)
        {
            return api_->remove_cluster_from_env(env_name, cluster_name);
        }


        //######################################################################
        bool add_cluster_to_cluster(const std::string &env_name,
                                            const std::string &parent_cluster,
                                            const std::string &child_cluster)
        {
            return api_->add_cluster_to_cluster(env_name, parent_cluster, child_cluster);
        }


        //######################################################################
        bool remove_cluster_from_cluster(const std::string &env_name,
                                                 const std::string &parent_cluster,
                                                 const std::string &child_cluster)
        {
            return api_->remove_cluster_from_cluster(env_name, parent_cluster, child_cluster);
        }

        //######################################################################
        bool remove_cluster(const std::string &env_name,
                                    const std::string &cluster_name)
        {
            return api_->remove_cluster(env_name, cluster_name);
        }


        //######################################################################
        bool add_host_to_cluster(const std::string &env_name,
                                         const std::string &parent_cluster,
                                         const std::string &hostname)
        {
            return api_->add_host_to_cluster(env_name, parent_cluster, hostname);
        }


        //######################################################################
        bool remove_host_from_cluster(const std::string &env_name,
                                              const std::string &parent_cluster,
                                              const std::string &hostname)
        {
            return api_->remove_host_from_cluster(env_name, parent_cluster, hostname);
        }

        //######################################################################
        bool add_host(const std::string &hostname)
        {
            return api_->add_host(hostname);
        }

        //######################################################################
        bool remove_host(const std::string &env_name,
                                 const std::string &hostname)
        {
            return api_->remove_host(env_name, hostname);
        }


        //######################################################################
        bool add_node_key_value(const std::string &env_name,
                                        const std::string &node_name,
                                        const std::string &key,
                                        const std::string &value)
        {
            return api_->add_node_key_value(env_name, node_name, key, value);
        }

        //######################################################################
        bool remove_node_key_value(const std::string &env_name,
                                        const std::string &node_name,
                                        const std::string &key,
                                        const std::string &value)
        {
            return api_->remove_node_key_value(env_name, node_name, key, value);
        }

        //######################################################################
        bool remove_key_from_node(const std::string &env_name,
                                          const std::string &node_name,
                                          const std::string &key)
        {
            return api_->remove_key_from_node(env_name, node_name, key);
        }

        //######################################################################
        bool add_node_env_dependency(const std::string &env_name,
                                             const std::string &node_name,
                                             const std::string &dependency_name)
        {
            return api_->add_node_env_dependency(env_name, node_name, dependency_name);
        }

        //######################################################################
        bool remove_node_env_dependency(const std::string &env_name,
                                                const std::string &node_name,
                                                const std::string &dependency_name)
        {
            return api_->remove_node_env_dependency(env_name, node_name, dependency_name);
        }

        //######################################################################
        bool add_node_ext_dependency(const std::string &env_name,
                                             const std::string &node_name,
                                             const std::string &dependency_env,
                                             const std::string &dependency_name)
        {
            return api_->add_node_ext_dependency(env_name, node_name, dependency_env, dependency_name);
        }

        //######################################################################
        bool remove_node_ext_dependency(const std::string &env_name,
                                                const std::string &node_name,
                                                const std::string &dependency_env,
                                                const std::string &dependency_name)
        {
            return api_->remove_node_ext_dependency(env_name, node_name, dependency_env, dependency_name);
        }

    private:
        boost::shared_ptr<RangeAPI_v1> api_;

};


//##############################################################################
// READ API
//##############################################################################
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_all_clusters, all_clusters, 1, 2);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_all_environments, all_environments, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_all_hosts, all_hosts, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_expand_range_expression, expand_range_expression, 2, 3);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_simple_expand, simple_expand, 2, 4);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_simple_expand_cluster, simple_expand_cluster, 2, 3);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_simple_expand_env, simple_expand_env, 1, 2);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_get_keys, get_keys, 2, 3);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_fetch_key, fetch_key, 3, 4);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_fetch_all_keys, fetch_all_keys, 2, 3);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_expand, expand, 2, 4);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_expand_cluster, expand_cluster, 2, 4);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_expand_env, expand_env, 1, 3);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_get_clusters, get_clusters, 2, 3);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_bfs_search_parents_for_first_key, bfs_search_parents_for_first_key, 3, 4);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_dfs_search_parents_for_first_key, dfs_search_parents_for_first_key, 3, 4);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_nearest_common_ancestor, nearest_common_ancestor, 3, 4);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_environment_topological_sort, environment_topological_sort, 1, 2);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(overloads_find_orphaned_nodes, find_orphaned_nodes, 0, 1);

BOOST_PYTHON_MODULE(librange_python) {

    class_<APIWrap>("Range", init<std::string>())
        .def("all_clusters", &APIWrap::all_clusters, overloads_all_clusters())
        .def("all_environments", &APIWrap::all_environments, overloads_all_environments())
        .def("all_hosts", &APIWrap::all_hosts, overloads_all_hosts())
        .def("expand_range_expression", &APIWrap::expand_range_expression, overloads_expand_range_expression())
        .def("simple_expand", &APIWrap::simple_expand, overloads_simple_expand())
        .def("simple_expand_cluster", &APIWrap::simple_expand_cluster, overloads_simple_expand_cluster())
        .def("simple_expand_env", &APIWrap::simple_expand_env, overloads_simple_expand_env())
        .def("get_keys", &APIWrap::get_keys, overloads_get_keys())
        .def("fetch_key", &APIWrap::fetch_key, overloads_fetch_key())
        .def("fetch_all_keys", &APIWrap::fetch_all_keys, overloads_fetch_all_keys())
        .def("expand", &APIWrap::expand, overloads_expand())
        .def("expand_cluster", &APIWrap::expand_cluster, overloads_expand_cluster())
        .def("expand_env", &APIWrap::expand_env, overloads_expand_env())
        .def("get_clusters", &APIWrap::get_clusters, overloads_get_clusters())
        .def("bfs_search_parents_for_first_key", &APIWrap::bfs_search_parents_for_first_key, overloads_bfs_search_parents_for_first_key())
        .def("dfs_search_parents_for_first_key", &APIWrap::dfs_search_parents_for_first_key, overloads_dfs_search_parents_for_first_key())
        .def("nearest_common_ancestor", &APIWrap::nearest_common_ancestor, overloads_nearest_common_ancestor())
        .def("environment_topological_sort", &APIWrap::environment_topological_sort, overloads_environment_topological_sort())
        .def("find_orphaned_nodes", &APIWrap::find_orphaned_nodes, overloads_find_orphaned_nodes())
        .def("create_env", &APIWrap::create_env)
        .def("remove_env", &APIWrap::remove_env)
        .def("add_cluster_to_env", &APIWrap::add_cluster_to_env)
        .def("remove_cluster_from_env", &APIWrap::remove_cluster_from_env)
        .def("add_cluster_to_cluster", &APIWrap::add_cluster_to_cluster)
        .def("remove_cluster_from_cluster", &APIWrap::remove_cluster_from_cluster)
        .def("remove_cluster", &APIWrap::remove_cluster)
        .def("add_host_to_cluster", &APIWrap::add_host_to_cluster)
        .def("remove_host_from_cluster", &APIWrap::remove_host_from_cluster)
        .def("add_host", &APIWrap::add_host)
        .def("remove_host", &APIWrap::remove_host)
        .def("add_node_key_value", &APIWrap::add_node_key_value)
        .def("remove_node_key_value", &APIWrap::remove_node_key_value)
        .def("remove_key_from_node", &APIWrap::remove_key_from_node)
        .def("add_node_env_dependency", &APIWrap::add_node_env_dependency)
        .def("remove_node_env_dependency", &APIWrap::remove_node_env_dependency)
        .def("add_node_ext_dependency", &APIWrap::add_node_ext_dependency)
        .def("remove_node_ext_dependency", &APIWrap::remove_node_ext_dependency)
        ;
}

