// ------------------------------------
// projects/allocator/TestAllocator1.c++
// Copyright (C) 2015
// Glenn P. Downing
// ------------------------------------

// --------
// includes
// --------

#include <algorithm> // count
#include <memory>    // allocator

#include "gtest/gtest.h"

#include "Allocator.h"

// --------------
// TestAllocator1
// --------------

template <typename A>
struct TestAllocator1 : testing::Test {
    // --------
    // typedefs
    // --------

    typedef          A             allocator_type;
    typedef typename A::value_type value_type;
    typedef typename A::size_type  size_type;
    typedef typename A::pointer    pointer;};

typedef testing::Types<
            std::allocator<int>,
            std::allocator<double>,
            Allocator<int,    100>,
            Allocator<double, 100> >
        my_types_1;

TYPED_TEST_CASE(TestAllocator1, my_types_1);

TYPED_TEST(TestAllocator1, test_1) {
    typedef typename TestFixture::allocator_type allocator_type;
    typedef typename TestFixture::value_type     value_type;
    typedef typename TestFixture::size_type      size_type;
    typedef typename TestFixture::pointer        pointer;

          allocator_type x;
    const size_type      s = 1;
    const value_type     v = 2;
    const pointer        p = x.allocate(s);
    if (p != nullptr) {
        x.construct(p, v);
        ASSERT_EQ(v, *p);
        x.destroy(p);
        x.deallocate(p, s);}}

TYPED_TEST(TestAllocator1, test_10) {
    typedef typename TestFixture::allocator_type allocator_type;
    typedef typename TestFixture::value_type     value_type;
    typedef typename TestFixture::size_type      size_type;
    typedef typename TestFixture::pointer        pointer;

          allocator_type  x;
    const size_type       s = 10;
    const value_type      v = 2;
    const pointer         b = x.allocate(s);
    if (b != nullptr) {
        pointer e = b + s;
        pointer p = b;
        try {
            while (p != e) {
                x.construct(p, v);
                ++p;}}
        catch (...) {
            while (b != p) {
                --p;
                x.destroy(p);}
            x.deallocate(b, s);
            throw;}
        ASSERT_EQ(s, std::count(b, e, v));
        while (b != e) {
            --e;
            x.destroy(e);}
        x.deallocate(b, s);}}

// --------------
// TestAllocator2
// --------------

TEST(TestAllocator2, const_index) {
    const Allocator<int, 100> x;
    ASSERT_EQ(x[0], 92);}

TEST(TestAllocator2, index) {
    Allocator<int, 100> x;
    ASSERT_EQ(x[0], 92);}

// --------------
// TestAllocator3
// --------------

template <typename A>
struct TestAllocator3 : testing::Test {
    // --------
    // typedefs
    // --------

    typedef          A             allocator_type;
    typedef typename A::value_type value_type;
    typedef typename A::size_type  size_type;
    typedef typename A::pointer    pointer;};

typedef testing::Types<
            Allocator<int,    100>,
            Allocator<double, 100> >
        my_types_2;

TYPED_TEST_CASE(TestAllocator3, my_types_2);

TYPED_TEST(TestAllocator3, test_1) {
    typedef typename TestFixture::allocator_type allocator_type;
    typedef typename TestFixture::value_type     value_type;
    typedef typename TestFixture::size_type      size_type;
    typedef typename TestFixture::pointer        pointer;

          allocator_type x;
    const size_type      s = 1;
    const value_type     v = 2;
    const pointer        p = x.allocate(s);
    if (p != nullptr) {
        x.construct(p, v);
        ASSERT_EQ(v, *p);
        x.destroy(p);
        x.deallocate(p, s);}}

TYPED_TEST(TestAllocator3, test_10) {
    typedef typename TestFixture::allocator_type allocator_type;
    typedef typename TestFixture::value_type     value_type;
    typedef typename TestFixture::size_type      size_type;
    typedef typename TestFixture::pointer        pointer;

          allocator_type x;
    const size_type      s = 10;
    const value_type     v = 2;
    const pointer        b = x.allocate(s);
    if (b != nullptr) {
        pointer e = b + s;
        pointer p = b;
        try {
            while (p != e) {
                x.construct(p, v);
                ++p;}}
        catch (...) {
            while (b != p) {
                --p;
                x.destroy(p);}
            x.deallocate(b, s);
            throw;}
        ASSERT_EQ(s, std::count(b, e, v));
        while (b != e) {
            --e;
            x.destroy(e);}
        x.deallocate(b, s);}
    }

    TEST(TestAllocatorConstructor, construct_int) {
        const int N = 100;
        Allocator<int, N> x;
        ASSERT_EQ(x[0], N - 2 * sizeof(SENTINEL_TYPE));
        ASSERT_EQ(x[N - sizeof(SENTINEL_TYPE)], N - 2 * sizeof(SENTINEL_TYPE));
    }
    
    TEST(TestAllocatorConstructor, construct_exception) {
        try{
            Allocator<int, sizeof(int)> x;
            FAIL();
        }catch(const std::bad_alloc& e){
            SUCCEED();
            return;
        }catch(...){
            FAIL();
        }
        FAIL();
    }   

    TEST(TestAllocatorConstructor, construct_double) {
        const int N = 100;
        Allocator<double, N> x;
        ASSERT_EQ(x[0], N - 2 * sizeof(SENTINEL_TYPE));
        ASSERT_EQ(x[N - sizeof(SENTINEL_TYPE)], N - 2 * sizeof(SENTINEL_TYPE));
    }

// --------------
// Test casses
// --------------


TEST(TestAllocator, valid1) {
	Allocator<int, 105> x;
	ASSERT_EQ(x.valid(), true);}

TEST(TestAllocator, valid2) {
	Allocator<double, 80> x;
	ASSERT_EQ(x.valid(), true);}

TEST(TestAllocator, valid3) {
	Allocator<char, 150> x;
	ASSERT_EQ(x.valid(), true);}

/*
TEST(TestAllocator, allocate1) {
	Allocator<int, 105> x;
	int *position = &x[0]+1;
	ASSERT_EQ(x.allocate(2), position);}

TEST(TestAllocator, allocate2) {
	Allocator<double, 80> x;
	char *position = &x[0]+13;
	x.allocate(5);
	ASSERT_EQ(x.allocate(8), position);} */
TEST(TestAllocator, allocate3) {
    Allocator<int, 100> x;
    int* first = x.allocate(10);
    int* second = x.allocate(10);
;}
/*
TEST(TestAllocator, deallocate1) {
	Allocator<int, 2000> x;
	ASSERT_EQ(x.valid(), true);}
	
TEST(TestAllocator, deallocate2) {
;}
TEST(TestAllocator, deallocate3) {
;}TEST(TestAllocator, ) {
;}TEST(TestAllocator, valid1) {
;}TEST(TestAllocator, valid1) {
;}TEST(TestAllocator, valid1) {
;}*/
    