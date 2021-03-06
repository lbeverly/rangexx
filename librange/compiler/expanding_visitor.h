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

#ifndef _RANGE_COMPILER_EXPANDING_VISITOR_H
#define _RANGE_COMPILER_EXPANDING_VISITOR_H

#include <boost/variant/static_visitor.hpp>

#include "../graph/graph_interface.h"
#include "../core/log.h"

#include "compiler_types.h"
#include "ast.h"

namespace range { namespace compiler {

static EmitterModuleRegistration RangeExpandingVisitorModule { "compiler.RangeExpandingVisitorModule" };
//##############################################################################
//##############################################################################
class RangeExpandingVisitor : public boost::static_visitor<>
{
    public:
        //######################################################################
        //######################################################################
        RangeExpandingVisitor(boost::shared_ptr<graph::GraphInterface> graph, 
                std::string env_name = "", std::string prefix = "") 
            : graph_(graph), prefix_(prefix), env_name_(env_name), 
            log(RangeExpandingVisitorModule)
        {
        }
        
        //######################################################################
        //######################################################################
        void operator()(ast::ASTWord&) const;
        void operator()(ast::ASTLiteral&) const;
        void operator()(ast::ASTRegex&) const;
        void operator()(ast::ASTNull&) const;
        void operator()(ast::ASTUnion&) const;
        void operator()(ast::ASTDifference&) const;
        void operator()(ast::ASTIntersection&) const;
        void operator()(ast::ASTSequence&) const;
        void operator()(ast::ASTExpand&) const;
        void operator()(ast::ASTGetCluster&) const;
        void operator()(ast::ASTAdmin&) const;
        void operator()(ast::ASTGroup&) const;
        void operator()(ast::ASTBraceExpand&) const;
        void operator()(ast::ASTFunctionArguments&) const;
        void operator()(ast::ASTFunction&) const;
        void operator()(ast::ASTKeyExpand&) const;

    private:
        boost::shared_ptr<graph::GraphInterface> graph_;
        std::string prefix_;
        std::string env_name_;
        range::Emitter log;

        void prefix_child(std::string& child) const;
        
};

static EmitterModuleRegistration FetchChildrenVisitorModule { "compiler.FetchChildrenVisitor" };
//##############################################################################
//##############################################################################
class FetchChildrenVisitor : public boost::static_visitor<std::vector<std::string>>
{
    public:
        FetchChildrenVisitor() : log(FetchChildrenVisitorModule) {}
        //######################################################################
        //######################################################################
        template <typename ChildType>
        std::vector<std::string> operator()(ChildType& n) const
        {
            RANGE_LOG_FUNCTION();
            std::vector<std::string> tmp = n.children;
            n.children.clear();                                                 // Cleanup after ourselves as we go
            n.children.shrink_to_fit();
            return tmp;
        }

        //######################################################################
        // Null doesn't have children
        //######################################################################
        std::vector<std::string> operator()(ast::ASTNull&) const
        { 
            RANGE_LOG_FUNCTION();
            return std::vector<std::string>();
        }

        //######################################################################
        // FunctionArguments should never be evaluated for children; just in case
        //######################################################################
        std::vector<std::string> operator()(ast::ASTFunctionArguments&) const
        {
            RANGE_LOG_FUNCTION();
            return std::vector<std::string>();
        }

    private:
        range::Emitter log;
        //boost::shared_ptr<graph::GraphInterface> graph_;
        
};




} /* namespace compiler */ } /* namespace range */

#endif

