// ------------------------------
// projects/allocator/Allocator.h
// Copyright (C) 2015
// Glenn P. Downing
// ------------------------------

#ifndef Allocator_h
#define Allocator_h

// --------
// includes
// --------

#include <cassert>    // assert
#include <cstddef>    // ptrdiff_t, size_t
#include <new>        // bad_alloc, new
#include <stdexcept>  // invalid_argument


#define SENTINEL_TYPE int

// -----
// valid
// -----

/**
* O(1) in space
* O(1) in time
* calculates |x| and returns the value
*/

static int abs_val(int x) {
        if (x < 0)
                return x * -1;
        return x;
}

// ---------
// Allocator
// ---------

template <typename T, std::size_t N>
class Allocator {
public:
        // --------
        // typedefs
        // --------

        typedef T value_type;

        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

        typedef       value_type*       pointer;
        typedef const value_type* const_pointer;

        typedef       value_type&       reference;
        typedef const value_type& const_reference;

public:
        // -----------
        // operator ==
        // -----------

        friend bool operator == (const Allocator &, const Allocator &) {
                return true;
        }                                                     // this is correct

        // -----------
        // operator !=
        // -----------

        friend bool operator != (const Allocator &lhs, const Allocator &rhs) {
                return !(lhs == rhs);
        }

private:
		// ----------------------
		// FRIEND_TEST for gtest
		// ----------------------
        FRIEND_TEST(TestAllocatorValid, valid1);
        FRIEND_TEST(TestAllocatorValid, valid2);
        FRIEND_TEST(TestAllocatorValid, valid3);        
        FRIEND_TEST(TestAllocatorAllocate, allocate1);        
        FRIEND_TEST(TestAllocatorAllocate, allocate2);        
        FRIEND_TEST(TestAllocatorAllocate, allocate3);        
        FRIEND_TEST(TestAllocatorDeallocate, deallocate1);        
        FRIEND_TEST(TestAllocatorDeallocate, deallocate2);        
        FRIEND_TEST(TestAllocatorDeallocate, deallocate3); 
        FRIEND_TEST(TestAllocatorConstructor, construct_int);
        FRIEND_TEST(TestAllocatorConstructor, construct_exception);
        FRIEND_TEST(TestAllocatorConstructor, construct_double);
        
        /**
         * O(1) in space
         * O(1) in time
         * operator [] to use for gtest
         * https://code.google.com/p/googletest/wiki/AdvancedGuide#Private_Class_Members
         */
        FRIEND_TEST(TestAllocator2, index);
        int& operator [] (int i) {
                return *reinterpret_cast<int*>(&a[i]);
        }
        
        
        // ----
        // data
        // ----

        char a[N];

        // -----
        // valid
        // -----

        /**
         * O(1) in space
         * O(n) in time
         * valid checks for the following two conditions to ensure correct state
         * 1. All free and allocated bytes add up to N
         * 2. Ensure that there are no two free blocks in a row
         */
        bool valid () const {
			std::cout << "printing valid()" << std::endl;
                /* Have to account for N bytes */
            using namespace std;
            cout << N << endl;
                int counted = 0;
                bool seen_free_once = false;

                while (counted < N) {
                        /* Add value to get to beginning sentinel */
                        int beg_sentinel = (*this)[counted];
                        cout << "beg sent: " << beg_sentinel << endl;
                        cout << "beg index: " << counted << endl;
                        /* Checking for consecutive free blocks */
                        if((*this)[counted] > 0) {
                                if(seen_free_once){
                                    return false;
                                }
                                seen_free_once = true;
                        }
                        else{
                                seen_free_once = false;
                        }

                        counted += abs_val(beg_sentinel) + sizeof(SENTINEL_TYPE);
                        /* Check beginning and ending sentinal matches */
                        SENTINEL_TYPE end_sentinel = (*this)[counted];
                        cout << "end sent: " << end_sentinel << endl;
                        cout << "end index: " << counted << endl;
                        if (beg_sentinel != end_sentinel){
                            cout << "not matching sentinels" << endl;
                            cout << "beginning sentinel " << beg_sentinel <<endl;
                            cout << "ending sentinel " << end_sentinel <<endl;
                            return false;
                        }
                        counted += sizeof(SENTINEL_TYPE);
                }

                if(counted == N)
                        return true;

                cout << "unaccounted for" <<endl;
                return false;
                std::cout << "end valid()" << std::endl;

        }
        
public:
        // ------------
        // constructors
        // ------------

