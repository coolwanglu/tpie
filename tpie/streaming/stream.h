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

#ifndef _TPIE_STREAMING_STREAM_H
#define _TPIE_STREAMING_STREAM_H
#include <tpie/stream.h>
#include <tpie/streaming/memory.h>
#include <tpie/streaming/util.h>

namespace tpie {
namespace streaming {

template <class stream_t, class dest_t> 
class stream_source : public common_single<stream_source<stream_t, dest_t>, dest_t> {
private:
	typedef common_single<stream_source<stream_t, dest_t>, dest_t> parent_t;
	using parent_t::dest;
	stream_t * stream;
public:
	stream_source(stream_t * s, dest_t & dest):
		parent_t(dest, 0.0f), stream(s) {}

	inline void run() {
		typename stream_t::item_type * item;
		dest().begin(stream->stream_len());
		while(stream->read_item(&item) != ami::END_OF_STREAM) 
			dest().push(*item);
		dest().end();
	}
};

		
template <class stream_t>
class pull_stream_source: public memory_single {
private:
	stream_t * stream;
public:
	typedef typename stream_t::item_type item_type;
	
	pull_stream_source(stream_t * s): stream(s) {};
	
	inline bool atEnd() {
		return stream->tell() == stream->stream_len();
	}
	
	inline const item_type & pull() {
		item_type * item;
		stream->read_item(&item);
		return *item;
	}
	
	static TPIE_OS_SIZE_T memory(TPIE_OS_SIZE_T cnt=1) {
		return sizeof(pull_stream_source)*cnt + stream_t::memory_usage(cnt);
	}

	virtual TPIE_OS_SIZE_T memoryBase() {return sizeof(*this);}
	
	void free() {}
};


template <class stream_t> 
class stream_sink: public memory_single {
private:
	stream_t * stream;
public:
	typedef typename stream_t::item_type item_type;
	
	inline stream_sink(stream_t * s): stream(s) {}
	inline void begin(TPIE_OS_OFFSET =0) {}
	inline void push(const item_type & item) {
		stream->write_item(item);
	}
	inline void end() {}
	
	static TPIE_OS_SIZE_T memory(TPIE_OS_SIZE_T cnt=1) {
		return sizeof(stream_sink)*cnt + stream_t::memory_usage(cnt);
	}
	virtual TPIE_OS_SIZE_T memoryBase() {return sizeof(*this);}
};

}
}
#endif //_TPIE_STREAMING_STREAM_H
