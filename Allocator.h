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
                int counted = 0;
                bool seen_free_once = false;

                while (counted < N) {
                        /* Add value to get to beginning sentinel */
                        SENTINEL_TYPE beg_sentinel = this->a[counted];

                        /* Checking for consecutive free blocks */
                        if(this->a[counted] > 0) {
                                if(seen_free_once)
                                        return false;
                                seen_free_once = true;
                        }
                        else{
                                seen_free_once = false;
                        }

                        counted += abs_val(beg_sentinel) + sizeof(SENTINEL_TYPE);

                        /* Check beginning and ending sentinal matches */
                        SENTINEL_TYPE end_sentinel = this->a[counted];
                        if (beg_sentinel != end_sentinel)
                                return false;
                        counted += sizeof(SENTINEL_TYPE);
                }

                if(counted == N)
                        return true;
                return false;
        }

        /**
         * O(1) in space
         * O(1) in time
         * <your documentation>
         * https://code.google.com/p/googletest/wiki/AdvancedGuide#Private_Class_Members
         */
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
                if(N < sizeof(T) + 2 * sizeof(SENTINEL_TYPE))
                    throw std::bad_alloc();

                int free_space = N - 2 * sizeof(SENTINEL_TYPE);
                (*this)[0] = free_space;
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
                if(n <= 0){
                    throw std::bad_alloc();
                }
                
                n = n * sizeof(T);
                SENTINEL_TYPE begin = this->a[0];
                int position = 0;
                while(position < N && begin < n + 2 * sizeof(SENTINEL_TYPE)) {
                        position += (begin + 2*sizeof(SENTINEL_TYPE));
                        begin = a[position];
                }

                if (position >= N)
                        throw std::bad_alloc();

                // allocating the whole block
                if (a[position] < n + 2 * sizeof(SENTINEL_TYPE) + 1) {
                        int sentinel = a[position];
                        a[position]*=-1;
                        a[position+sentinel]*=-1;
                        assert(valid());
                        return (pointer)&a[position+sizeof(SENTINEL_TYPE)];
                }
                // spliting the block to two
                int whole_block = a[position];
                int new_block_size = whole_block-2*sizeof(SENTINEL_TYPE)-n;
                a[position] = -1*n;
                a[position+sizeof(SENTINEL_TYPE)+n] = -1*n;
                a[position+2*sizeof(SENTINEL_TYPE)+n] = new_block_size;
                a[position+3*sizeof(SENTINEL_TYPE)+n+new_block_size] = new_block_size;
                assert(valid());
                return (pointer)&a[position+sizeof(SENTINEL_TYPE)];
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
         * <your documentation>
         */
        void deallocate(pointer p, size_type n) {

                /* Make sure p is valid */
                if((void*)p < &a[4] || (void*)p > &a[N-4])
                        throw std::invalid_argument("Pointer is out of bounds");

                int index = (char*)p - &a[0];
                /* Assume this block is beginning and end */
                int begin_sentinel_index = index - 4;
                int data = a[begin_sentinel_index] * -1;
                int end_sentinel_index = index + data;

                /* Check if block before needs to be combined */
                /* p cannot be first block */
                if(begin_sentinel_index >= 0) {
                        int before_block_sentinel_index = begin_sentinel_index - sizeof(SENTINEL_TYPE);

                        if (a[before_block_sentinel_index] > 0) {
                                data += a[before_block_sentinel_index] + 2 * sizeof(SENTINEL_TYPE);
                                begin_sentinel_index = before_block_sentinel_index - a[before_block_sentinel_index] - sizeof(SENTINEL_TYPE);
                        }
                }
                /* Check if block after needs to be combined */
                /* p cannot be last block */
                if(end_sentinel_index <= N - sizeof(SENTINEL_TYPE)) {
                        int after_block_sentinel = end_sentinel_index + sizeof(SENTINEL_TYPE);
                        if(a[after_block_sentinel] > 0) {
                                data += a[after_block_sentinel] + 2 * sizeof(SENTINEL_TYPE);
                                end_sentinel_index = after_block_sentinel + a[after_block_sentinel] + sizeof(SENTINEL_TYPE);

                        }
                }

                /* Combine */
                a[begin_sentinel_index] = data;
                a[end_sentinel_index] = data;
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
