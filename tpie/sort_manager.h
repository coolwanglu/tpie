// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

// This file contains the class sort_manager that actually performs sorting
// given an internal sort implementation and merge heap implementation
// The merge heap classes can be found in the file mergeheap.h, 
// and the internal sort classes can be found in the file  internal_sort.h
#ifndef _TPIE_AMI_SORT_MANAGER_H
#define _TPIE_AMI_SORT_MANAGER_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
#include <tpie/stream.h> 
#include <tpie/tempname.h>
#include <tpie/merge_sorted_runs.h>
#include <tpie/mergeheap.h>  //For templated heaps
#include <tpie/internal_sort.h> // Contains classes for sorting internal runs
                           // using different comparison types
#include <cmath> //for log, ceil, etc.
#include <string>

#include <tpie/progress_indicator_base.h>

namespace tpie {

    namespace ami {

#ifndef AMI_STREAM_IMP_SINGLE
#warning Including __FILE__ when AMI_STREAM_IMP_SINGLE undefined.
#endif

// A class of manager objects for merge sorting objects of type T.  We
// will actually use one of two subclasses of this class which use
// either a comparison object,  or the binary comparison operator <.

	template <class T, class I, class M>
	class sort_manager {
	    
	public:
	    sort_manager(I* isort, M* mheap);

	    ~sort_manager() {
		//  No code in this destructor.
	    };

	    //  Sort in stream to out stream an save in stream (uses 3x space)
	    err sort(stream<T>* in, stream<T>* out, 
		     progress_indicator_base* indicator = NULL);
	    //Sort in stream and overwrite unsorted input with sorted output
	    //(uses 2x space)
	    err sort(stream<T>* in, progress_indicator_base* indicator = NULL); 
	    
	private:
	    // *************
	    // * Functions *
	    // *************
	    
	    err start_sort();              // high level wrapper to full sort 
	    err compute_sort_params();     // compute nInputItems, mrgArity, nRuns
	    err partition_and_sort_runs(); // make initial sorted runs
	    err merge_to_output();         // loop over merge tree, create output stream
	    // Merge a single group mrgArity streams to an output stream
	    err single_merge(stream<T>**, arity_t,  stream<T>*, stream_offset_type = -1);
	    // helper function for creating filename
	    inline void make_name(
		const std::string& prepre, 
		const std::string& pre, memory_size_type id, std::string& dest);
	    
	    // **************
	    // * Attributes *
	    // **************
	    
	    I*              m_internalSorter;   // Method for sorting runs in memory
	    M*              m_mergeHeap;        // Merge heap implementation 
	    stream<T>*      inStream;   
	    stream<T>*      outStream;   
	    err             ae;                 // For catching error codes
	    stream_offset_type  nInputItems;        // Number of items in inStream;
	    memory_size_type  mmBytesAvail;       // Amount of spare memory we can use
	    memory_size_type  mmBytesPerStream;   // Memory consumed by each Stream obj
	    
	    progress_indicator_base* m_indicator; // pointer to progress indicator

	    stream_offset_type progCount; //counter for showing progress
	    
	    bool use2xSpace; //flag to indicate if we are doing a 2x sort
	    
	    // The maximum number of stream items of type T that we can
	    // sort in internal memory
	    memory_size_type nItemsPerRun;
	    
	    stream_offset_type nRuns; //The number of sorted runs left to merge
	    arity_t mrgArity; //Max runs we can merge at one time
	    
	    // The output stream to which we are currently writing runs
	    stream<T>* curOutputRunStream;
	    
	    // The mininum number of runs in each output stream
	    // some streams can have one additional run
	    stream_offset_type minRunsPerStream; 
	    // The number of extra runs or the number of streams that
	    // get one additional run.
	    arity_t nXtraRuns;
	    
	    // The last run can have fewer than nItemsPerRun;
	    memory_size_type nItemsInLastRun;
	    // How many items we will sort in a given run
	    memory_size_type nItemsInThisRun;
	    // For each output stream, how many runs it should get
	    stream_offset_type runsInStream;
	    
	    // A suffix to use in forming output file names. During the merge phase
	    // we keep two sets of files, the input files and the output files to
	    // which we are merging. The input file suffix is the opposite of the
	    // output file suffix. After merging one level, the output streams
	    // become the input for the next level.
	    std::string suffixName[2];

	    // A buffer for building the output file names
		std::string   newName;

	    //prefix of temp files created during sort
	    std::string working_disk;

	private:
	    sort_manager(const sort_manager<T,I,M>& other);
	    sort_manager<T,I,M>& operator=(const sort_manager<T,I,M>& other);
	};
	
