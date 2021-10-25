#include "eigentools.h"

#include "moddepmgr.h"
#include "plugins.h"
#include "testprog.h"

#include <iostream>

static bool test_mirrorpad_odd()
{
    Eigen::ArrayXd input(7), output(14), expect(14);
    input << 1,2,3,4,5,6,7;
    expect << 3,2,1,1,2,3,4,5,6,7,7,6,5,4;
    mirrorPad( input, output );
    std::cout<<"test_mirrorpad_odd\n";
    std::cout<<output<<"\n";
    return (output==expect).all();
}

static bool test_mirrorpad_even()
{
    Eigen::ArrayXd input(6), output(12), expect(12);
    input << 1,2,3,4,5,6;
    expect << 3,2,1,1,2,3,4,5,6,6,5,4;
    mirrorPad( input, output );
    std::cout<<"test_mirrorpad_even\n";
    std::cout<<output<<"\n";
    return (output==expect).all();
}

static bool test_symmetricpad_odd()
{
    Eigen::ArrayXd input(7), output(14), expect(14);
    input << 1,2,3,4,5,6,7;
    expect << 4,3,2,1,2,3,4,5,6,7,6,5,4,3;
    symmetricPad( input, output );
    std::cout<<"test_symmetricpad_odd\n";
    std::cout<<output<<"\n";
    return (output==expect).all();
}

static bool test_symmetricpad_even()
{
    Eigen::ArrayXd input(6), output(12), expect(12);
    input << 1,2,3,4,5,6;
    expect << 4,3,2,1,2,3,4,5,6,5,4,3;
    symmetricPad( input, output );
    std::cout<<"test_symmetricpad_even\n";
    std::cout<<output<<"\n";
    return (output==expect).all();
}

static bool test_fftshift_even()
{
    Eigen::ArrayXd input(6), output(6), expect(6);
    input << 1,2,3,4,5,6;
    expect << 4,5,6,1,2,3;
    fftshift( input, output );
    return (output==expect).all();

}

static bool test_fftshift_odd()
{
    Eigen::ArrayXd input(7), output(7), expect(7);
    input << 1,2,3,4,5,6,7;
    expect << 5,6,7,1,2,3,4;
    fftshift( input, output );
    return (output==expect).all();

}

int mTestMainFnName( int argc, char** argv )
{
    mInitTestProg();
    PIM().loadAuto( false );
//    OD::ModDeps().ensureLoaded( "VMDAttrib" );
    PIM().loadAuto( true );

    if ( !test_mirrorpad_odd() )
	return 1;
    if ( !test_mirrorpad_even() )
	return 1;
    if ( !test_symmetricpad_odd() )
	return 1;
    if ( !test_symmetricpad_even() )
	return 1;
    if ( !test_fftshift_odd() )
	return 1;
    if ( !test_fftshift_even() )
	return 1;

    return 0;
}
