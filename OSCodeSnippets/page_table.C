/* 

    Date  : Sep 15 2012
    Added code for Page Fault exception handler 

    Date  : Oct 1 2012
    Added code for moving the Page Table into Process Memory Pool 

    Date  : Aug 6 2013
    Added DEBUG Flag for Debugging Purpose
    Added code comments and Description
*/

/* Author: Shyam S Ramachandran
   sr[DOT]shyam[AT]gmail[DOT]com

   Description:
   This is a stand-alone file to realize a paging implementation
   by following the tutorial at http://wiki.osdev.org/Paging 

*  */


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
#define MB * (0x1 << 20)
#define KB * (0x1 << 10)

//#define DEBUG

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

//#include "assert.H"
#include "utils.H"
#include "console.H"
#include "frame_pool.H"
#include "page_table.H"
#include "paging_low.H"

/*--------------------------------------------------------------------------*/
/* DECLARATION OF STATIC VARIABLES */
/*--------------------------------------------------------------------------*/

PageTable*     	PageTable::current_page_table; 
unsigned int    PageTable::paging_enabled;     
FramePool*     	PageTable::kernel_mem_pool;    
FramePool*	PageTable::process_mem_pool;   
VMPool*		PageTable::vmpool_list[20];   
unsigned long   PageTable::shared_size;


/*--------------------------------------------------------------------------*/
/* PageTable Constructor */
/*--------------------------------------------------------------------------*/
PageTable::PageTable() {
	/* page directory related data */
	unsigned long page_dir_frame;
	page_dir_frame = kernel_mem_pool->get_frame();
	unsigned long page_directory_temp = page_dir_frame * 4 KB;
	memcpy(&page_directory, &page_directory_temp, sizeof(unsigned long));

	/* page table related data*/
	unsigned long* page_table;
	unsigned long page_table_frame;
        page_table_frame = process_mem_pool->get_frame();  /* GET FRAME FOR PAGE TABLE FROM PROCESS POOL : MP2 */
        unsigned long page_table_temp = page_table_frame * 4 KB;
        memcpy(&page_table, &page_table_temp, sizeof(unsigned long));
	
	unsigned long address=0; // holds the physical address of where a page is
	unsigned int i;

	// map the first 4MB of memory :: from osdever tutorial
	for(i=0; i<1024; i++)
	{
		page_table[i] = address | 3; // attribute set to: supervisor level, read/write, present(011 in binary)
		address = address + 4096; // 4096 = 4kb
	};

	// fill the first entry of the page directory
	page_directory[0] = page_table_temp; // attribute set to: supervisor level, read/write, present(011 in binary)
	page_directory[0] = page_directory[0] | 3;
	
	for(i=1; i<1024; i++)
	{
		// attribute set to: supervisor level, read/write, not present(010 in binary)
		page_directory[i] = 0 | 2; 
	}
	// fille the last entry of PD back to itself - trick !!
	page_directory[1023] = page_directory_temp | 3;
	
	for(int i=0;i<20;i++) {
		vmpool_list[i] = 0;
	}
	
}


void PageTable::init_paging(FramePool * _kernel_mem_pool,
                          FramePool * _process_mem_pool,
                          const unsigned long _shared_size) {
	/* only initialize data-structures */
	kernel_mem_pool = _kernel_mem_pool;
	process_mem_pool = _process_mem_pool;
	shared_size = _shared_size;
	paging_enabled = 0;

	return;
}

void PageTable::load() {

	current_page_table = this;
	write_cr3((unsigned long)page_directory); // put that page directory address into CR3
	return;
}

void PageTable::enable_paging() {

	unsigned long paging_bit = 0x80000000;
	write_cr0(read_cr0() | paging_bit); // set the paging bit in CR0 to 1
	paging_enabled = 1;
	return;
}


/*--------------------------------------------------------------------------*/
/* PageFault Handler */
/* Written on 9/15/12 -  Getting Memory
   Modified on 10/1/12 -  move page table to process memory pool to expand space */
