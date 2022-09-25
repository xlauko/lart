/*
 * (c) 2021 Henrich Lauko <xlauko@mail.muni.cz>
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

#include <cc/operation.hpp>
#include <cc/shadow.hpp>

namespace lart
{

    struct lifter
    {
        using operation = lart::op::operation;

        explicit lifter( sc::module_ref m, operation o, const shadow_map &s )
            : module( m ), op( o ), shadows( s )
        {
            if ( function()->empty() )
                generate();
        }

        std::string name() const;
        llvm::Function* function() const;

    private:
        void generate() const;

        mutable llvm::Function *_function = nullptr;
        sc::module_ref module;

    public:
        operation op;
        const shadow_map &shadows;
    };

} // namespace lart
