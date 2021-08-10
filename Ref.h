#ifndef REF
#define REF

#include "MemoryManager.h"

namespace ME
{
	/**
	 Ref is Reference smart pointer built over Allocator systems of MarsEngine. This class aloow the user to have 
	 multiple reference of a pointer.
	 FIX: Need to make a better implementation for UpstreamMemory initializer.
	**/
	template<typename T, typename upstreammemory = alloc_dealloc_UpstreamMemory> class ControlBlock
	{
	public:
		ControlBlock()
			:Count(0) {}

		void inc() { Count += 1; }
		void dec() { Count -= 1; }

		size_t Count;
		char Obj[sizeof(T)];
	};
	template<typename T, typename upstreammemory = alloc_dealloc_UpstreamMemory> class Ref
	{
	public:
		ControlBlock<T, upstreammemory>* m_ControlBlock;

		Ref()
			:m_ControlBlock(nullptr) {}

		Ref(nullptr_t)
			:m_ControlBlock(nullptr) {}

		template<typename U> Ref(const Ref<U, upstreammemory>& other)
		{
			m_ControlBlock = reinterpret_cast<ControlBlock<T, upstreammemory>*>(other.m_ControlBlock);
			m_ControlBlock->inc();
		}

		template<typename U> Ref(Ref<U, upstreammemory>&& other)
		{
			m_ControlBlock = reinterpret_cast<ControlBlock<T, upstreammemory>*>(other.m_ControlBlock);
			m_ControlBlock->inc();
		}

		~Ref()
		{
			if (m_ControlBlock != nullptr)
			{
				m_ControlBlock->dec();
				if (m_ControlBlock->Count == 0)
					upstreammemory::stref->deallocate(m_ControlBlock, sizeof(ControlBlock<T, upstreammemory>), "REF: Deallocating Control Block");
			}
		}

		template<typename U> Ref& operator=(const Ref<U, upstreammemory>& other)
		{
			Ref ref;
			m_ControlBlock = reinterpret_cast<ControlBlock<T, upstreammemory>*>(other.m_ControlBlock);
			ref.m_ControlBlock->inc();

			return ref;
		}

		template<typename U> Ref& operator=(Ref<U, upstreammemory>&& other)
		{
			Ref ref;
			m_ControlBlock = reinterpret_cast<ControlBlock<T, upstreammemory>*>(other.m_ControlBlock);
			other.m_ControlBlock = nullptr;

			return ref;
		}

		void reset()
		{
			upstreammemory::stref->deallocate(m_ControlBlock, sizeof(ControlBlock<T, upstreammemory>), "REF: Deallocating Control Block");
			m_ControlBlock = nullptr;
		}

		T& operator*() { return *reinterpret_cast<T*>(m_ControlBlock->Obj); }
		T* operator->() { return reinterpret_cast<T*>(m_ControlBlock->Obj); }
		T const& operator*() const { return *reinterpret_cast<T*>(m_ControlBlock->Obj); }
		T const* operator->() const { return reinterpret_cast<T*>(m_ControlBlock->Obj); }
		template<typename U> bool operator==(const Ref<U, upstreammemory>& other) { return other.m_ControlBlock == m_ControlBlock; }
		template<typename U> bool operator!=(const Ref<U, upstreammemory>& other) { return other.m_ControlBlock != m_ControlBlock; }
	private:
		friend Ref;
		template<typename T, typename ...Args, typename upstreammemory> friend auto CreateRef(Args&& ...args);
	};
	template<typename T, typename ...Args, typename upstreammemory = alloc_dealloc_UpstreamMemory> auto CreateRef(Args&& ...args) 
	{
		Ref<T, upstreammemory> ref;
		ref.m_ControlBlock = (ControlBlock<T, upstreammemory>*)(upstreammemory::stref->allocate(sizeof(ControlBlock<T, upstreammemory>), "REF: Allocating Control Block"));
		ref.m_ControlBlock->Count = 1;
		new (ref.m_ControlBlock->Obj) T(args...);

		return ref;
	}
}
#endif // !REF