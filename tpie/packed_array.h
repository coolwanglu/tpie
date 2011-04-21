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
#ifndef __TPIE_PACKED_ARRAY_H__
#define __TPIE_PACKED_ARRAY_H__

#include <tpie/config.h>
#include <boost/static_assert.hpp>

///////////////////////////////////////////////////////////////////////////
/// \file bitarray.h
/// Contains a packed array with known memory requirements
///////////////////////////////////////////////////////////////////////////

namespace tpie {

/////////////////////////////////////////////////////////
/// \internal
/// \brief Base class for the iterators
/// \tparam CT CRTP child class
/// \tparam forward Is this a forward iterator?
/////////////////////////////////////////////////////////
template <typename CT, bool forward, typename RT>
class packed_array_iter_facade {
private:
	CT & self() {return *reinterpret_cast<CT*>(this);}
	template <typename, bool, typename> friend class packed_array_iter_facade;

public:
	const CT & self() const {return *reinterpret_cast<const CT*>(this);}

	typedef ptrdiff_t difference_type;
	typedef std::random_access_iterator_tag iterator_category;
	template <typename TT>
	inline bool operator==(const TT & o) const {return self().index() == o.self().index();}
	template <typename TT>
	inline bool operator!=(const TT & o) const {return self().index() != o.self().index();}
	inline CT & operator++() {self().index() += forward?1:-1; return self();}
	inline CT operator++(int) {CT x=self(); ++self(); return x;}
	inline CT & operator--() {self().index() += forward?-1:1; return self();}
	inline CT operator--(int) {CT x=self(); --self(); return x;}
	inline bool operator<(const CT & o) const {return self().index() < o.self().index();}
	inline bool operator>(const CT & o) const {return self().index() > o.self().index();}
	inline bool operator<=(const CT & o) const {return self().index() <= o.self().index();}
	inline bool operator>=(const CT & o) const {return self().index() >= o.self().index();}
	inline ptrdiff_t operator-(const CT & o) const {return self().index() < o.self().index();}
	inline CT operator+(difference_type n) const {CT x=self(); return x += n;}
	inline CT operator-(difference_type n) const {CT x=self(); return x -= n;}
	inline CT & operator+=(difference_type n) {self().index() += (forward?n:-n); return self();}
	inline CT & operator-=(difference_type n) {self().index() += (forward?n:-n); return self();}
	inline RT operator[](difference_type n) {return *(self() + n);}
};

template <typename CT, bool f, typename RT>
CT operator+(ptrdiff_t n, const packed_array_iter_facade<CT, f, RT> & i) {
	CT tmp(i.self());
	tmp += n;
	return tmp;
}

///////////////////////////////////////////////////////////////////////////
/// \brief An array storring elements of type T using B
/// bits to  to store a element.
///
/// T must be either bool or int. B must devide the word size,
/// in reality only 1, 2 or 4 seems usamle
///
/// \tparam T The type of elements to store in the array
/// \tparam B The number of bits used to store a single element
///////////////////////////////////////////////////////////////////////////
template <typename T, int B>
class packed_array: public linear_memory_base<packed_array<T, B> >{
public:
	typedef size_t storage_type;
private:
	BOOST_STATIC_ASSERT(sizeof(storage_type) * 8 % B == 0);
	static inline size_t perword() {return sizeof(storage_type) * 8 / B;}
	static inline size_t high(size_t index) {return index/perword();}
	static inline size_t low(size_t index) {return index%perword();}
	static inline size_t words(size_t m){return (perword()-1+m)/perword();}
	static inline storage_type mask() {return (1 << B)-1;}

	template <bool forward>
	class const_iter_base;

	template <bool forward>
	class iter_base;

	/////////////////////////////////////////////////////////
	/// \internal
	/// \brief Iterator and reverse iterator will return an element
	/// of this type, it can be cast to a T, and assigned a T value
	/////////////////////////////////////////////////////////
	class iter_return_type {
	private:
		iter_return_type(storage_type * e, size_t i): elms(e), index(i) {}
		storage_type * elms;
		size_t index;
	public:
		template <bool> friend class packed_array::iter_base;
		template <bool> friend class packed_array::const_iter_base;
		operator T() const {return static_cast<T>((elms[high(index)] >> low(index))&mask());}
	 	inline iter_return_type & operator=(const T b) {
			storage_type * p = elms+high(index);
			size_t i = low(index);
			*p = (*p & ~(mask()<<i)) | ((b & mask()) << i);
	 		return *this;
		}
	};

	/////////////////////////////////////////////////////////
	/// \internal
	/// \brief Base class for iterator and reverse iterator
	/// \tparam forward Is this a forward iterator?
	/////////////////////////////////////////////////////////
	template <bool forward>
	class iter_base: public packed_array_iter_facade<iter_base<forward>, forward, iter_return_type> {
	private:
		iter_return_type elm;		
		