/*--------------------------------------------------------------------------*/
void PageTable::handle_fault(REGS *_r) {
	unsigned long addr = read_cr2();
	#ifdef DEBUG
 	Console::puts("Address : ");
	Console::putui(addr);	
	Console::puts("\n");
	#endif

	/* MP2 newly added trick for recursive lookup */	
	unsigned long page_dir_index;
	page_dir_index = addr >> 22;
	unsigned long page_table_index;
	page_table_index = addr<<10;
	page_table_index = page_table_index >> 22;
	page_table_index = (addr >> 12) & 0x3FF;
	unsigned long* page_table;
	
	if((current_page_table->page_directory[page_dir_index] & 0x00000001) == 1) { /* page directory is valid, but an entry in page table is invalid */

		//Set up Page Table address as table_trick_addr
	//	if(*directory_trick_addr & 0x00000001 == 1) { /* page directory is valid, but an entry in page table is invalid */
		unsigned long* directory_trick_addr = (unsigned long*) (0xFFFFF000 & page_dir_index);
		unsigned long trick_temp = 0xFFFFFFFF;
		trick_temp = trick_temp <<22;
		unsigned long page_dir_trick = page_dir_index << 12;
		unsigned long* table_trick_addr = (unsigned long*) (trick_temp | page_dir_trick | (page_table_index*4));

//	        unsigned long page_table_frame =  (*directory_trick_addr >> 12);
	        unsigned long page_table_frame = current_page_table->page_directory[page_dir_index] >> 12;

	        unsigned long process_frame;
        	process_frame = process_mem_pool->get_frame();

	        unsigned long address = (unsigned long) process_frame * 4 KB; // holds the physical address of where a page is
        	unsigned int i;

		*table_trick_addr = address | 3;

		#ifdef DEBUG
 		Console::puts("Inside PD found PT not found PDI: ");
		Console::putui(page_dir_index);	
 		Console::puts("PTI: ");
		Console::putui(page_table_index);	
		Console::puts("\n");
		#endif

	} else { /* PD is invalid and page table is also  invalid */

		// the page table trick address is 'table_trick_addr_actual'
		unsigned long* directory_trick_addr = (unsigned long*) (0xFFFFF000 & page_dir_index);
		unsigned long trick_temp = 0xFFFFFFFF;
		trick_temp = trick_temp <<22;
		unsigned long page_dir_trick = page_dir_index << 12;
		unsigned long* table_trick_addr = (unsigned long*) (trick_temp | page_dir_trick );
		unsigned long* table_trick_addr_actual;

        	unsigned long page_table_frame;
	        page_table_frame = process_mem_pool->get_frame();
        	unsigned long page_table_temp = page_table_frame * 4 KB;

	        memcpy(&page_table, &page_table_temp, sizeof(unsigned long));

		unsigned long process_frame;
		process_frame = process_mem_pool->get_frame();

	        unsigned long address = (unsigned long) (process_frame * 4 KB); // holds the physical address of where a page is
        	unsigned int i;
        

		// fill the entry of the page directory before filling page table because we need this
		//*directory_trick_addr = page_table_temp; // attribute set to: supervisor level, read/write, present(011 in binary)
		//*directory_trick_addr = *directory_trick_addr | 3;
        	current_page_table->page_directory[page_dir_index] = page_table_temp; // attribute set to: supervisor level, read/write, present(011 in binary)
	        current_page_table->page_directory[page_dir_index] = current_page_table->page_directory[page_dir_index] | 3;


        	for(i=0; i<1024; i++)
        	{
			table_trick_addr_actual = (unsigned long*) (((unsigned long)table_trick_addr) | (i*4));
                	(*table_trick_addr_actual) |= 2; // attribute set to: supervisor level, read/write, present(011 in binary)
			if(i==page_table_index) {
			(*table_trick_addr_actual) = address | 3;
		}
        }
		#ifdef DEBUG
 		Console::puts("Inside PD not found PT not found PDI: ");
		Console::putui(page_dir_index);	
 		Console::puts("PTI: ");
		Console::putui(page_table_index);	
 		Console::puts("table_trick_addr : ");
		Console::putui((unsigned long)table_trick_addr);	
		Console::puts("table_trick_addr_actual : ");
                Console::putui((unsigned long)table_trick_addr_actual);
		Console::puts("\n");
		#endif

	}

	return;
}

void free_page(unsigned long _page_no) {
  	/* Release the frame associated with the page _page_no */
	unsigned long page_dir_index = 	_page_no / 1024;
	unsigned long page_table_index = _page_no % 1024;

	unsigned long* directory_trick_addr = (unsigned long*) (0xFFFFF000 & page_dir_index);
        unsigned long trick_temp = 0xFFFFFFFF;
        trick_temp = trick_temp <<22;
        unsigned long page_dir_trick = page_dir_index << 12;
        unsigned long* table_trick_addr = (unsigned long*) (trick_temp | page_dir_trick | (page_table_index*4));
		
	unsigned long frame = (*table_trick_addr ^ 3) / (4 KB);
	//process_mem_pool->release_frame(frame);
	
	*table_trick_addr |= 2; // invalidate entry
    	return;
}

void register_vmpool(VMPool *_pool) {
	
	int i;
	#ifdef VM_POOL
	while(PageTable::vmpool_list[i] !=0  && (i <20)){
		i++;
	}
	PageTable::vmpool_list[i] = _pool;
	#endif
    return;
}


