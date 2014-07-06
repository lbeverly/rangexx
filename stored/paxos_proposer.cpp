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

#include <thread>

#include <rangexx/core/stored_message.h>

#include "paxos.h"
#include "signalhandler.h"

namespace range { namespace stored { namespace paxos {

const std::string Proposer::request_queue_ { "ProposerRequestQueue" };

//##############################################################################
//##############################################################################
Proposer::Proposer(boost::shared_ptr<::range::StoreDaemonConfig> cfg)
    : QueueWorkerThread("Proposer"), cfg_{cfg} 
{
}

//##############################################################################
//##############################################################################
void
Proposer::event_loop_init()
{
}

//##############################################################################
//##############################################################################
void
Proposer::event_task()
{
    BOOST_LOG_FUNCTION();

    this->q_wait();

    stored::Request req;
    while(this->q_pop(req)) {
        LOG(debug1, "proposal_received") << req.method() << ": " << req.client_id();
        ::range::stored::Ack ack;
        ack.set_status(true);
        ack.set_client_id(req.client_id());
        ack.set_request_id(req.request_id());
        ::range::stored::RequestQueueListener reqql { request_queue_, cfg_ };
        reqql.send_ack(req.client_id(), ack);

        try {
            if (cfg_->node_id() == distinguished_proposer()) {
                LOG(debug0, "handling_received_proposal") << req.method() << ": " << req.client_id();
                uint8_t bow_out = 1;
                while (bow_out < 128) {
                    while(! prepare(req) && bow_out++ < 128) {
                        uint32_t ms = (bow_out * (bow_out + 1)) / 2;        // Triangle number exponential backoff + semi-random thread-jitter
                        std::chrono::milliseconds delay { ms };
                        std::this_thread::sleep_for(delay);
                    }
                    if(propose(req)) {
                        break;
                    }
                }
            }
        } catch (range::Exception &e) {
            LOG(error, "range_exception") << e.what();
        }
    } 
}

//##############################################################################
//##############################################################################
std::string
Proposer::distinguished_proposer()
{
    ::range::RangeAPI_v1 range { cfg_ };
    ::range::RangeStruct range_proposers;
    std::string cluster_name { cfg_->range_cell_name() + '.' + "proposers" };
    try {
        range_proposers = range.simple_expand_cluster("_local_", cluster_name);
    } catch (::range::graph::NodeNotFoundException) {
        LOG(fatal, "_local_#" + cluster_name + "cluster not found");
        SignalHandler::terminate();
    }

    std::string distinguished_proposer;
    auto vals = boost::get<::range::RangeArray>(range_proposers).values;

    if (! vals.empty()) {
        distinguished_proposer = boost::get<::range::RangeString>(vals.front()).value;
    }
    LOG(debug2, "distinguished_proposer") << distinguished_proposer;

    return distinguished_proposer;
}

//##############################################################################
//##############################################################################
bool
Proposer::prepare(stored::Request req) {
    LOG(debug1, "preparing_request") << req.method();
    return true;
}

//##############################################################################
//##############################################################################
bool
Proposer::propose(stored::Request req) {
    LOG(notice, "proposing_request") << req.method();
    return true;
}
    
} /* namespace paxos */ } /* namespace stored */ } /* namespace range */