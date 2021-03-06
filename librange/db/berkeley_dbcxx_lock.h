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
#ifndef _RANGE_DB_BERKELEY_DBCXX_LOCK_H
#define _RANGE_DB_BERKELEY_DBCXX_LOCK_H

#include <db_cxx.h>

#include "../core/log.h"

#include "db_interface.h"

namespace range { namespace db {

//##############################################################################
//##############################################################################
class BerkeleyDBCXXLock : public GraphInstanceLock {
    public:
        BerkeleyDBCXXLock(DbEnv * env, bool readwrite=false);
        virtual ~BerkeleyDBCXXLock() noexcept override;
        virtual void unlock() override;
        virtual bool readonly() override { return !readwrite_; };
        void promote() { readwrite_ = true; }
    private:
        friend class BerkeleyDBCXXLockTxnGetter;
        DbEnv * env_;
        bool readwrite_;
        DbTxn * txn_;
        bool initialized_;
        range::Emitter log;
};

//##############################################################################
//##############################################################################
class BerkeleyDBCXXLockTxnGetter {
    friend class BerkeleyDBCXXDb;
    friend class BerkeleyDBCXXCursor;
    friend class BerkeleyDBCXXTxLogCursor;
    friend class BerkeleyDBCXXTxLogDb;
    
    explicit BerkeleyDBCXXLockTxnGetter(boost::shared_ptr<BerkeleyDBCXXLock> lock) 
        : lck_(lock)
    {
    }

    DbTxn * txn() { return lck_->txn_; }
    boost::shared_ptr<BerkeleyDBCXXLock> lck_;
};


} /* namespace db */ } /* namespace range */

#endif
