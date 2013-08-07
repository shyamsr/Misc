/* 

 Date  : Sep 15 2012
 Author: Shyam S Ramachandran
 Email : sr[DOT]shyam[AT]gmail[DOT]com
 */


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

//#include "assert.H"
#include "utils.H"
#include "console.H"
#include "frame_pool.H"

unsigned char FramePool::permission[MAX_NUM_FRAMES]={INVALID};

FramePool::FramePool(unsigned long _base_frame_no,
             unsigned long _nframes,
             unsigned long _info_frame_no) {
        base_frame_no = _base_frame_no;
        nframes = _nframes;
        info_frame_no = _info_frame_no;
	robin_head = base_frame_no;
	unsigned long i;
	for(i=0;i<MAX_NUM_FRAMES;i++) {
		if(i < nframes) {
			FramePool::permission[base_frame_no + i] = UNASSIGNED; 
		}
	}
		
}

unsigned long FramePool::get_frame() {

	/* Frames are allocated in round robin way,
	   this ensures that no frame is allocated frequently

	*/
	unsigned long temp_frame_no = 0;
	unsigned long curr_robin_head = robin_head;
	while(permission[robin_head] != UNASSIGNED) {
		robin_head++;
		if(robin_head >= base_frame_no + nframes) {
		/* we ran out of frames, reset robin_head and search from beginning */
			robin_head = base_frame_no;
		} else if (robin_head == curr_robin_head) {
		/* we are out of frames PANIC or Swap in/out */


		}
	}
		
	temp_frame_no = robin_head;
	permission[robin_head] = ASSIGNED;
	robin_head++;
	if(robin_head >= base_frame_no + nframes) {
	/* Handle bounds */
		robin_head = base_frame_no;
	}
	
	/* Do not return this if frame is marked INACCESSIBLE, we already checked this */
	return temp_frame_no;
}

void FramePool::mark_inaccessible(unsigned long _base_frame_no,
                          unsigned long _nframes) {
	unsigned long i;
	
	for(i=0;i < _nframes ;i++) {
		FramePool::permission[_base_frame_no + i] = INACCESSIBLE;

	}
	return;
}


void FramePool::release_frame(unsigned long _frame_no) {
	/* Handle frame validity */
	if(permission[_frame_no] != ASSIGNED ) {
	/* trying to release a frame which is not Assigned : this is bad, panic? */

	}
	FramePool::permission[_frame_no] = UNASSIGNED;

	return;
}
