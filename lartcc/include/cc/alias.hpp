/*
 * (c) 2020 Henrich Lauko <xlauko@mail.muni.cz>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

#include <cc/logger.hpp>

#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include <svf/Graphs/SVFG.h>
#include <svf/WPA/Andersen.h>

#include <sc/generator.hpp>

#include <queue>

namespace lart::aa
{
    struct andersen
    {
        void init( SVF::SVFIR * pag )
        {
            pta = SVF::AndersenWaveDiff::createAndersenWaveDiff( pag );
            callgraph = pta->getPTACallGraph();

            value_flow_graph = std::make_unique< SVF::VFG >( callgraph );

            SVF::SVFGBuilder value_flow_builder(true);
            value_flow = std::unique_ptr< SVF::SVFG >( value_flow_builder.buildFullSVFG(pta) );
        }

        inline auto node( const llvm::Value *value )
        {
            return pta->getPAG()->getValueNode(value);
        }

        inline auto pta_node( const llvm::Value *value)
        {
            return pta->getPAG()->getGNode( node(value) );
        }

        inline auto value_flow_node( const llvm::Value *value )
        {
            return value_flow->getDefSVFGNode( pta_node(value) );
        }

        inline sc::generator< llvm::Value * > pointsto( llvm::Value *value )
        {
            auto to_value = [&] (auto node) {
                return const_cast< llvm::Value * >( node->getValue() );
            };
            for (auto pts : pta->getPts( node( value ) ) ) {
                auto target = pta->getPAG()->getGNode(pts);
                if ( target->hasValue() ) {
                    co_yield to_value( target );
                }
            }
        }

        using value_flow_node_t = SVF::VFGNode;

        inline sc::generator< llvm::Value * > uses( llvm::Value *value )
        {
            auto to_value = [&] (auto node) {
                return const_cast< llvm::Value * >(value_flow->getLHSTopLevPtr(node)->getValue());
            };

            std::queue< const value_flow_node_t* > worklist;
            std::unordered_set< const value_flow_node_t* > seen;

            worklist.push( value_flow_node( value ) );

            while (!worklist.empty()) {
                auto current = worklist.front();
                for (auto out : current->getOutEdges()) {
                    auto succ = out->getDstNode();
                    if (!seen.count(succ)) {
                        seen.insert(succ);
                        co_yield to_value(succ);
                        worklist.push(succ);
                    }
                }

                worklist.pop();
            }
        }

        SVF::PTACallGraph * callgraph = nullptr;
        SVF::AndersenWaveDiff * pta = nullptr;

        std::unique_ptr< SVF::VFG > value_flow_graph;
        std::unique_ptr< SVF::SVFG > value_flow;
    };

} // namespace lart::aa