	template <class T, class I, class M>
	sort_manager<T, I, M>::sort_manager(I* isort, M* mheap):
	    m_internalSorter(isort), 
	    m_mergeHeap(mheap),
	    inStream(NULL), 
	    outStream(NULL),
	    ae(NO_ERROR),
	    nInputItems(0),
	    mmBytesAvail(0),
	    mmBytesPerStream(0),
	    m_indicator(NULL),
	    progCount(0),
	    use2xSpace(false),
	    nItemsPerRun(0),
	    nRuns(0),
	    mrgArity(0),
	    curOutputRunStream(NULL),
	    minRunsPerStream(0),
	    nXtraRuns(0),
	    nItemsInLastRun(0),
	    nItemsInThisRun(0),
	    runsInStream(0) {
	    
	    suffixName[0]="_0_";
	    suffixName[1]="_1_";

	    // Prefix of temp files created during sort
		working_disk = std::string(tempname::tpie_name("sort"));
	};

	template<class T, class I, class M>
	err sort_manager<T,I,M>::sort(stream<T>* in, stream<T>* out,
				      progress_indicator_base* indicator){

	    //  This version saves the original input and uses 3x space
	    //  (input, current temp runs, output runs)
	    
	    m_indicator = indicator;
	    inStream=in;
	    outStream=out;
	    use2xSpace=false;

	    // Basic checks that input is ok
	    if (inStream==NULL || outStream==NULL) { 
		return NULL_POINTER;
	    }

	    if (!inStream || !outStream) {
		return OBJECT_INVALID; 
	    }

	    if (inStream->stream_len() < 2) {
		return SORT_ALREADY_SORTED; 
	    }
	    
	    // Else, there is something to sort, do it
	    return start_sort();
	}

	template<class T, class I, class M>
	err sort_manager<T,I,M>::sort(stream<T>* in, progress_indicator_base* indicator){
	    
	    //This version overwrites the original input and uses 2x space
	    //The input stream is truncated to length 0 after forming initial runs
	    //and only two levels of the merge tree are on disk at any one time.
	    m_indicator = indicator;
	    inStream=in;
	    outStream=in; //output destination is same as input
	    use2xSpace=true;

	    // Basic checks that input is ok
	    if (inStream==NULL) { 
		return NULL_POINTER;
	    }
	    
	    if (!inStream) { 
		return OBJECT_INVALID; 
	    }
	    
	    if (inStream->stream_len() < 2) { 
		return SORT_ALREADY_SORTED; 
	    }
	    
	    // Else, there is something to sort, do it
	    return start_sort();
	}
	
	template<class T, class I, class M>
	err sort_manager<T,I,M>::start_sort(){
	    
	    TP_LOG_DEBUG_ID ("sort_manager::sort START");
	    
	    // ********************************************************************
	    // * PHASE 1: See if we can sort the entire stream in internal memory *
	    // * without the need to use general merge sort                       *
	    // ********************************************************************
	    	    
	    // Figure out how much memory we've got to work with.
	    mmBytesAvail = MM_manager.consecutive_memory_available();
	    
	    // Space for internal buffers for the input and output stream may not
	    // have been allocated yet. Query the space usage and subtract.
	    if ((ae = inStream->main_memory_usage
		 (&mmBytesPerStream,mem::STREAM_USAGE_MAXIMUM))
		!= NO_ERROR) {
		
		TP_LOG_DEBUG_ID ("Error returned from main_memory_usage");
		
		return ae;
	    }

	    TP_LOG_DEBUG ("BTE says we use at most "<< static_cast<stream_offset_type>(mmBytesPerStream)
			  << "bytes per stream\n");
	    
	    // This is how much we can use for internal sort if
	    // we are not doing general merge sort
	    mmBytesAvail -= 2 * mmBytesPerStream;
	    
	    // Check if all input items can be sorted internally using less than
	    // mmBytesAvail
	    nInputItems = inStream->stream_len();
	    
	    if (m_indicator) {
		m_indicator->set_title("\nStarting TPIE Sort");
		m_indicator->set_percentage_range(0, nInputItems, 1000);
	    }
	    
	    inStream->seek (0);
	    
	    if (nInputItems < stream_offset_type(m_internalSorter->MaxItemCount(mmBytesAvail))){
		if (m_indicator) {
		    m_indicator->init("Sorting items internally");
		}

		// allocate the internal array items
		m_internalSorter->allocate(static_cast<memory_size_type>(nInputItems));
		
		// load the items into main memory, sort, and write to output.
		// m_internalSorter also checks if inStream/outStream are the same and
		// truncates/rewrites inStream if they are. This probably should not
		// be the job of m_internalSorter-> TODO: build a cleaner interface
		if ((ae = m_internalSorter->sort(inStream, outStream, static_cast<memory_size_type>(nInputItems)))
		    != NO_ERROR) {
		    
		    TP_LOG_FATAL_ID ("main_mem_operate failed");
		    
		    return ae;
		}

		// de-allocate the internal array of items
		m_internalSorter->deallocate();

		if (m_indicator) {
		    m_indicator->done();
		}

		return NO_ERROR;
	    }

	    // ******************************************************************
	    // * Input stream too large for main memory, use general merge sort *
	    // ******************************************************************
	    
	    // PHASE 2: compute nItemsPerRun, nItemsPerRun, nRuns
	    ae=compute_sort_params();
	    if (ae != NO_ERROR){ 
		return ae; 
	    }

	    // ********************************************************************
	    // * By this point we have checked that we have valid input, checked  *
	    // * that we indeed need an external memory sort, verified that we    *
	    // * have enough memory to partition and at least do a binary merge.  *
	    // * Also checked that we have enough file descriptors to  merge,     *
	    // * and calculated the mrgArity and nItemsPerRun given memory        *
	    // * constraints. We have also calculated nRuns for the initial       *
	    // * number of runs we will partition into. Let's sort!               *
	    // ********************************************************************
	    
	    // ********************************************************************
	    // * WARNING: Since we accounted for all known memory usage in PHASE 2*
	    // * be very wary of memory allocation via "new" or constructors from *
	    // * this point on and make sure it was accounted for in PHASE 2      *
	    // ********************************************************************
	    
	    // PHASE 3: partition and form sorted runs
	    TP_LOG_DEBUG_ID ("Beginning general merge sort.");
	    ae=partition_and_sort_runs();
	    if (ae != NO_ERROR){ 
		return ae; 
	    }

	    // PHASE 4: merge sorted runs to a single output stream
	    ae=merge_to_output();
	    if (ae != NO_ERROR){ 
		return ae; 
	    }
	    
	    // That's it
	    if (m_indicator) {
		m_indicator->done();
	    }
	    
	    return NO_ERROR;
	}

