//
//  TestDKPredicate.m
//  Duck
//
//  Created by Derek Nylen on 2014-09-25.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}

@interface TestDKPredicate : XCTestCase

@end

@implementation TestDKPredicate

- (void) setUp
{
    [super setUp];

    DKSetErrorCallback( RaiseException );
}

- (void) tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void) testDKPredicate
{
    DKPredicateRef test = NULL;
    #define PREDICATE( op, a, b )               \
    {                                           \
        DKRelease( test );                      \
        test = DKPredicateCreate( op, a, b );   \
    }
    
    DKNumberRef nope = DKNumberCreateInt32( 0 );
    DKNumberRef yep = DKNumberCreateInt32( 1 );
    
    DKListRef nope_nope = DKListCreateWithObjects( NULL, nope, nope, NULL );
    DKListRef nope_yep = DKListCreateWithObjects( NULL, nope, yep, NULL );
    DKListRef yep_nope = DKListCreateWithObjects( NULL, yep, nope, NULL );
    DKListRef yep_yep = DKListCreateWithObjects( NULL, yep, yep, NULL );

    // True/False
    PREDICATE( DKPredicateTRUE, NULL, NULL );
    XCTAssert( DKEvaluate( test ) == true );
    
    PREDICATE( DKPredicateFALSE, NULL, NULL );
    XCTAssert( DKEvaluate( test ) == false );

    // Not
    PREDICATE( DKPredicateNOT, nope, NULL );
    XCTAssert( DKEvaluate( test ) == true );

    PREDICATE( DKPredicateNOT, yep, nope );
    XCTAssert( DKEvaluate( test ) == false );

    // And
    PREDICATE( DKPredicateAND, nope, nope );
    XCTAssert( DKEvaluate( test ) == false );

    PREDICATE( DKPredicateAND, nope, yep );
    XCTAssert( DKEvaluate( test ) == false );

    PREDICATE( DKPredicateAND, yep, nope );
    XCTAssert( DKEvaluate( test ) == false );

    PREDICATE( DKPredicateAND, yep, yep );
    XCTAssert( DKEvaluate( test ) == true );
    
    PREDICATE( DKPredicateAND, nope_nope, NULL );
    XCTAssert( DKEvaluate( test ) == false );

    PREDICATE( DKPredicateAND, nope_yep, NULL );
    XCTAssert( DKEvaluate( test ) == false );

    PREDICATE( DKPredicateAND, yep_nope, NULL );
    XCTAssert( DKEvaluate( test ) == false );

    PREDICATE( DKPredicateAND, yep_yep, NULL );
    XCTAssert( DKEvaluate( test ) == true );

    // Or
    PREDICATE( DKPredicateOR, nope, nope );
    XCTAssert( DKEvaluate( test ) == false );

    PREDICATE( DKPredicateOR, nope, yep );
    XCTAssert( DKEvaluate( test ) == true );

    PREDICATE( DKPredicateOR, yep, nope );
    XCTAssert( DKEvaluate( test ) == true );

    PREDICATE( DKPredicateOR, yep, yep );
    XCTAssert( DKEvaluate( test ) == true );

    PREDICATE( DKPredicateOR, nope_nope, NULL );
    XCTAssert( DKEvaluate( test ) == false );

    PREDICATE( DKPredicateOR, nope_yep, NULL );
    XCTAssert( DKEvaluate( test ) == true );

    PREDICATE( DKPredicateOR, yep_nope, NULL );
    XCTAssert( DKEvaluate( test ) == true );

    PREDICATE( DKPredicateOR, yep_yep, NULL );
    XCTAssert( DKEvaluate( test ) == true );

    // Eq
    PREDICATE( DKPredicateEQ, yep, yep );
    XCTAssert( DKEvaluate( test ) == true );

    PREDICATE( DKPredicateEQ, yep, nope );
    XCTAssert( DKEvaluate( test ) == false );
    
    // In
    PREDICATE( DKPredicateIN, yep, yep_nope );
    XCTAssert( DKEvaluate( test ) == true );

    PREDICATE( DKPredicateIN, yep, nope_nope );
    XCTAssert( DKEvaluate( test ) == false );

    PREDICATE( DKPredicateIN, DKSTR( "cat" ), DKSTR( "lolcat" ) );
    XCTAssert( DKEvaluate( test ) == true );

    PREDICATE( DKPredicateIN, DKSTR( "cat" ), DKSTR( "dinosaur" ) );
    XCTAssert( DKEvaluate( test ) == false );
    
    // Isa
    PREDICATE( DKPredicateISA, yep, DKNumberClass() );
    XCTAssert( DKEvaluate( test ) == true );

    PREDICATE( DKPredicateISA, yep, DKSelector(Comparison) );
    XCTAssert( DKEvaluate( test ) == true );

    PREDICATE( DKPredicateISA, yep, DKDataClass() );
    XCTAssert( DKEvaluate( test ) == false );

    PREDICATE( DKPredicateISA, yep, DKSelector(List) );
    XCTAssert( DKEvaluate( test ) == false );

    // Cleanup
    DKRelease( nope );
    DKRelease( yep );
    
    DKRelease( nope_nope );
    DKRelease( nope_yep );
    DKRelease( yep_nope );
    DKRelease( yep_yep );
    
    DKRelease( test );
}


/*
- (void) testPerformanceExample
{
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}
*/

@end





