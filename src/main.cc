/* Copyright (c) 2026 VillageSQL Contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "address_standardizer.h"
#include "villagesql/vsql/func_types.h"
#include <villagesql/vsql.h>

#include <cstring>

using namespace vsql_addr_std;
using namespace vsql;

address parse_address(std::string_view address)
{
  delivery_line dl;
  last_line ll;

  auto tokens = tokenise(address);
  auto delivery_line_end = parse_last_line(tokens, &ll);
  parse_delivery_line(tokens, delivery_line_end, &dl);

  return {dl, ll};
}

void address_standardize_impl(StringArg address, StringResult out)
{
  if (address.is_null())
  {
    out.set_null();
    return;
  }

  auto addr = parse_address(address.value()).to_string();
  auto needed = addr.size();

  if (needed > out.buffer().size())
  {
    out.set_length(needed);
    return;
  }

  out.set_length(needed);
  out.set(addr);
}

void address_parse_json_impl(StringArg address, StringResult out)
{
  if (address.is_null())
  {
    out.set_null();
    return;
  }

  auto addr = parse_address(address.value()).to_json();
  auto needed = addr.size();

  if (needed > out.buffer().size())
  {
    out.set_length(needed);
    return;
  }

  out.set(addr);
}

VEF_GENERATE_ENTRY_POINTS(make_extension()
                              .func(make_func<&address_standardize_impl>("address_standardize")
                                        .returns(STRING)
                                        .param(STRING)
                                        .deterministic()
                                        .build())
                              .func(make_func<&address_parse_json_impl>("address_parse_json")
                                        .returns(STRING)
                                        .param(STRING)
                                        .deterministic()
                                        .build()))