		template <bool> friend class packed_array::const_iter_base;
		friend class packed_array;
		template <typename, bool, typename> friend class packed_array_iter_facade;
		iter_base(storage_type * elms, size_t index): elm(elms, index) {};

		inline size_t & index() {return elm.index;}
		inline const size_t & index() const {return elm.index;}
	public:
		typedef iter_return_type value_type;
		typedef iter_return_type & reference;
		typedef iter_return_type * pointer;
		
		iter_return_type & operator*() {return elm;}
		iter_return_type * operator->() {return &elm;}
		iter_base & operator=(const iter_base & o) {elm.index = o.elm.index; elm.elms=o.elm.elms; return *this;}
		iter_base(iter_base const &o): elm(o.elm) {};
	};
	
	/////////////////////////////////////////////////////////
	/// \internal
	/// \brief Base class for const ierator and const reverse iterator
	/// \tparam forward Is this a forward iterator?
	/////////////////////////////////////////////////////////
	template <bool forward>
	class const_iter_base: public packed_array_iter_facade<const_iter_base<forward>, forward, T> {
	private:
		const storage_type * elms;
		size_t idx;
		
		friend class packed_array;
		friend class boost::iterator_core_access;
		template <typename, bool, typename> friend class packed_array_iter_facade;
		const_iter_base(const storage_type * e, size_t i): elms(e), idx(i) {}

		inline size_t & index() {return idx;}
		inline const size_t & index() const {return idx;}
	public:
		typedef T value_type;
		typedef T reference;
		typedef T * pointer;

		const_iter_base & operator=(const const_iter_base & o) {idx = o.idx; elms=o.elms; return *this;}
		T operator*() const {return static_cast<T>(elms[high(idx)] >> low(idx) & mask());}
		const_iter_base(const_iter_base const& o): elms(o.elms), idx(o.idx) {}
		const_iter_base(iter_base<forward> const& o): elms(o.elm.elms), idx(o.elm.index) {}
	};		


	/////////////////////////////////////////////////////////
	/// \internal
	/// \brief This type is returned by the [] operator.
	/// It can be cast to a T, and assigned a T value.
	/////////////////////////////////////////////////////////
	struct return_type{
	private:
		storage_type * p;
	 	size_t i;
		return_type(storage_type * p_, size_t i_): p(p_), i(i_) {}
		friend class packed_array;
	public:
	 	inline operator T() const {return static_cast<T>((*p >> i) & mask());}
	 	inline return_type & operator=(const T b) {
			*p = (*p & ~(mask()<<i)) | ((static_cast<const storage_type>(b) & mask()) << i);
	 		return *this;
		}
	 	inline return_type & operator=(const return_type & t){
	 		*this = (T) t;
	 		return *this;
	 	}
	};

	storage_type * m_elements;
	size_t m_size;
public:		
	/////////////////////////////////////////////////////////
	/// \brief Type of values containd in the array
	/////////////////////////////////////////////////////////
	typedef T value_type;
	
	/////////////////////////////////////////////////////////
	/// \brief Iterator over a const array
	/////////////////////////////////////////////////////////
	typedef const_iter_base<true> const_iterator;

	/////////////////////////////////////////////////////////
	/// \brief Reverse iterator over a const array
	/////////////////////////////////////////////////////////
	typedef const_iter_base<false> const_reverse_iterator;

	/////////////////////////////////////////////////////////
	/// \brief Iterator over an array
	/////////////////////////////////////////////////////////
	typedef iter_base<true> iterator;

	/////////////////////////////////////////////////////////
	/// \brief Reverse iterator over an array
	/////////////////////////////////////////////////////////
	typedef iter_base<false> reverse_iterator;

	/////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_coefficient()
	/// \copydetails linear_memory_structure_doc::memory_coefficient()
	/////////////////////////////////////////////////////////
	inline static double memory_coefficient(){
		return B/8.0;
	}
	
	/////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_overhead()
	/// \copydetails linear_memory_structure_doc::memory_overhead()
	/////////////////////////////////////////////////////////
	static double memory_overhead() {
		return (double)sizeof(packed_array) + MM_manager.space_overhead()+(double)sizeof(storage_type);
	}	

	/////////////////////////////////////////////////////////
	/// \brief Construct array of given size.
	///
	/// The elements have undefined values
	/// \param s The number of elements in the array
	/////////////////////////////////////////////////////////
	packed_array(size_t s=0): m_elements(0), m_size(0) {resize(s);}

	/////////////////////////////////////////////////////////
	/// \brief Construct array of given size.
	///
	/// \param s The number of elements in the array
	/// \param value Each entry of the array is initialized with this value
	/////////////////////////////////////////////////////////
	packed_array(size_t s, T value): m_elements(0), m_size(0) {resize(s,value);}

	/////////////////////////////////////////////////////////
	/// \brief Construct a copy of another array
	/// \param other The array to copy
	/////////////////////////////////////////////////////////
	packed_array(const packed_array & a): m_elements(0), m_size(0) {*this=a;}

