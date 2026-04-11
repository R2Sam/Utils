#pragma once

template <bool copy = false, bool move = false>
struct NonCopyable
{
	NonCopyable() = default;

	NonCopyable(const NonCopyable& other)
		requires(!copy)
	= delete;
	NonCopyable& operator=(const NonCopyable& other)
		requires(!copy)
	= delete;

	NonCopyable(NonCopyable&& other)
		requires(!move)
	= delete;
	NonCopyable& operator=(NonCopyable&& other)
		requires(!move)
	= delete;

	NonCopyable(const NonCopyable& other)
		requires(copy)
	= default;
	NonCopyable& operator=(const NonCopyable& other)
		requires(copy)
	= default;

	NonCopyable(NonCopyable&& other)
		requires(move)
	= default;
	NonCopyable& operator=(NonCopyable&& other)
		requires(move)
	= default;

	~NonCopyable() = default;
};