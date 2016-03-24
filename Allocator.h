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
		// FRIEND_TEST for gtest
        FRIEND_TEST(TestAllocator, valid1);
        FRIEND_TEST(TestAllocator, valid2);
        FRIEND_TEST(TestAllocator, valid3);        
        FRIEND_TEST(TestAllocator, allocate1);        
        FRIEND_TEST(TestAllocator, allocate2);        
        FRIEND_TEST(TestAllocator, allocate3);        
        FRIEND_TEST(TestAllocator, deallocate1);        
        FRIEND_TEST(TestAllocator, deallocate2);        
        FRIEND_TEST(TestAllocator, deallocate3); 
        
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
                /* Have to account for N bytes */
            using namespace std;
            //cout << N << endl;
                int counted = 0;
                bool seen_free_once = false;
                std::cout << "N " << N; 

                while (counted < N) {
                        /* Add value to get to beginning sentinel */
                        int beg_sentinel = (*this)[counted];
                        //cout << "beg sent: " << beg_sentinel << endl;
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
                        //cout << "counted: " << counted << endl;
                        /* Check beginning and ending sentinal matches */
                        SENTINEL_TYPE end_sentinel = (*this)[counted];
                        //cout << "end sent: " << end_sentinel << endl;
                        if (beg_sentinel != end_sentinel){
                            //cout << "not matching sentinels" << endl;
                            //cout << "beginning sentinel " << beg_sentinel <<endl;
                            //cout << "ending sentinel " << end_sentinel <<endl;
                            return false;
                        }
                        counted += sizeof(SENTINEL_TYPE);
                }

                if(counted == N)
                        return true;

                //cout << "unaccounted for" <<endl;
                return false;
        }

        /**
         * O(1) in space
         * O(1) in time
         * <your documentation>
         * https://code.google.com/p/googletest/wiki/AdvancedGuide#Private_Class_Members
         */
        FRIEND_TEST(TestAllocatorConstructor, construct_int);
        FRIEND_TEST(TestAllocatorConstructor, construct_exception);
        FRIEND_TEST(TestAllocatorConstructor, construct_double);
        FRIEND_TEST(TestAllocator2, index);
        int& operator [] (int i) {
                return *reinterpret_cast<int*>(&a[i]);
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
            //cout << "ALLOCATE" << endl;
                if(n <= 0){
                    throw std::bad_alloc();
                }

                n = n * sizeof(T);
                SENTINEL_TYPE begin = (*this)[0];
                int position = 0;
                while(position < N && begin < n + 2 * sizeof(SENTINEL_TYPE)) {
                        position += (begin + 2 * sizeof(SENTINEL_TYPE));
                        begin = (*this)[position];
                }

                if (position >= N)
                        throw std::bad_alloc();

                // allocating the whole block
                if ((*this)[position] < n + 2 * sizeof(SENTINEL_TYPE) + 1) {
                        int sentinel = (*this)[position];
                        (*this)[position] *= -1;
                        (*this)[position + sentinel] *= -1;
                        assert(valid());
                        return (pointer) &(*this)[position + sizeof(SENTINEL_TYPE)];
                }
                // spliting the block into two blocks
                int whole_block = (*this)[position];
                int new_block_size = whole_block - 2 * sizeof(SENTINEL_TYPE) - n;
                (*this)[position] = -1 * n;
                (*this)[position + sizeof(SENTINEL_TYPE) + n] = -1 * n;
                (*this)[position + 2 * sizeof(SENTINEL_TYPE) + n] = new_block_size;
                (*this)[position + 3 * sizeof(SENTINEL_TYPE) + n + new_block_size] = new_block_size;
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
                //cout << "DEALLOCATE" << endl;
                assert(valid());
                /* Make sure p is valid */
                if((void*)p < &(*this)[4] || (void*)p > &(*this)[N - 4])
                        throw std::invalid_argument("Pointer is out of bounds");

                int index = (char*)p - &a[0];
                //cout << "given index " << index << endl;
                /* Assume this block is beginning and end */
                int begin_sentinel_index = index - 4;
                int data = (*this)[begin_sentinel_index] * -1;
                int end_sentinel_index = index + data;
                //cout << "Before combine" <<  begin_sentinel_index << endl;
                /* Check if block before needs to be combined */
                /* p cannot be first block */
                if(begin_sentinel_index >= 0) {
                        int before_block_sentinel_index = begin_sentinel_index - sizeof(SENTINEL_TYPE);

                        if ((*this)[before_block_sentinel_index] > 0) {
                                data += (*this)[before_block_sentinel_index] + 2 * sizeof(SENTINEL_TYPE);
                                begin_sentinel_index = before_block_sentinel_index - (*this)[before_block_sentinel_index] - sizeof(SENTINEL_TYPE);
                        }
                }
                /* Check if block after needs to be combined */
                /* p cannot be last block */
                if(end_sentinel_index <= N - sizeof(SENTINEL_TYPE)) {
                        int after_block_sentinel = end_sentinel_index + sizeof(SENTINEL_TYPE);
                        if((*this)[after_block_sentinel] > 0) {
                                data += (*this)[after_block_sentinel] + 2 * sizeof(SENTINEL_TYPE);
                                end_sentinel_index = after_block_sentinel + (*this)[after_block_sentinel] + sizeof(SENTINEL_TYPE);

                        }
                }

                /* Combine */
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