	template<class T, class I, class M>
	err sort_manager<T,I,M>::compute_sort_params(void){
	    // ********************************************************************
	    // * PHASE 2: Compute/check limits                                    *
	    // * Compute the maximum number of items we can sort in main memory   *
	    // * and the maximium number of sorted runs we can merge at one time  *
	    // * Before doing any sorting, check that we can fit at least one item*
	    // * in internal memory for sorting and that we can merge at least two*
	    // * runs at at time                                                  *
	    // *                                                                  *
	    // * Memory needed for the run formation phase:                       *
	    // * 2*mmBytesPerStream +                  {for input/output streams} *
	    // * nItemsPerRun*space_per_sort_item() +  {for each item sorted }    *
	    // * space_overhead_sort()                 {constant overhead in      *
	    // *                                        sort management object    *
	    // *                                        during sorting       }    *
	    // *                                                                  *
	    // * Memory needed for a D-way merge:                                 *
	    // *  Cost per merge stream:                                          *
	    // *   mmBytesPerStream+              {a open stream to read from}    *
	    // *   space_per_merge_item()+        {used in internal merge heap}   *
	    // *   sizeof(T*)+sizeof(off_t)       {arrays in single_merge()}      *
	    // *   sizeof(stream<T>*)         {array element that points to   *
	    // *                                    merge stream}                 *
	    // *  Fixed costs:                                                    *
	    // *    2*mmBytesPerStream+        {original input stream + output    *
	    // *                                 of current merge}                *
	    // *    space_overhead_merge()+    {fixed dynamic memory costs of     *
	    // *                                 merge heap}                      *
	    // *    3*space_overhead()         {overhead per "new" memory request *
	    // *                                for allocating array of streams   *
	    // *                                in merge_to_output and two arrays *
	    // *                                in single_merge}                  *
	    // *                                                                  *
	    // *  Total cost for D-way Merge:                                     *
	    // *    D*(Cost per merge stream)+(Fixed costs)                       *
	    // *                                                                  *
	    // *  Any additional memory requests that call "new" directly or      *
	    // *  indirectly should be documented and accounted for in this phase *
	    // ********************************************************************
	    
	    TP_LOG_DEBUG_ID ("Computing merge sort parameters.");
	    
	    memory_size_type mmBytesAvailSort; // Bytes available for sorting
	    
	    TP_LOG_DEBUG ("Each object of size " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(sizeof(T)) << " uses "
			  << static_cast<TPIE_OS_OUTPUT_SIZE_T>(m_internalSorter->space_per_item ()) << " bytes "
			  << "for sorting in memory\n");
	    
	    // Subtract off size of temp output stream
	    // The size of the input stream was already subtracted from
	    // mmBytesAvail
	    mmBytesAvailSort=mmBytesAvail - mmBytesPerStream;
	    
	    nItemsPerRun=m_internalSorter->MaxItemCount(mmBytesAvailSort);
	    
	    if(nItemsPerRun<1){

		TP_LOG_FATAL_ID ("Insufficient Memory for forming sorted runs");
		
		return INSUFFICIENT_MAIN_MEMORY;
	    }

	    // Now we know the max number of Items we can sort in a single
	    // internal memory run. Next, compute the number of runs we can
	    // merge together at one time
	    
	    memory_size_type mmBytesPerMergeItem = mmBytesPerStream +
		m_mergeHeap->space_per_item() + sizeof(T*) +
		sizeof(stream_offset_type)+sizeof(stream<T>*);
	    
	    // Fixed cost of mergheap impl. + MM_manager overhead of allocating
	    // an array of stream<T> ptrs (pending)
	    // cost of Input stream already accounted for in mmBytesAvail..
	    memory_size_type mmBytesFixedForMerge = m_mergeHeap->space_overhead() +
		mmBytesPerStream + 3*MM_manager.space_overhead();
	    
	    memory_size_type mmBytesAvailMerge = mmBytesAvail - mmBytesFixedForMerge;

	    // Need to support at least binary merge
	    if(mmBytesAvailMerge<2*mmBytesPerMergeItem){

		TP_LOG_FATAL_ID ("Merge arity < 2 -- Insufficient memory for a merge.");

		return INSUFFICIENT_MAIN_MEMORY;
	    }

	    // Cast down from stream_offset_type (type of mmBytesAvail).
	    // mmBytesPerMergeItem is at least 1KB, so we are OK unless we
	    // have more than 2 TerraBytes of memory, assuming 64 bit
	    // (or smaller) stream_offset_typeS. I look forward to the day
	    // this comment seems silly and wrong
	    mrgArity = static_cast<arity_t>(mmBytesAvail-mmBytesFixedForMerge) /
		mmBytesPerMergeItem;
	    TP_LOG_DEBUG("mem avail=" << static_cast<TPIE_OS_OUTPUT_SIZE_T>(mmBytesAvail-mmBytesFixedForMerge)
			 << " bytes per merge item=" <<  static_cast<TPIE_OS_OUTPUT_SIZE_T>(mmBytesPerMergeItem)
			 << " initial mrgArity=" << static_cast<TPIE_OS_OUTPUT_SIZE_T>(mrgArity) << "\n");
	    
	    // Make sure that the AMI is willing to provide us with the
	    // number of substreams we want.  It may not be able to due to
	    // operating system restrictions, such as on the number of regions
	    // that can be mmap()ed in, max number of file descriptors, etc.
	    int availableStreams = inStream->available_streams ();
	    
	    // Merging requires an available stream/file decriptor for
	    // each of the mrgArity input strems. We need one additional file descriptor
	    // for the output of the current merge, so binary merge requires
	    // three available streams.
	    if (availableStreams < 3) {
		
		TP_LOG_FATAL_ID ("Not enough stream descriptors available " <<
				 "to perform merge.");
		
		return INSUFFICIENT_AVAILABLE_STREAMS;
	    }
	    
	    // Can at least do binary merge. See if availableStreams limits
	    // maximum mrgArity
	    // Due to the previous test, we know that available_streams >= 3.
	    if (mrgArity > static_cast<arity_t>(availableStreams - 1)) {

		mrgArity = static_cast<arity_t>(availableStreams - 1);
		
		TP_LOG_WARNING_ID ("Reduced merge arity due to AMI restrictions.");
	    }
	    
	    // The number of memory-sized runs that the original input stream
	    // will be partitioned into.
	    nRuns = ((nInputItems + nItemsPerRun - 1) / nItemsPerRun);
	    
#ifdef TPIE_SORT_SMALL_MRGARITY
	    // KEEP OUT!!!
	    // This should not be done by the typical user and is only for
	    // testing/debugging purposes. ONLY define this flag and set a value
	    // if you know what you are doing.
	    TP_LOG_WARNING_ID("TPIE_SORT_SMALL_MRGARITY flag is set."
			      " Did you mean to do this?");
	    if(mrgArity > TPIE_SORT_SMALL_MRGARITY) {
		TP_LOG_WARNING_ID("Reducing merge arity due to compiler specified flag");
		mrgArity=TPIE_SORT_SMALL_MRGARITY;
	    }
#endif // TPIE_SORT_SMALL_MRGARITY

#ifdef TPIE_SORT_SMALL_RUNSIZE
	    // KEEP OUT!!!
	    // This should not be done by the typical user and is only for
	    // testing/debugging purposes ONLY define this flag and set a value
	    // if you know what you are doing.
	    TP_LOG_WARNING_ID("TPIE_SORT_SMALL_RUNSIZE flag is set."
			      " Did you mean to do this?");
	    if(nItemsPerRun > TPIE_SORT_SMALL_RUNSIZE) {
		TP_LOG_WARNING_ID("Reducing run size due to compiler specified flag");
		nItemsPerRun=TPIE_SORT_SMALL_RUNSIZE;
	    }

	    // need to adjust nRuns
	    nRuns = ((nInputItems + nItemsPerRun - 1) / nItemsPerRun);
#endif // TPIE_SORT_SMALL_RUNSIZE

	    //#define MINIMIZE_INITIAL_RUN_LENGTH
#ifdef MINIMIZE_INITIAL_RUN_LENGTH
	    // If compiled with the above flag, try to reduce the length of
	    // the initial sorted runs without increasing the merge tree height
	    // This could be a speed-up if it is faster to quicksort many small
	    // runs and merge them than it is to quicksort fewer long 
	    // runs and merge them.
	    TP_LOG_DEBUG_ID ("Minimizing initial run lengths without increasing" <<
			     " the height of the merge tree.");

	    // The tree height is the ceiling of the log base mrgArity of the
	    // number of original runs.
	    double tree_height = log((double)nRuns) / log((double)mrgArity);
	    tp_assert (tree_height > 0, "Negative or zero tree height!");
	    tree_height = ceil (tree_height);

	    // See how many runs we could possibly fit in the tree without
	    // increasing the height.
	    double maxOrigRuns = pow ((double) mrgArity, tree_height);
	    tp_assert (maxOrigRuns >= nRuns "Number of permitted runs was reduced!");

	    // How big will such runs be?
	    double new_nItemsPerRun = ceil (nInputItems/ maxOrigRuns);
	    tp_assert (new_nItemsPerRun <= nItemsPerRun, 
		       "Size of original runs increased!");

	    // Update the number of items per run and the number of original runs
	    nItemsPerRun = (memory_size_type) new_nItemsPerRun;

	    TP_LOG_DEBUG_ID ("With long internal memory runs, nRuns = "
			     << nRuns);

	    nRuns = (nInputItems + nItemsPerRun - 1) / nItemsPerRun;

	    TP_LOG_DEBUG_ID ("With shorter internal memory runs "
			     << "and the same merge tree height, nRuns = "
			     << nRuns );

	    tp_assert (maxOrigRuns >= nRuns,
		       "We increased the merge height when we weren't supposed to do so!");
#endif  // MINIMIZE_INITIAL_SUBSTREAM_LENGTH


	    // If we have just a few runs, we don't need the
	    // full mrgArity. This is the last change to mrgArity
	    // N.B. We need to "up"-cast mrgArity here!
	    if(static_cast<stream_offset_type>(mrgArity)>nRuns){
		// N.B. We know that nRuns is small, so 
		//      it is safr to downcast.
		mrgArity=static_cast<memory_size_type>(nRuns);
	    }

	    // We should always end up with at least two runs
	    // otherwise why are we doing it externally?
	    tp_assert (nRuns > 1, "Less than two runs to merge!");
	    // Check that numbers are consistent with input size
	    tp_assert (nRuns * nItemsPerRun - nInputItems < nItemsPerRun,
		       "Total expected output size is too large.");
	    tp_assert (nInputItems - (nRuns - 1) * nItemsPerRun <= nItemsPerRun,
		       "Total expected output size is too small.");

	    TP_LOG_DEBUG_ID ("Input stream has " << nInputItems << " items");
	    TP_LOG_DEBUG ("Max number of items per runs " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(nItemsPerRun) );
	    TP_LOG_DEBUG ("\nInitial number of runs " << nRuns );
	    TP_LOG_DEBUG ("\nMerge arity is " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(mrgArity) << "\n" );

	    return NO_ERROR;
	}

