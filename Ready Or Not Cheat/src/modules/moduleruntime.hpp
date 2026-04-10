#pragma once

#include <atomic>

namespace moduleruntime
{
	inline std::atomic<bool> bWorkersActive{ true };

	inline void RequestStop() noexcept
	{
		bWorkersActive.store(false, std::memory_order_release);
	}

	inline bool WorkersActive() noexcept
	{
		return bWorkersActive.load(std::memory_order_acquire);
	}
}
