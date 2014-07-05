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

#include "mqserv.h"

#include <rangexx/core/api.h>
#include <rangexx/core/stored_message.h>

#include "signalhandler.h"
#include "network.h"

namespace range { namespace stored {

//##############################################################################
//##############################################################################
MQServer::MQServer(boost::shared_ptr<::range::StoreDaemonConfig> cfg) 
            : WorkerThread("MQServer"), cfg_(cfg)
{
}

//##############################################################################
//##############################################################################
void
MQServer::event_loop_init()
{
    reqql_ = std::unique_ptr<::range::stored::RequestQueueListener>(
                new ::range::stored::RequestQueueListener(cfg_)
            );
}
//##############################################################################
//##############################################################################
void
MQServer::event_task()
{
    BOOST_LOG_FUNCTION();

    ::range::RangeAPI_v1 range { cfg_ };
    ::range::RangeStruct range_proposers;

    std::string cluster_name { cfg_->range_cell_name() + '.' + "proposers" };
    try { 
        range_proposers = range.simple_expand_cluster("_local_", cluster_name);
    } catch(::range::graph::NodeNotFoundException) {
        LOG(fatal, "_local_#" + cluster_name + " cluster not found");
        this->set_shutdown(true);
        this->set_running(false);
        SignalHandler::terminate();
        return;
    }


    std::vector<std::string> proposers; 
    for(RangeStruct v : boost::get<::range::RangeArray>(range_proposers).values) {
        proposers.push_back(boost::get<::range::RangeString>(v).value);
    }

    do {
        ::range::stored::Request req;
        try {
            if (reqql_->receive(req)) {
                LOG(debug2, "received_message_queue_request") << req.method() << ": " << req.client_id();
                range::stored::network::UDPMultiClient cl { proposers, cfg_->port() };
                auto res = cl.timed_send(req.SerializeAsString(), 500);
            }
        } catch (range::stored::MqueueException &e) {
            LOG(error, "invalid_message") << e.what();
        } catch (boost::system::system_error &e) {
            LOG(error, "boost_error") << e.what();
        }
    }
    while(reqql_->pending() > 0);
}

} /* namespace */ } /* namespace stored */

