// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

#include <stdio.h>

#include <tpie/streaming/virtual_real.h>
#include <tpie/streaming/vbuffsize.inc>

namespace tpie {
namespace streaming {

template <typename item_t, typename begin_data_t=empty_type, typename end_data_t=empty_type>
class virtual_source: public virtual virtual_source_real<item_t, TPIE_STREAMING_VBUFF_SIZE, begin_data_t, end_data_t> {};

template <typename dest_t>
class virtual_source_impl 
	: public virtual_source_impl_real<dest_t, TPIE_STREAMING_VBUFF_SIZE>,
	public virtual_source<typename dest_t::item_type>
{
private:
	typedef virtual_source_impl_real<dest_t, TPIE_STREAMING_VBUFF_SIZE> parent_t;
public:
	virtual_source_impl(dest_t & dest): parent_t(dest) {};
	virtual memory_size_type base_memory() {
		return sizeof(*this);
	}
 	virtual void begin(stream_size_type size=0, typename dest_t::begin_data_type *data=0) {
 		parent_t::begin(size, data);
 	}
 	virtual void push(const typename dest_t::item_type & item) {
 		parent_t::push(item);
 	}
 	virtual void end(typename dest_t::end_data_type *data=0) {
 		parent_t::end(data);
 	}
};

template <typename item_t, typename begin_data_t=empty_type, typename end_data_t=empty_type> 
class virtual_sink_impl: public virtual_sink_real_impl<item_t, TPIE_STREAMING_VBUFF_SIZE, begin_data_t, end_data_t> {
private:
	typedef virtual_sink_real_impl<item_t, TPIE_STREAMING_VBUFF_SIZE, begin_data_t, end_data_t> parent_t;

public:
	typedef virtual_source<item_t, begin_data_t, end_data_t> dest_t;
	virtual_sink_impl(dest_t * dest): parent_t(dest) {}

	virtual memory_size_type base_memory() {
		return sizeof(*this);
	}
};

}
}

#undef TPIE_STREAMING_VBUFF_SIZE
