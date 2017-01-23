#pragma once

template <typename Fun>
class DeferGuard {
public:
	DeferGuard(Fun&& f) : m_f{std::move(f)} {}
	~DeferGuard() {
		if (m_f) {
			m_f();
		}
	}

private:
	Fun m_f;
};

template <typename S>
DeferGuard<S> defer(S&& s) {
	return DeferGuard<S>{std::move(s)};
}
