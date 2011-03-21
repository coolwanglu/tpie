// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
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
#include <tpie/prime.h>

///////////////////////////////////////////////////////////////////////////
/// \file prime.cpp
/// \brief Contains computations related to prime numbers
///////////////////////////////////////////////////////////////////////////


namespace tpie {

is_prime_t::is_prime_t(): m(4294967295ul), mr(static_cast<size_t>(sqrt(double(m))+1)) {}

void is_prime_t::init() {
	if (m_pr.size() != 0) return;
	array<bool> sieve(mr >> 1, true);
	size_t pc=1;
	std::cout << sieve[13 >> 1] << std::endl;
	for (size_t i=3; i < mr; i+= 2) {
		if (!sieve[i>>1]) continue;
		++pc;
		size_t i2=i+i;
		for (size_t j=i2+i; j < mr; j += i2) {
			sieve[j>>1] = false;
		}
	}
	m_pr.resize(pc);
	size_t p=1;
	m_pr[0] = 2;
	for (size_t i=3; i < mr; i += 2) {
		if (sieve[i>>1]) m_pr[p++] = i;
	}
}

void is_prime_t::finish() {m_pr.resize(0);}

is_prime_t is_prime;
}
