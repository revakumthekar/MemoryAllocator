#include "mem.h"
extern BLOCK_HEADER* first_header;

int Is_Free(BLOCK_HEADER* curr)
{

    if((curr->size_alloc & 1) == 0)
    {
        return 1; // if it is 0, that means it's free so we are returning "true"
    }else{
        return 0; // if it is 1, that means it's allocated so we are returning "false"
    }
}

int Get_Size(BLOCK_HEADER* curr)
{
    if(Is_Free(curr))
    {
        return curr->size_alloc;
    }else{
        int size = curr->size_alloc-1;
        return size;
    }
    
    //return (curr->size_alloc & 0xFFFFFFFE);
}

void Set_Allocated(BLOCK_HEADER* curr)
{
    curr->size_alloc = (curr->size_alloc | 1);
}

int Get_Padding(BLOCK_HEADER* curr, int pl)
{
    return Get_Size(curr) - 8 - pl;
}

BLOCK_HEADER* Get_Next_Header(BLOCK_HEADER* curr)
{
    return (BLOCK_HEADER*)((unsigned long) curr + Get_Size(curr));
}
 
void* Get_User_Pointer(BLOCK_HEADER* curr)
{
    return (void*)((unsigned long) curr + 8);
} 



// return a pointer to the payload
// if a large enough free block isn't available, return NULL
void* Mem_Alloc(int size){
    // find a free block that's big enough

    BLOCK_HEADER *current = first_header;
    while(current->size_alloc != 1)
    {

        if(Is_Free(current) && Get_Size(current) >= (size + 8) && Get_Size(current) >= 16)
        {
            //allocate it
            current->payload = size;
            if(Get_Padding(current, size) >= 16)
            {
                // do some math to figure out where the new header could go 
                // also do the split
                int original_block_size = Get_Size(current);
                int new_block_size = (((size+7)/16)+1)*16;
                current->size_alloc = new_block_size;
                BLOCK_HEADER *new_free_block = Get_Next_Header(current);
                new_free_block->size_alloc = original_block_size-new_block_size;
                new_free_block->payload = Get_Size(new_free_block) - 8;

            }
            Set_Allocated(current);
            return Get_User_Pointer(current);
        }

        current = Get_Next_Header(current);

    }


    return NULL; 
}


// return 0 on success
// return -1 if the input ptr was invalid
int Mem_Free(void *ptr){
    
    
    BLOCK_HEADER *current = first_header;
    while(1)
    {
        
        if(Get_User_Pointer(current) == ptr)
        {
            break;
        }
        if(current->size_alloc == 1)
        {
            return -1;
        }
        
        current = Get_Next_Header(current);
    }

    //freeing it
    if(!Is_Free(current))
    {
        current->size_alloc = (current->size_alloc - 1); //freeing it
        current->payload = (current->payload + Get_Padding(current,current->payload)); //adding padding back to payload size
    }

    //coalescing
    current = first_header;
    BLOCK_HEADER *next = Get_Next_Header(current);
    
    while(1)
    {
        if(next->size_alloc == 1)
        {
            break;
        }

        if(Is_Free(current) && Is_Free(next))
        {

            current->size_alloc = Get_Size(next) + Get_Size(current);
            current->payload = Get_Size(current) - 8;
            next = Get_Next_Header(next);

        }else{
            current = next;
            next = Get_Next_Header(current);
        }
        
    }
    
    return 0;
}
