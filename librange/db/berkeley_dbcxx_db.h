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
#ifndef _RANGEXX_DB_BERKELEY_DBCXX_DB_H
#define _RANGEXX_DB_BERKELEY_DBCXX_DB_H

#include <unordered_map>

#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <db_cxx.h>

#include "../core/log.h"

#include "db_interface.h"
#include "config_interface.h"
#include "changelist.pb.h"

#include "berkeley_dbcxx_env.h"
#include "berkeley_dbcxx_txn.h"
#include "berkeley_dbcxx_lock.h"

namespace range { namespace db {
class BerkeleyDB;
//##############################################################################
//##############################################################################
class BerkeleyDBCXXDb : public GraphInstanceInterface,
    public boost::enable_shared_from_this<BerkeleyDBCXXDb> 
{
    public:
        static boost::shared_ptr<BerkeleyDBCXXDb> get(const std::string &name,
                boost::shared_ptr<BerkeleyDB> backend,
                const boost::shared_ptr<db::ConfigIface> db_config);
        static boost::shared_ptr<BerkeleyDBCXXDb> get(const std::string &name,
                boost::shared_ptr<BerkeleyDB> backend,
                const boost::shared_ptr<db::ConfigIface> db_config,
                boost::shared_ptr<BerkeleyDBCXXEnv> env);
        static void close_all_db() { multiton_map_.clear(); };

        virtual size_t n_vertices() const override;
        virtual size_t n_edges() const override;
        virtual size_t n_redges() const override;
        virtual uint64_t version() const override;
        virtual cursor_t get_cursor() const override;
        virtual std::string get_record(record_type type, const std::string& key) const override;
        virtual lock_t read_lock(record_type type, const std::string& key) const override;
        virtual lock_t write_lock(record_type type, const std::string& key) override;
        virtual txn_t start_txn() override;
        virtual bool write_record(record_type type, const std::string& key,
                                uint64_t object_version, const std::string& data) override;
        virtual history_list_t get_change_history() const override;

        virtual ~BerkeleyDBCXXDb() noexcept override;

        bool commit_record(change_t);
        ChangeList read_changelist() const;

        static std::string key_prefix(record_type type);
        static std::string key_name(record_type type, const std::string &name);
        static record_type get_type_from_keyname(const std::string &fullkey);
        static std::string unprefix(const std::string &fullkey);
    private:
        BerkeleyDBCXXDb(const std::string &name,
                boost::shared_ptr<BerkeleyDB> backend, 
                const boost::shared_ptr<db::ConfigIface> db_config,
                boost::shared_ptr<BerkeleyDBCXXEnv> env);

        thread_local static std::unordered_map<std::string, boost::shared_ptr<BerkeleyDBCXXDb>> multiton_map_;
        thread_local static bool thread_registered_;

        // because instances of this db RAII class are thread-local (because they are created in this thread
        // if not found in the thread_local multiton map), any txn or db instance is held by this
        // thread; so we don't need to coordinate these.
        boost::weak_ptr<BerkeleyDBCXXTxn> current_txn_;

        boost::shared_ptr<Db> inst_;
        std::string name_;
        boost::shared_ptr<BerkeleyDB> backend_;
        boost::shared_ptr<BerkeleyDBCXXEnv> env_;
        const boost::shared_ptr<db::ConfigIface> db_config_;
        range::Emitter log;
};

} /* namespace db */ } /* namespace range */

#endif
