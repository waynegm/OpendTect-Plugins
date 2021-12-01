#include "eigentools.h"

#include "moddepmgr.h"
#include "plugins.h"
#include "testprog.h"

#include <iostream>

using namespace EigenTools;
using namespace std;

static bool test_mirrorpad_odd()
{
    Eigen::ArrayXd input(7), output(14), expect(14);
    input << 1,2,3,4,5,6,7;
    expect << 3,2,1,1,2,3,4,5,6,7,7,6,5,4;
    mirrorPad( input, output );
    if ((output!=expect).all())
    {
	cout<<"test_mirrorpad_odd failed:\n";
	cout<<"Expected - Received\n";
	for ( int idx=0; idx<output.size(); idx++)
	    cout<< to_string(expect(idx)) <<" - "<< to_string(output(idx)) << "\n";
	    return false;
	}
    cout<<"test_mirrorpad_odd: OK\n";
    return true;
}

static bool test_mirrorpad_even()
{
    Eigen::ArrayXd input(6), output(12), expect(12);
    input << 1,2,3,4,5,6;
    expect << 3,2,1,1,2,3,4,5,6,6,5,4;
    mirrorPad( input, output );
    if ((output!=expect).all())
    {
	cout<<"test_mirrorpad_even failed:\n";
	cout<<"Expected - Received\n";
	for ( int idx=0; idx<output.size(); idx++)
	    cout<< to_string(expect(idx)) <<" - "<< to_string(output(idx)) << "\n";
	    return false;
	}
    cout<<"test_mirrorpad_even: OK\n";
    return true;
}

static bool test_symmetricpad_odd()
{
    Eigen::ArrayXd input(7), output(14), expect(14);
    input << 1,2,3,4,5,6,7;
    expect << 4,3,2,1,2,3,4,5,6,7,6,5,4,3;
    symmetricPad( input, output );
    if ((output!=expect).all())
    {
	cout<<"test_symmetricpad_odd failed:\n";
	cout<<"Expected - Received\n";
	for ( int idx=0; idx<output.size(); idx++)
	    cout<< to_string(expect(idx)) <<" - "<< to_string(output(idx)) << "\n";
	    return false;
	}
    cout<<"test_symmetricpad_odd: OK\n";
    return true;
}

static bool test_symmetricpad_even()
{
    Eigen::ArrayXd input(6), output(12), expect(12);
    input << 1,2,3,4,5,6;
    expect << 4,3,2,1,2,3,4,5,6,5,4,3;
    symmetricPad( input, output );
    if ((output!=expect).all())
    {
	cout<<"test_symmetricpad_even failed:\n";
	cout<<"Expected - Received\n";
	for ( int idx=0; idx<output.size(); idx++)
	    cout<< to_string(expect(idx)) <<" - "<< to_string(output(idx)) << "\n";
	    return false;
	}
    cout<<"test_symmetricpad_even: OK\n";
    return true;
}

static bool test_fftshift_even()
{
    Eigen::ArrayXd input(6), output(6), expect(6);
    input << 1,2,3,4,5,6;
    expect << 4,5,6,1,2,3;
    fftshift( input, output );
    if ((output!=expect).all())
    {
	cout<<"test_fftshift_even failed:\n";
	cout<<"Expected - Received\n";
	for ( int idx=0; idx<output.size(); idx++)
	    cout<< to_string(expect(idx)) <<" - "<< to_string(output(idx)) << "\n";
	    return false;
	}
    cout<<"test_fftshift_even: OK\n";
    return true;
}

static bool test_fftshift_odd()
{
    Eigen::ArrayXd input(7), output(7), expect(7);
    input << 1,2,3,4,5,6,7;
    expect << 5,6,7,1,2,3,4;
    fftshift( input, output );
    if ((output!=expect).all())
    {
	cout<<"test_fftshift_odd failed:\n";
	cout<<"Expected - Received\n";
	for ( int idx=0; idx<output.size(); idx++)
	    cout<< to_string(expect(idx)) <<" - "<< to_string(output(idx)) << "\n";
	    return false;
	}
    cout<<"test_fftshift_odd: OK\n";
    return true;
}

static bool test_costaper()
{
    Eigen::ArrayXd input(12), expect(12);
    input = 1.0;
    expect << 0.0, 0.25, 0.5, 0.75, 1.0, 1.0, 1.0, 1.0, 0.75, 0.5, 0.25, 0.0;
    cosTaper( 5, input );
    if ((input!=expect).all())
    {
	cout<<"test_costaper failed:\n";
	cout<<"Expected - Received\n";
	for ( int idx=0; idx<input.size(); idx++)
	    cout<< to_string(expect(idx)) <<" - "<< to_string(input(idx)) << "\n";
	    return false;
	}
    cout<<"test_costaper: OK\n";
    return true;
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
    if ( !test_costaper() )
	return 1;

    return 0;
}
