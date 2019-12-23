#include <inc/lib.h>

// malloc()
//	This function use NEXT FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

struct AllocationInfo
{
	uint32 BlockStart,BlockSize,StartIndex;
};


uint32 NumberOfPages = (USER_HEAP_MAX - USER_HEAP_START +1 ) /PAGE_SIZE;
bool UsedPages[(USER_HEAP_MAX - USER_HEAP_START +1 ) /PAGE_SIZE];
struct AllocationInfo AllAllocations[(USER_HEAP_MAX - USER_HEAP_START +1 ) / PAGE_SIZE];

int AddBlock(uint32 BlockStart,uint32 StartIndex,uint32 BlockSize)
{
	for(uint32 i = 0; i< NumberOfPages; i++)
	{
		if(AllAllocations[i].BlockStart == 0)
		{
			AllAllocations[i].BlockStart = BlockStart;
			AllAllocations[i].BlockSize = BlockSize;
			AllAllocations[i].StartIndex = StartIndex;
			while(BlockSize--)
			{
				UsedPages[StartIndex] = 1;
				StartIndex = (StartIndex + 1) % NumberOfPages;
			}
			return i;
		}
	}
	return -1;
}

int FindBlockIndex(uint32 BlockStart)
{
	for(uint32 i = 0; i< NumberOfPages; i++)
	{
		if(AllAllocations[i].BlockStart == BlockStart)
		{
			return i;
		}
	}
	return -1;
}



uint32 NextFitStrategy(uint32 PagesToAllocate)
{
	uint32 AllocationStart = 0;
	uint32 StartIndex = 0;
	uint32 CurrentBlockSize = 0;
	static uint32 CurrentAddress = USER_HEAP_START;
	static uint32 i = 0;
	uint32 SeenPages = 0;
	bool Counting = 0;

	while(SeenPages < NumberOfPages)
	{
		if(UsedPages[i] == 0)
		{
			if(!Counting)
			{
				StartIndex = i;
				AllocationStart = CurrentAddress;
				Counting = 1;
			}

			CurrentBlockSize++;
			if(CurrentBlockSize >= PagesToAllocate)
			{
				AddBlock(AllocationStart,StartIndex,PagesToAllocate);
				CurrentAddress+= PAGE_SIZE;
				if(CurrentAddress == USER_HEAP_MAX)
				{
					CurrentAddress = USER_HEAP_START;
				}
				i = (i + 1) % NumberOfPages;
				return AllocationStart;
			}
		}
		else
		{
			CurrentBlockSize = 0;
			Counting = 0;
		}

		CurrentAddress+= PAGE_SIZE;
		if(CurrentAddress == USER_HEAP_MAX)
		{
			CurrentAddress = USER_HEAP_START;
		}
		i = (i + 1) % NumberOfPages;
		SeenPages++;
	}

	return 0;

}


void* malloc(uint32 size)
{
	//TODO: [PROJECT 2019 - MS2 - [4] User Heap] malloc() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");

	// Steps:
	//	1) Implement NEXT FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_allocateMem to invoke the Kernel for allocation
	// 	4) Return pointer containing the virtual address of allocated space,
	//

	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyNEXTFIT()
	//to check the current strategy
	size = ROUNDUP(size,PAGE_SIZE);
	size /= PAGE_SIZE;
	uint32 AllocationStart;
	if(sys_isUHeapPlacementStrategyNEXTFIT())
	{
		AllocationStart = NextFitStrategy(size);
		if(AllocationStart != 0)
		{
			//cprintf("Allocated %d Pages \n",size);
			sys_allocateMem(AllocationStart,size*PAGE_SIZE);
		}
		return (void*)AllocationStart;
	}

	return 0;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address)
{
	//TODO: [PROJECT 2019 - MS2 - [4] User Heap] free() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");

	//you shold get the size of the given allocation using its address
	//you need to call sys_freeMem()
	//refer to the project presentation and documentation for details

	uint32 BlockStart = (uint32)virtual_address;
	int BlockIndex = FindBlockIndex(BlockStart);
	if(BlockIndex == -1)
	{
		return;
	}
	sys_freeMem((uint32)virtual_address,AllAllocations[BlockIndex].BlockSize*PAGE_SIZE);
	uint32 AllocationSize = AllAllocations[BlockIndex].BlockSize;
	uint32 StartIndex = AllAllocations[BlockIndex].StartIndex;

	while(AllocationSize--)
	{
		UsedPages[StartIndex] = 0;
		StartIndex = (StartIndex + 1) % NumberOfPages;
	}
	AllAllocations[BlockIndex].BlockStart = 0;
	AllAllocations[BlockIndex].BlockSize = 0;

}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2019 - BONUS3] User Heap Realloc [User Side]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");

	return NULL;
}



//==================================================================================//
//============================= OTHER FUNCTIONS ====================================//
//==================================================================================//

void expand(uint32 newSize)
{
}

void shrink(uint32 newSize)
{
}

void freeHeap(void* virtual_address)
{
	return;
}


//=============
// [1] sfree():
//=============
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//[] Free Shared Variable [User Side]
	// Write your code here, remove the panic and write your code
	//panic("sfree() is not implemented yet...!!");

	//	1) you should find the ID of the shared variable at the given address
	//	2) you need to call sys_freeSharedObject()

}

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//[[6] Shared Variables: Creation] smalloc() [User Side]
	// Write your code here, remove the panic and write your code
	panic("smalloc() is not implemented yet...!!");

	// Steps:
	//	1) Implement FIRST FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_createSharedObject(...) to invoke the Kernel for allocation of shared variable
	//		sys_createSharedObject(): if succeed, it returns the ID of the created variable. Else, it returns -ve
	//	4) If the Kernel successfully creates the shared variable, return its virtual address
	//	   Else, return NULL

	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyFIRSTFIT() to check the current strategy

	return 0;
}

void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//[[6] Shared Variables: Get] sget() [User Side]
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");

	// Steps:
	//	1) Get the size of the shared variable (use sys_getSizeOfSharedObject())
	//	2) If not exists, return NULL
	//	3) Implement FIRST FIT strategy to search the heap for suitable space
	//		to share the variable (should be on 4 KB BOUNDARY)
	//	4) if no suitable space found, return NULL
	//	 Else,
	//	5) Call sys_getSharedObject(...) to invoke the Kernel for sharing this variable
	//		sys_getSharedObject(): if succeed, it returns the ID of the shared variable. Else, it returns -ve
	//	6) If the Kernel successfully share the variable, return its virtual address
	//	   Else, return NULL
	//

	//This function should find the space for sharing the variable
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyFIRSTFIT() to check the current strategy

	return NULL;
}

