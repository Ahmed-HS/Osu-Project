#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)

struct AllocationInfo
{
	uint32 BlockStart,BlockSize;;
};

const uint32 MaxNumberOfAllocations = (KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE;
struct AllocationInfo AllAllocations[(KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE];

int AddBlock(uint32 BlockStart,uint32 BlockSize)
{
	for(uint32 i = 0; i< MaxNumberOfAllocations; i++)
	{
		if(AllAllocations[i].BlockStart == 0)
		{
			AllAllocations[i].BlockStart = BlockStart;
			AllAllocations[i].BlockSize = BlockSize;
			return i;
		}
	}
	return -1;
}

int FindBlockIndex(uint32 BlockStart)
{
	for(uint32 i = 0; i< MaxNumberOfAllocations; i++)
	{
		if(AllAllocations[i].BlockStart == BlockStart)
		{
			return i;
		}
	}
	return -1;
}

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kmalloc()
	// Write your code here, remove the panic and write your code

	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	//NOTE: Allocation is based on Best FIT strategy
	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details

	size = ROUNDUP(size,PAGE_SIZE);
	uint32 PagesToAllocate = size / PAGE_SIZE;
	uint32 BestBlockStart = 0,BestBlockSize = 0xffffffff;
	uint32 CurrentBlockSize = 0;
	uint32 CurrentAddress = KERNEL_HEAP_START;

	while(CurrentAddress < KERNEL_HEAP_MAX)
	{
		uint32 *PageTable;
		struct Frame_Info *CurrentFrame = get_frame_info(ptr_page_directory,(void*)CurrentAddress,&PageTable);
		if(CurrentFrame == NULL)
		{
			CurrentBlockSize++;
		}
		else
		{
			if(CurrentBlockSize >= PagesToAllocate && CurrentBlockSize < BestBlockSize)
			{
				BestBlockStart = CurrentAddress - CurrentBlockSize*PAGE_SIZE;
				BestBlockSize  = CurrentBlockSize;
			}
			CurrentBlockSize = 0;
		}

		CurrentAddress+= PAGE_SIZE;
	}

	if(BestBlockStart == 0 && CurrentBlockSize >= PagesToAllocate)
	{
		BestBlockStart = CurrentAddress - CurrentBlockSize*PAGE_SIZE;
		BestBlockSize  = CurrentBlockSize;
	}

	if(BestBlockStart == 0)
	{
		return NULL;
	}

	for(uint32 i = 0; i< PagesToAllocate; i++)
	{
		struct Frame_Info *FreeFrame;
		allocate_frame(&FreeFrame);
		map_frame(ptr_page_directory,FreeFrame,(void*)(BestBlockStart + (i*PAGE_SIZE)),PERM_WRITEABLE | PERM_PRESENT);
		FreeFrame->va = BestBlockStart + (i*PAGE_SIZE);
	}

	AddBlock(BestBlockStart,PagesToAllocate);

	//TODO: [PROJECT 2019 - BONUS1] Implement the first FIT strategy for Kernel allocation
	// Beside the Best FIT
	// use "isKHeapPlacementStrategyBESTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer
	return (void*)BestBlockStart;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kfree()
	// Write your code here, remove the panic and write your code

	uint32 BlockStart = (uint32)virtual_address;
	int BlockIndex = FindBlockIndex(BlockStart);
	if(BlockIndex == -1)
	{
		return;
	}
	uint32 BlockSize = AllAllocations[BlockIndex].BlockSize;

	for(uint32 i = 0; i< BlockSize; i++)
	{
		uint32 *PageTable;
		uint32 CurrentAddress = BlockStart + (i*PAGE_SIZE);
		struct Frame_Info *CurrentFrame = get_frame_info(ptr_page_directory,(void*)CurrentAddress,&PageTable);
		free_frame(CurrentFrame);
		PageTable[PTX(CurrentAddress)] = 0;
	}
	AllAllocations[BlockIndex].BlockStart = 0;
	AllAllocations[BlockIndex].BlockSize = 0;
	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kheap_virtual_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer
	struct Frame_Info *CurrentFrameInfo = to_frame_info(physical_address);
	if(CurrentFrameInfo != NULL)
	{
		return CurrentFrameInfo->va;
	}
	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kheap_physical_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	uint32 *PageTable;
	get_page_table(ptr_page_directory,(void*)virtual_address,&PageTable);
	if(PageTable != NULL && (PageTable[PTX(virtual_address)] & PERM_PRESENT))
	{
		return PageTable[PTX(virtual_address)] & 0xfffff000;
	}

	return 0;
}


//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2019 - BONUS2] Kernel Heap Realloc
	// Write your code here, remove the panic and write your code

	return NULL;
	panic("krealloc() is not implemented yet...!!");

}
