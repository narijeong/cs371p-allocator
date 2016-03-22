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

#include <cassert>   // assert
#include <cstddef>   // ptrdiff_t, size_t
#include <new>       // bad_alloc, new
#include <stdexcept> // invalid_argument


#define SENTINEL_TYPE int

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
        }                                                              // this is correct

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
         * <your documentation>
         */
        bool valid () const {
                using namespace std;
                /* Have to account for N bytes */
                int counted = 0;
                bool seen_free_once = false;
                while (counted < N) {
                        /* Add value to get to beginning sentinel */
                        SENTINEL_TYPE beg_sentinel = this->a[counted];
                        cout << beg_sentinel << endl;

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
                        cout << end_sentinel << endl;
                        if (beg_sentinel != end_sentinel)
                                return false;
                        counted += sizeof(SENTINEL_TYPE);

                        cout << "total counted: " << counted << endl;
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
        // constructorss
        // ------------

        /**
         * O(1) in space
         * O(1) in time
         * throw a bad_alloc exception, if N is less than sizeof(T) + (2 * sizeof(int))
         */
        Allocator () {
                using namespace std;
                int free_space = N - 2 * sizeof(SENTINEL_TYPE);
                (*this)[0] = free_space;
                (*this)[N - sizeof(SENTINEL_TYPE)] = free_space;
                cout << "Size of heap is " << N << endl;
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
         * throw a bad_alloc exception, if n is invalid
         */
        pointer allocate (size_type n) {
                using namespace std;
                n = n * sizeof(T);
                SENTINEL_TYPE begin = this->a[0];
                int position = 0;
                while(position < N && begin < n + 2*sizeof(SENTINEL_TYPE)) {
                        position += (begin + 2*sizeof(SENTINEL_TYPE));
                        begin = a[position];
                }

                if(position >= N)
                        throw std::bad_alloc();

                // allocating the whole block
                if(a[position] < n+2*sizeof(SENTINEL_TYPE)+1) {
                        int sentinel = a[position];
                        a[position]*=-1;
                        a[position+sentinel]*=-1;
                        cout << "Allocating whole block " << endl;
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
                cout << "Splitting block into two. required " << n << " bytes " << endl;
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
                cout << "begin construct" <<endl;

                new (p) T(v);                     // this is correct and exempt
                assert(valid());                 // from the prohibition of new
                cout << "finished construct" <<endl;
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
        void deallocate (pointer p, size_type) {
                using namespace std;
                cout << "DEALLOCATE" <<endl;
                /* Make sure p is valid */
                void* voidp = (void*) p;
                if(voidp < a || voidp > a+N)
                        throw std::invalid_argument("Pointer is out of bounds");

                /* Assume this block is beginning and end */
                int* beginning_sentinel = (int*)voidp - sizeof(SENTINEL_TYPE);
                int data = *beginning_sentinel * -1;
                int* end_sentinel = (int*)voidp + data;

                /* Check if block before needs to be combined */
                /* p cannot be first block */
                if((void*)beginning_sentinel != (void*)a) {
                        int* before_end_sentinel = beginning_sentinel - sizeof(SENTINEL_TYPE);

                        if(*before_end_sentinel > 0) {
                                data += *before_end_sentinel + 2*sizeof(SENTINEL_TYPE);
                                beginning_sentinel = before_end_sentinel + (*before_end_sentinel * -1) + (-1 * sizeof(SENTINEL_TYPE));
                        }
                }
                /* Check if block after needs to be combined */
                /* p cannot be last block */
                if((void*)end_sentinel != (void*)(a + N - sizeof(SENTINEL_TYPE))) {
                        int* after_begin_sentinel = end_sentinel + sizeof(SENTINEL_TYPE);
                        if(*after_begin_sentinel > 0) {
                                data += *after_begin_sentinel + 2*sizeof(SENTINEL_TYPE);
                                end_sentinel = after_begin_sentinel + *after_begin_sentinel + sizeof(SENTINEL_TYPE);
                        }
                }

                /* Combine */
                *beginning_sentinel = data;
                *end_sentinel = data;

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
