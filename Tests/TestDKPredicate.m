//
//  TestDKPredicate.m
//  Duck
//
//  Created by Derek Nylen on 2014-09-25.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <Duck/Duck.h>

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

    DKRuntimeInit( 0 );
    DKSetErrorCallback( RaiseException );
    DKPushAutoreleasePool();
}

- (void) tearDown
{
    DKPopAutoreleasePool();
    
    [super tearDown];
}

- (void) testDKPredicate
{
    DKBooleanRef nope = DKFalse();
    DKBooleanRef yep = DKTrue();
    
    DKListRef nope_nope = DKListWithObjects( nope, nope, NULL );
    DKListRef nope_yep = DKListWithObjects( nope, yep, NULL );
    DKListRef yep_nope = DKListWithObjects( yep, nope, NULL );
    DKListRef yep_yep = DKListWithObjects( yep, yep, NULL );

    DKPredicateRef test;

    // Not
    test = DKPredicate( DKPredicateNOT, nope, NULL );
    XCTAssert( DKEvaluate( test ) == true );

    test = DKPredicate( DKPredicateNOT, yep, nope );
    XCTAssert( DKEvaluate( test ) == false );

    // And
    test = DKPredicate( DKPredicateAND, nope, nope );
    XCTAssert( DKEvaluate( test ) == false );

    test = DKPredicate( DKPredicateAND, nope, yep );
    XCTAssert( DKEvaluate( test ) == false );

    test = DKPredicate( DKPredicateAND, yep, nope );
    XCTAssert( DKEvaluate( test ) == false );

    test = DKPredicate( DKPredicateAND, yep, yep );
    XCTAssert( DKEvaluate( test ) == true );
    
    // Or
    test = DKPredicate( DKPredicateOR, nope, nope );
    XCTAssert( DKEvaluate( test ) == false );

    test = DKPredicate( DKPredicateOR, nope, yep );
    XCTAssert( DKEvaluate( test ) == true );

    test = DKPredicate( DKPredicateOR, yep, nope );
    XCTAssert( DKEvaluate( test ) == true );

    test = DKPredicate( DKPredicateOR, yep, yep );
    XCTAssert( DKEvaluate( test ) == true );

    // All
    test = DKPredicate( DKPredicateALL, nope_nope, NULL );
    XCTAssert( DKEvaluate( test ) == false );

    test = DKPredicate( DKPredicateALL, nope_yep, NULL );
    XCTAssert( DKEvaluate( test ) == false );

    test = DKPredicate( DKPredicateALL, yep_nope, NULL );
    XCTAssert( DKEvaluate( test ) == false );

    test = DKPredicate( DKPredicateALL, yep_yep, NULL );
    XCTAssert( DKEvaluate( test ) == true );

    // Any
    test = DKPredicate( DKPredicateANY, nope_nope, NULL );
    XCTAssert( DKEvaluate( test ) == false );

    test = DKPredicate( DKPredicateANY, nope_yep, NULL );
    XCTAssert( DKEvaluate( test ) == true );

    test = DKPredicate( DKPredicateANY, yep_nope, NULL );
    XCTAssert( DKEvaluate( test ) == true );

    test = DKPredicate( DKPredicateANY, yep_yep, NULL );
    XCTAssert( DKEvaluate( test ) == true );

    // None
    test = DKPredicate( DKPredicateNONE, nope_nope, NULL );
    XCTAssert( DKEvaluate( test ) == true );

    test = DKPredicate( DKPredicateNONE, nope_yep, NULL );
    XCTAssert( DKEvaluate( test ) == false );

    test = DKPredicate( DKPredicateNONE, yep_nope, NULL );
    XCTAssert( DKEvaluate( test ) == false );

    test = DKPredicate( DKPredicateNONE, yep_yep, NULL );
    XCTAssert( DKEvaluate( test ) == false );

    // Eq
    test = DKPredicate( DKPredicateEQ, yep, yep );
    XCTAssert( DKEvaluate( test ) == true );

    test = DKPredicate( DKPredicateEQ, yep, nope );
    XCTAssert( DKEvaluate( test ) == false );
    
    // In
    test = DKPredicate( DKPredicateIN, yep, yep_nope );
    XCTAssert( DKEvaluate( test ) == true );

    test = DKPredicate( DKPredicateIN, yep, nope_nope );
    XCTAssert( DKEvaluate( test ) == false );

    test = DKPredicate( DKPredicateIN, DKSTR( "cat" ), DKSTR( "lolcat" ) );
    XCTAssert( DKEvaluate( test ) == true );

    test = DKPredicate( DKPredicateIN, DKSTR( "cat" ), DKSTR( "dinosaur" ) );
    XCTAssert( DKEvaluate( test ) == false );
    
    // Isa
    test = DKPredicate( DKPredicateISA, yep, DKNumberClass() );
    XCTAssert( DKEvaluate( test ) == true );

    test = DKPredicate( DKPredicateISA, yep, DKSelector(Comparison) );
    XCTAssert( DKEvaluate( test ) == true );

    test = DKPredicate( DKPredicateISA, yep, DKDataClass() );
    XCTAssert( DKEvaluate( test ) == false );

    test = DKPredicate( DKPredicateISA, yep, DKSelector(List) );
    XCTAssert( DKEvaluate( test ) == false );
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





