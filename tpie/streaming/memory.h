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

#ifndef _TPIE_STREAMING_MEMORY_H
#define _TPIE_STREAMING_MEMORY_H

#include <vector>
#include <tpie/portability.h>

namespace tpie {
namespace streaming {

template <typename T>
struct memory_fits {
	static TPIE_OS_SIZE_T fits(TPIE_OS_SIZE_T mem) {
		TPIE_OS_SIZE_T high=1;
		while(T::memory(high) <= mem)
			high *= 2;
		TPIE_OS_SIZE_T low=high/2;

		TPIE_OS_SIZE_T best=low;
		while(low <= high) {
			TPIE_OS_SIZE_T c = (high+low)/2;
			
			TPIE_OS_SIZE_T m = T::memory(c);
			if(m <= mem) {
				best = c;
				low = c+1;
			} else
				high = c-1;
		}
		return best;
	}
};
	
		
class memory_base {
private:
	double memory_priority;
	TPIE_OS_SIZE_T mem;
public:
	void setMemoryPriority(double priority) {
		memory_priority=priority;
	}
	TPIE_OS_SIZE_T memory() {return mem;}
	void setMemory(double f) {
		mem = (double)MM_manager.consecutive_memory_available() * f;
	}
	
	void setMemory(TPIE_OS_SIZE_T m) {
		mem = m;
	}
	virtual void memoryNext(std::vector<memory_base *> &) {}
	virtual void memoryPrev(std::vector<memory_base *> &) {}
	virtual TPIE_OS_SIZE_T minimumMemory() {
		throw std::runtime_error("minimumMemory() not impremented");
	}
	void registerDataStructure(TPIE_OS_SIZE_T id, double priority) {}
	virtual void isSplit();
};

class memory_split: public memory_base {
private:
public:
	TPIE_OS_SIZE_T memoryIn();
	TPIE_OS_SIZE_T memoryOut();
};
		


class priority_memory_manager_private;

class priority_memory_manager {
private:
	priority_memory_manager_private * p;
public:
	priority_memory_manager();
	~priority_memory_manager();
	void add(memory_base * object);
	void allocate();
	void allocate(double f);
	void allocate(TPIE_OS_SIZE_T mem);
};
}
}

#endif //_TPIE_STREAMING_MEMORY_H
