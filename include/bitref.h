#pragma once

#include "types.h"

template <typename T>
class BitRef {
public:
	BitRef(T& ref, int i) : m_ref{ref}, m_i{i} {}
	BitRef(T&& ref, int i) : m_ref{ref}, m_i{i} {}
	BitRef(const BitRef&) = default;
	BitRef& operator=(const BitRef&) = default;
	BitRef(BitRef&&) = delete;
	BitRef& operator=(BitRef&&) = delete;

	operator bool() const { return ((m_ref >> m_i) & 1) != 0; };
	void operator=(bool rhs) {
		m_ref = static_cast<u8>((m_ref & (~(1 << m_i))) | (rhs << m_i));
	}

private:
	T& m_ref;
	int m_i;
};
