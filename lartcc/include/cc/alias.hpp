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

#include <svf/Graphs/PAG.h>
#include <svf/WPA/Andersen.h>

#include <sc/generator.hpp>

namespace lart::aa
{
    struct andersen
    {
        void init( SVF::PAG * pag )
        {
            pta = SVF::AndersenWaveDiff::createAndersenWaveDiff( pag );
        }

        inline auto node( const llvm::Value *value )
        {
            return pta->getPAG()->getValueNode(value);
        }

        inline sc::generator< llvm::Value * > pointsto( llvm::Value *value )
        {
            for (auto pts : pta->getPts( node( value ) ) ) {
                auto target = pta->getPAG()->getPAGNode(pts);
                if ( target->hasValue() )
                    co_yield const_cast< llvm::Value * >( target->getValue() );
            }
        }

        SVF::AndersenWaveDiff * pta = nullptr;
    };

} // namespace lart::aa