	/////////////////////////////////////////////////////////
	/// \brief Free up all memory used by the array
	/////////////////////////////////////////////////////////
	~packed_array() {resize(0);}

	/////////////////////////////////////////////////////////
	/// \brief Copy elements from another array into this.
	///
	/// Note this array is resized to the size of other
	/// \param other The array to copy from
	/// \return a reference to this array
	/////////////////////////////////////////////////////////
	packed_array & operator=(const packed_array & a) {
		resize(a.m_size);
		for(size_t i=0;i<words(m_size);++i)
			m_elements[i] = a.m_elements[i];
		return *this;
	}

	/////////////////////////////////////////////////////////
	/// \brief Change the size of the array
	///
	/// All elements are lost, after resize the value of the entries
	/// is undefined
	/// \param s the new size of the array
	/////////////////////////////////////////////////////////
	void resize(size_t s) {
		if (s == m_size) return;
		delete m_elements;
		m_size = s;
		m_elements = m_size?new storage_type[words(m_size)]:0;
	}

	/////////////////////////////////////////////////////////
	/// \brief Change the size of the array
	///
	/// All elements are lost, after resize the value of the entries
	/// is initialized by a copy of elm
	/// \param s the new size of the array
	/// \param elm the initialization element
	/////////////////////////////////////////////////////////
	void resize(size_t s, T value) {
		resize(s);
		storage_type x=0;
		for (size_t i=0; i < perword(); ++i) 
			x = (x << B) + ((storage_type)value & mask());
		for (size_t i=0; i < words(m_size); ++i)
			m_elements[i] = x;
	}

	/////////////////////////////////////////////////////////
	/// \brief Return the size of the array
	///
	/// \return the size of the array
	/////////////////////////////////////////////////////////
	inline size_t size() const {return m_size;}

	/////////////////////////////////////////////////////////
	/// \brief Check if the array is empty
	///
	/// \return true if and only if size is 0
	/////////////////////////////////////////////////////////
	inline bool empty() const {return m_size ==0;}

	/////////////////////////////////////////////////////////
	/// \brief Return an array entry
	///
	/// \param i the index of the entry to return
	/// \return the array entry
	/////////////////////////////////////////////////////////
	inline T operator[](size_t t)const {
		assert(t < m_size);
		return static_cast<T>((m_elements[high(t)] >> low(t))&mask());
	}	
	
	/////////////////////////////////////////////////////////
	/// \brief Return a object behaving like a reference to an array entry
	///
	/// \param i the index of the entry to return
	/// \return The object behaving like a reference
	/////////////////////////////////////////////////////////
	inline return_type operator[](size_t t) {
		assert(t < m_size);
		return return_type(m_elements+high(t), low(t));
	}

	/////////////////////////////////////////////////////////
	/// \brief Return an iterator to the i'th element of the array
	///
	/// \param i the index of the element we want an iterator to
	/// \return an iterator to the i'th element
	/////////////////////////////////////////////////////////
	inline iterator find(size_type i) {return iterator(m_elements,i);}

	/////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the i'th element of the array
	///
	/// \param i the index of the element we want an iterator to
	/// \return a const iterator to the i'th element
	/////////////////////////////////////////////////////////
	inline const_iterator find(size_type i) const {return const_iterator(m_elements,i);}

	/////////////////////////////////////////////////////////
	/// \brief Return an iterator to the beginning of the array
	///
	/// \return an iterator tho the beginning of the array
	/////////////////////////////////////////////////////////
	inline iterator begin() {return iterator(m_elements,0);}

	/////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the beginning of the array
	///
	/// \return a const iterator tho the beginning of the array
	/////////////////////////////////////////////////////////
	inline const_iterator begin() const {return const_iterator(m_elements,0);}

	/////////////////////////////////////////////////////////
	/// \brief Return an iterator to the end of the array
	///
	/// \return an iterator tho the end of the array
	/////////////////////////////////////////////////////////
	inline iterator end() {return iterator(m_elements,m_size);}

	/////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the end of the array
	///
	/// \return a const iterator tho the end of the array
	/////////////////////////////////////////////////////////
	inline const_iterator end() const {return const_iterator(m_elements,m_size);}

	//We use m_elements -1 as basic for the reverse operators
	//To make sure that the index of rend is positive
	inline reverse_iterator rbegin() {return reverse_iterator(m_elements-1, perword()+m_size-1);}
	inline const_reverse_iterator rbegin() const {return const_reverse_iterator(m_elements-1, perword()+m_size-1);}
	inline reverse_iterator rend() {return reverse_iterator(m_elements-1, perword()-1);}
	inline const_reverse_iterator rend() const {return const_reverse_iterator(m_elements-1, perword()-1);}
};

}
#endif //__TPIE_PACKED_ARRAY_H__