        /**
         * O(1) in space
         * O(1) in time
         * Throws a bad_alloc exception, if N is less than sizeof(T) + (2 * sizeof(int))
         * Otherwise, initializes memory by adding beginning and ending sentinels
         */
        Allocator () {
            using namespace std;
            cout << "!!!!! SIZE OF T: " << sizeof(T) << endl;
                if(N < sizeof(T) + 2 * sizeof(SENTINEL_TYPE))
                    throw std::bad_alloc();

                int free_space = N - 2 * sizeof(SENTINEL_TYPE);
                //cout << "free space " << free_space << endl;
                (*this)[0] = free_space;

                //cout << "a[0] = " << (*this)[0] << endl;
                (*this)[N - sizeof(SENTINEL_TYPE)] = free_space;
                assert(valid());
        }

        // Default copy, destructor, and copy assignment
        // Allocator  (const Allocator&);
        // ~Allocator ();
        // Allocator& operator = (const Allocator&);

        // --------
        // allocate
        // --------

        /**
         * O(1) in space
         * O(n) in time
         * after allocation there must be enough space left for a valid block
         * the smallest allowable block is sizeof(T) + (2 * sizeof(int))
         * choose the first block that fits
         * throw a bad_alloc exception, if n <= 0 or no possible fit
         */
        pointer allocate(size_type n) {
            using namespace std;
            cout << "ALLOCATE" << endl;
                if(n <= 0){
                    throw std::bad_alloc();
                }

                n = n * sizeof(T);
                SENTINEL_TYPE freespace = (*this)[0];
                int position = 0;

                /* find first fit */
                cout << "position: " << position << endl;
                cout << "TOTAL: " << N << endl;
                cout << "pos < TOTAL = " << (position < N) << endl;
                cout << "freespace: " << freespace << endl; 
                cout << "required space: " << n << endl;
                cout << "free < req = " << ((int)freespace < (int) n) << endl;


                while(position < N && (int)freespace < (int)n) {
                        cout << "INSIDE WHILE.........." << endl;    
                        position += (abs_val(freespace) + 2 * sizeof(SENTINEL_TYPE));
                        freespace = (*this)[position];
                        cout << "position: " << position << endl;
                        cout << "freespace: " << freespace << endl;
                }

                if (position >= N)
                        throw std::bad_alloc();

                int total = (*this)[position] + 2 * sizeof(SENTINEL_TYPE);
                int required = total - (int)n - 2 * sizeof(SENTINEL_TYPE);
                int second_sentinel_value = required - 2 * sizeof(SENTINEL_TYPE);
                
                // allocating whole block
                if(second_sentinel_value < (int)sizeof(T))
                {
                        cout << "allocating whole......" <<endl;
                        int sentinel = (*this)[position];
                        (*this)[position] = sentinel * -1;
                        (*this)[position + sizeof(SENTINEL_TYPE) + sentinel] = sentinel * -1;
                }
                else{
                    // spliting the block into two blocks
                    cout << "splitting......" <<endl;
                    //int whole_block = (*this)[position];
                    //int new_block_size = whole_block - 2 * sizeof(SENTINEL_TYPE) - n;
                    (*this)[position] = -1 * n;
                    (*this)[position + sizeof(SENTINEL_TYPE) + n] = -1 * n;
                    (*this)[position + 2 * sizeof(SENTINEL_TYPE) + n] = second_sentinel_value;
                    (*this)[position + 3 * sizeof(SENTINEL_TYPE) + n + second_sentinel_value] = second_sentinel_value;
                    cout << "allocate" <<endl;
                }

                assert(valid());
                return (pointer) &(*this)[position + sizeof(SENTINEL_TYPE)];
        }

        // ---------
        // construct
        // ---------