	template<class T, class I, class M>
	err sort_manager<T,I,M>::partition_and_sort_runs(void){
	    // ********************************************************************
	    // * PHASE 3: Partition                                               *
	    // * Partition the input stream into nRuns of at most nItemsPerRun    *
	    // * and sort them, and write them to temporay output files.          *
	    // * The last run may have fewer than nItemsPerRun. To keep the number*
	    // * of files down and to support sequential I/O, we distribute the   *
	    // * nRuns evenly across mrgArity files, thus each file on disk holds *
	    // * multiple sorted runs.                                            *
	    // ********************************************************************

	    // The mininum number of runs in each output stream
	    // some streams can have one additional run
	    minRunsPerStream = nRuns/mrgArity;
	    // The number of extra runs or the number of streams that
	    // get one additional run. This is less than mrgArity and
	    // it is OK to downcast to an arity_t.
	    nXtraRuns = static_cast<arity_t>(nRuns - minRunsPerStream*mrgArity);
	    tp_assert(nXtraRuns<mrgArity, "Too many extra runs");

	    // The last run can have fewer than nItemsPerRun;
	    // general case
	    nItemsInLastRun = static_cast<memory_size_type>(nInputItems % nItemsPerRun);
	    if(nItemsInLastRun==0){
		// Input size is an exact multiple of nItemsPerStream
		nItemsInLastRun=nItemsPerRun;
	    }

	    // Initialize memory for the internal memory runs
	    // accounted for in phase 2:  (nItemsPerRun*size_of_sort_item) +
	    // space_overhead_sort
	    m_internalSorter->allocate(nItemsPerRun);

	    TP_LOG_DEBUG_ID ("Partitioning and forming sorted runs.");

	    // nItemsPerRun except for last run.
	    nItemsInThisRun=nItemsPerRun;
	    
        // Rewind the input stream, we are about to begin
	    inStream->seek(0);

	    // ********************************************************************
	    // * Partition and make initial sorted runs                           *
	    // ********************************************************************
	    stream_offset_type check_size = 0; //for debugging


	    if (m_indicator) {
		m_indicator->set_description("Forming runs      ");
		m_indicator->set_range(0,mrgArity,1);
		m_indicator->refresh();
	    }

	    for(arity_t ii=0; ii<mrgArity; ii++){   //For each output stream
		// Make the output file name
		make_name(working_disk, suffixName[0], ii, newName);
		// Dynamically allocate the stream
		// We account for these mmBytesPerStream in phase 2 (output stream)
		curOutputRunStream = new stream<T>(newName);
		// How many runs should this stream get?
		// extra runs go in the LAST nXtraRuns streams so that
		// the one short run is always in the LAST output stream
		runsInStream = minRunsPerStream + ((ii >= mrgArity-nXtraRuns)?1:0);

		for(stream_offset_type  jj=0; jj < runsInStream; jj++ ) { // For each run in this stream
		    // See if this is the last run
		    if( (ii==mrgArity-1) && (jj==runsInStream-1)) {
			nItemsInThisRun=nItemsInLastRun;
		    }
		    // Sort it
		    if ((ae = m_internalSorter->sort(inStream, curOutputRunStream, 
						     nItemsInThisRun))!= NO_ERROR)
		    {
			TP_LOG_FATAL_ID ("main_mem_operate failed");
			return ae;
		    }
		} // For each run in this stream

		// All runs created for this stream, clean up
		TP_LOG_DEBUG_ID ("Wrote " << runsInStream << " runs and "
				 << curOutputRunStream->stream_len() << " items to file " 
				 << static_cast<TPIE_OS_OUTPUT_SIZE_T>(ii));
		check_size+=curOutputRunStream->stream_len();
		curOutputRunStream->persist(PERSIST_PERSISTENT);
		delete curOutputRunStream;

		if (m_indicator) {
		    m_indicator->step();
		}

	    }//For each output stream

	    tp_assert(check_size == nInputItems, "item count mismatch");

	    // Done with partitioning and initial run formation
	    // free space associated with internal memory sorting
	    m_internalSorter->deallocate();
	    if (m_indicator) {
		m_indicator->done();
	    }
	    if(use2xSpace){ 
		//recall outStream/inStream point to same file in this case
		inStream->truncate(0); //free up disk space
		inStream->seek(0);
	    } 
	    return NO_ERROR;
	}