        /**
         * O(1) in space
         * O(1) in time
         */
        void construct (pointer p, const_reference v) {
                using namespace std;
                new (p) T(v);                     // this is correct and exempt
                assert(valid());                 // from the prohibition of new
        }

        // ----------
        // deallocate
        // ----------
        /**
         * O(1) in space
         * O(1) in time
         * after deallocation adjacent free blocks must be coalesced
         * throw an invalid_argument exception, if p is invalid
         */
        void deallocate(pointer p, size_type n) {
                using namespace std;
                
                cout << endl;
                cout << "DEALLOCATE" << endl;
                
                assert(valid());
                /* Make sure p is valid */
                if((void*)p < &(*this)[sizeof(SENTINEL_TYPE)] || (void*)p > &(*this)[N - sizeof(SENTINEL_TYPE)])
                        throw std::invalid_argument("Pointer is out of bounds");
				//TO DO: see if p is deallocabable pointer inside the bounds(check first and last sentinel same, and positive)
                
                int val_index = (char*)p - &a[0];
                cout << "given value index " << val_index << endl;
                /* Assume this block is beginning and end */
                int begin_sentinel_index = val_index - sizeof(SENTINEL_TYPE);
                int data = (*this)[begin_sentinel_index] * -1;
                int end_sentinel_index = val_index + data;
                cout << "begin_sentinel_index " <<  begin_sentinel_index << endl;
                cout << "end_sentinel_index " <<  end_sentinel_index << endl;
                
                /* Check if block before needs to be combined */
                /* p cannot be first block */
                int before_block_sentinel_index = begin_sentinel_index - sizeof(SENTINEL_TYPE);
                cout << "before_block_sentinel_index " << before_block_sentinel_index << endl;
                cout << "range " << sizeof(T) + sizeof(SENTINEL_TYPE) << endl; 
                if(before_block_sentinel_index >= (sizeof(T) + sizeof(SENTINEL_TYPE))) {
						if((*this)[before_block_sentinel_index] > 0) {
							cout << "combining with the before block" << endl;
							data += (*this)[before_block_sentinel_index] + 2 * sizeof(SENTINEL_TYPE);
							begin_sentinel_index = before_block_sentinel_index - (*this)[before_block_sentinel_index] - sizeof(SENTINEL_TYPE);
							cout << "changed begin_sentinel_index " << begin_sentinel_index << endl;
						}
				}

                /* Check if block after needs to be combined */
                /* p cannot be last block */
                int after_block_sentinel_index = val_index + (*this)[end_sentinel_index]*-1 + sizeof(SENTINEL_TYPE);
                if(after_block_sentinel_index <= (N - sizeof(T) - sizeof(SENTINEL_TYPE))) {
					    if((*this)[after_block_sentinel_index] > 0) {
							cout << "combining with the after block" << endl;
							cout << "after_block_sentinel " << (*this)[after_block_sentinel_index] << endl;
							data += (*this)[after_block_sentinel_index] + 2 * sizeof(SENTINEL_TYPE);
							end_sentinel_index = after_block_sentinel_index + sizeof(SENTINEL_TYPE) + (*this)[after_block_sentinel_index];
							cout << "changed end_sentinel_index " << end_sentinel_index << endl;	
						}					
                }

                /* Combine */
                cout << "assign begin and end sentinels after deallocating" << endl;
                cout << "begin_sentinel_index " <<  begin_sentinel_index << endl;
                cout << "end_sentinel_index " <<  end_sentinel_index << endl;
                cout << "data " << data << endl;
                (*this)[begin_sentinel_index] = data;
                (*this)[end_sentinel_index] = data;
                assert(valid());
        }

        // -------
        // destroy
        // -------

        /**
         * O(1) in space
         * O(1) in time
         */
        void destroy (pointer p) {
                p->~T();           // this is correct
                assert(valid());
        }

        /**
         * O(1) in space
         * O(1) in time
         * <your documentation>
         */
        const int& operator [] (int i) const {
                return *reinterpret_cast<const int*>(&a[i]);
        }
};

#endif // Allocator_h