	template<class T, class I, class M>
	err sort_manager<T,I,M>::merge_to_output(void){

	    // ********************************************************************
	    // * PHASE 4: Merge                                                   *
	    // * Loop over all levels of the merge tree, reading mrgArity runs    *
	    // * at a time from the streams at the current level and distributing *
	    // * merged runs over mrgArity output streams one level up, until     *
	    // * a single output stream exists                                    *
	    // ********************************************************************

	    // The input streams we from which will read sorted runs
	    // This Memory allocation accounted for in phase 2:
	    //   mrgArity*sizeof(stream<T>*) + space_overhead()[fixed cost]
	    stream<T> **mergeInputStreams = new stream<T>*[mrgArity];

	    TP_LOG_DEBUG_ID("Allocated " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(sizeof(stream<T>*)*mrgArity)
			    << " bytes for " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(mrgArity) << " merge input stream pointers.\n"
			    << "Allocated " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(MM_manager.space_overhead()) << " bytes" 
			    << " of overhead on \"new\" call.\n" 
			    << "Mem. avail. is " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(MM_manager.consecutive_memory_available ()) );

	    // the number of iterations the main loop has gone through,
	    // at most the height of the merge tree log_{M/B}(N/B),
	    // typically 1 or 2
	    int mrgHeight  = 0;
	    int treeHeight = 0; //for progress
	    memory_size_type ii; //index vars
	    stream_offset_type jj; //index vars

	    // This Memory allocation accounted for in phase 2:
	    //   mrgArity*space_per_merge_item
	    m_mergeHeap->allocate( mrgArity ); //Allocate mem for mergeheap

	    // *****************************************************************
	    // *                                                               *
	    // * The main loop.  At the outermost level we are looping over    *
	    // * levels of the merge tree.  Typically this will be very small, *
	    // * e.g. 1-3.  The final merge pass is handled outside the loop.  *
	    // * Future extension may want to do something special in the last *
	    // * merge                                                         *
	    // *                                                               *
	    // *****************************************************************

	    if (m_indicator) {
		//compute merge depth, number of passes over data
		treeHeight= static_cast<int>(ceil(log(static_cast<float>(nRuns)) /
						  log(static_cast<float>(mrgArity))));
	    }
  
	    //nRuns is initially the number of runs we formed in partition_and_sort
	    //phase. nXtraRuns is initially the number of outputs streams that
	    //contain one extra run. Runs and nXtraRuns are updated as we 
	    //complete a merge level. 
	    while (nRuns > stream_offset_type(mrgArity)) {
		if (m_indicator) {
			std::string description;
			std::stringstream buf;
			buf << "Merge pass " << mrgHeight+1 << " of " << treeHeight << " ";
			buf >> description;
		    m_indicator->set_percentage_range(0, nInputItems);
		    m_indicator->init(description);
		}

		// We are not yet at the top of the merge tree
		// Write merged runs to temporary output streams
		TP_LOG_DEBUG ("Intermediate merge. level="<<mrgHeight << "\n");
    
		// The number of output runs we will form after a mrgArity merge
		nRuns = (nRuns + mrgArity - 1)/mrgArity;

		// Distribute the new nRuns evenly across mrgArity (or fewer)
		// output streams
		minRunsPerStream = nRuns/mrgArity;

		// We may have less mrgArity input runs for the last
		// merged output run if the current set of merge streams has 
		// xtra runs
		arity_t mergeRunsInLastOutputRun=(nXtraRuns>0) ? nXtraRuns : mrgArity;

		// The number of extra runs or the number of output streams that
		// get one additional run. This is less than mrgArity and
		// it is OK to downcast to an arity_t.
		nXtraRuns = static_cast<arity_t>(nRuns - minRunsPerStream*mrgArity);
		tp_assert(nXtraRuns<mrgArity, "Too many extra runs");

		// How many Streams we will create at the next level
		arity_t nOutputStreams = (minRunsPerStream > 0) ? mrgArity : nXtraRuns;

		arity_t nRunsToMerge = mrgArity; // may change for last output run

		// is current merge output the last run on this merge level?
		bool lastOutputRun = false;

		// open the mrgArity Input streams from which to read runs
		for(ii = 0; ii < mrgArity; ii++){
		    // Make the input file name
		    make_name(working_disk, suffixName[mrgHeight%2], ii, newName);
		    // Dynamically allocate the stream
		    // We account for these mmBytesPerStream in phase 2
		    // (input stream to read from)
		    mergeInputStreams[ii] = new stream<T>(newName);
		    mergeInputStreams[ii]->seek(0);
		}

		stream_offset_type check_size=0;
		// For each new output stream, fill with merged runs.
		// strange indexing is for the case that there are fewer than mrgArity
		// output streams needed, and we use the LAST nOutputStreams. This
		// always keeps the one possible short run in the LAST of the
		// mrgArity output streams.
		TP_LOG_DEBUG("Writing " << nRuns << " runs to " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(nOutputStreams)
			     << " output files.\nEach output file has at least "
			     << minRunsPerStream << " runs.\n");

		for(ii = mrgArity-nOutputStreams; ii < mrgArity; ii++){
		    // Make the output file name
		    make_name(working_disk, suffixName[(mrgHeight+1)%2], ii, newName);
		    // Dynamically allocate the stream
		    // We account for these mmBytesPerStream in phase 2
		    // (temp merge output stream)
		    curOutputRunStream = new stream<T>(newName);

		    // How many runs should this stream get?
		    // extra runs go in the LAST nXtraRuns streams so that
		    // the one short run is always in the LAST output stream
		    runsInStream = minRunsPerStream + ((ii >= mrgArity-nXtraRuns)?1:0);
		    TP_LOG_DEBUG("Writing " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(runsInStream) << " runs to output "
				 << " file " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(ii) << "\n");
		    for( jj=0; jj < runsInStream; jj++ ) { // For each run in this stream
			// See if this is the last run.
			if( (ii==mrgArity-1) && (jj==runsInStream-1)) {
			    lastOutputRun=true;
			    nRunsToMerge=mergeRunsInLastOutputRun;
			}
			// Merge runs to curOutputRunStream
			ae = single_merge(mergeInputStreams+mrgArity-nRunsToMerge,
					  nRunsToMerge, curOutputRunStream, 
					  nItemsPerRun); 
			if (ae != NO_ERROR) {
			    TP_LOG_FATAL_ID("AMI_single_merge error"<< ae <<" in deep merge");
			    return ae;
			}
		    } // For each output run in this stream

		    // Commit new output stream to disk
		    TP_LOG_DEBUG("Wrote " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(runsInStream) << " runs and "
				 << curOutputRunStream->stream_len() << " items " 
				 << "to file " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(ii) << "\n");
		    check_size+=curOutputRunStream->stream_len();
		    curOutputRunStream->persist(PERSIST_PERSISTENT);
		    delete curOutputRunStream;
		} // For each new output stream

		tp_assert(check_size==nInputItems, "item count mismatch in merge");
		// All output streams created/filled.
		// Clean up, go up to next level

		// Delete temp input merge streams
		for(ii = 0; ii < mrgArity; ii++){
		    mergeInputStreams[ii]->persist(PERSIST_DELETE);
		    delete mergeInputStreams[ii];
		}
		// Update run lengths
		nItemsPerRun=mrgArity*nItemsPerRun; //except for maybe last run
		mrgHeight++; // moving up a level
	    } // while (nRuns > mrgArity)

	    tp_assert( nRuns > 1, "Not enough runs to merge to final output");
	    tp_assert( nRuns <= stream_offset_type(mrgArity), "Too many runs to merge to final output");

	    // We are at the last merge phase, write to specified output stream
	    // Open up the nRuns final merge streams to merge
	    // These runs are packed in the LAST nRuns elements of the array
	    // nRuns is small, so it is safe to downcast.
	    TP_LOG_DEBUG_ID ("Final merge. level="<<mrgHeight);
	    TP_LOG_DEBUG("Merge runs left="<<nRuns<<"\n");
	    for(ii = mrgArity-static_cast<memory_size_type>(nRuns); ii < mrgArity; ii++){
		// Make the input file name
		make_name(working_disk, suffixName[mrgHeight%2], ii, newName);
		/* Dynamically allocate the stream
		   We account for these mmBytesPerStream in phase 2 
		   (input stream to read from)
		   Put LAST nRuns files in FIRST nRuns spots here
		   either one of mergeInputStreams loading or the call to
		   single_merge is a little messy. I put the mess here. (abd) */
		TP_LOG_DEBUG ("Putting merge stream "<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(ii) << " in slot "
			      << static_cast<TPIE_OS_OUTPUT_SIZE_T>(ii-(mrgArity-static_cast<memory_size_type>(nRuns))) << "\n");
		mergeInputStreams[ii-(mrgArity-static_cast<memory_size_type>(nRuns))] = new stream<T>(newName);
		mergeInputStreams[ii-(mrgArity-static_cast<memory_size_type>(nRuns))]->seek(0);
	    }

	    if (m_indicator) {
		m_indicator->set_percentage_range(0, nInputItems);
		m_indicator->init("Final merge pass  ");
	    }
	    // Merge last remaining runs to the output stream.
	    // mergeInputStreams is address( address (the first input stream) )
	    // N.B. nRuns is small, so it is safe to downcast.
	    ae = single_merge (mergeInputStreams, static_cast<arity_t>(nRuns), outStream);

	    tp_assert(outStream->stream_len() == nInputItems, "item count mismatch");

	    if (ae != NO_ERROR) {
		TP_LOG_FATAL_ID ("AMI_ERROR " << ae << " returned by single_merge "
				 << "in final merge phase");
		return ae;
	    }

	    TP_LOG_DEBUG("merge cleanup\n");

	    // We are done, except for cleanup. Is anyone still reading this?
	    // Delete temp input merge streams
	    // N.B. nRuns is small, so it is safe to downcast.
	    for(ii = 0; ii < arity_t(nRuns); ii++){
		mergeInputStreams[ii]->persist(PERSIST_DELETE);
		delete mergeInputStreams[ii];
	    }
	    // Delete stream ptr arrays
	    delete [] mergeInputStreams;
	    // Deallocate the merge heap, free up memory
	    m_mergeHeap->deallocate();
	    TP_LOG_DEBUG_ID ("Number of passes incl run formation is " <<
			     mrgHeight+2 ); 
	    TP_LOG_DEBUG("AMI_partition_and_merge END\n");
	    return NO_ERROR;
	}

	template<class T, class I, class M>
	err sort_manager<T,I,M>::single_merge( stream < T > **inStreams,
					       arity_t arity, stream < T >*outStream, stream_offset_type cutoff)
	{

	    return merge_sorted_runs(inStreams, arity, outStream, m_mergeHeap,
				     cutoff, m_indicator);
	}


	template<class T, class I, class M>
	inline void sort_manager<T,I,M>::make_name(
	    const std::string& prepre, const std::string& pre, memory_size_type id, std::string& dest)
	{
	    //This buffer must be long enough to hold the
	    //largest possible stream id (in decimal)
	    //largest ID is at most mrgArity
		std::stringstream buf;
		buf << prepre << pre << id;
		buf >> dest;
	}

    }  //  ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_SORT_MANAGER_H 
