#include "efd.h"

#include "moddepmgr.h"
#include "plugins.h"
#include "testprog.h"

#include <iostream>

using namespace Eigen;
using namespace std;

class EFD_test : public EFD
{
public:
    EFD_test() : EFD(10) {}

    bool test_computeFreqSegments()
    {
	const int nfreq = 44;
	VectorXi segbounds(5);
	VectorXf centralfreqs(4);
	VectorXi rankidx(4);
	inputfreq_ = VectorXcf::Zero(nfreq);

	inputfreq_(seq(0,nfreq/2)) << 1.0,1.5,1.8,0.5,4.0,5.0,3.0,1.0,0.5,1.0,3.0,1.5,0.5,1.0,1.5,3.0,4.0,2.0,1.0,0.5,0.5,0.4,0.4;
	segbounds << 0, 3, 8, 12, 22;
	centralfreqs << 2.0, 5.0, 10.0, 16.0;
	centralfreqs = centralfreqs*2.0f*M_PI/nfreq;
	rankidx << 5, 16, 10, 2;
	computeFreqSegments();

	if ((segbounds.size()!=segbounds_.size()) || (segbounds.array()!=segbounds_.array()).all())
	{
	    cout<<"test_computeFreqSegments failed: Segment Bounds\n";
	    cout<<"Expected - Received\n";
	    for ( int idx=0; idx<max(segbounds.size(), segbounds_.size()); idx++)
		cout<< (idx<segbounds.size() ? to_string(segbounds(idx)) : " ")<<" - "<< (idx<segbounds_.size() ? to_string(segbounds_(idx)) : " ")<<"\n";
	    return false;
	}
	if ((rankidx.size()!=rankidx_.size()) || (rankidx.array()!=rankidx_.array()).all())
	{
	    cout<<"test_computeFreqSegments failed: Rank Index\n";
	    cout<<"Expected - Received\n";
	    for ( int idx=0; idx<max(rankidx.size(), rankidx_.size()); idx++)
		cout<< (idx<rankidx.size() ? to_string(rankidx(idx)) : " ")<<" - "<< (idx<rankidx_.size() ? to_string(rankidx_(idx)) : " ")<<"\n";
	    return false;
	}
	if ((centralfreqs.size()!=centralfreqs_.size()) || (abs(centralfreqs.array()-centralfreqs_.array())>=1e-7f).all())
	{
	    cout<<"test_computeFreqSegments failed: Central Frequencies\n";
	    cout<<"Expected - Received\n";
	    cout<<centralfreqs.size()<<" - "<<centralfreqs_.size()<<"\n";
	    for ( int idx=0; idx<max(centralfreqs.size(), centralfreqs_.size()); idx++)
		cout<< (idx<centralfreqs.size() ? to_string(centralfreqs(idx)) : " ")<<" - "<< (idx<centralfreqs_.size() ? to_string(centralfreqs_(idx)) : " ")<<"\n";
	    return false;
	}

	cout<<"test_computeFreqSegments: OK\n";
	return true;
    }

    bool test_validMode()
    {
	if (validMode(-2)!=false)
	{
	    cout<<"test_validMode failed: negative mode number should be invalid\n";
	    return false;
	}
	if (validMode(0)!=true)
	{
	    cout<<"test_validMode failed: mode number of 0 should be valid\n";
	    return false;
	}
	if (validMode(3)!=true)
	{
	    cout<<"test_validMode failed: mode number of 3 should be valid\n";
	    return false;
	}
	if (validMode(6)!=false)
	{
	    cout<<"test_validMode failed: mode number of 6 should be invalid\n";
	    return false;
	}

	cout<<"test_validMode: OK\n";
	return true;
    }

    bool test_signal()
    {
	const float T = 1.f;
	const float dt = 0.001;
	const int sz = 1001;
	ArrayXf t = ArrayXf::LinSpaced(sz, 0.f, T);

	ArrayXf f1 = 1.f * cos(900.f*M_PI*t);
	ArrayXf f2 = 2.f*cos(8.f*M_PI*t);
	ArrayXf f3 = 5.f*cos(40.f*M_PI*t);
	ArrayXf input = f1 + f2 + f3;

	setMaxModes(3);
	setInput(input);
	cout<<centralfreqs_<<"\n";
	return false;
    }
};

int mTestMainFnName( int argc, char** argv )
{
    mInitTestProg();
    PIM().loadAuto( false );
    OD::ModDeps().ensureLoaded( "EFDAttrib" );
    PIM().loadAuto( true );

    EFD_test efdtester;
    if ( !efdtester.test_computeFreqSegments() )
	return 1;
    if ( !efdtester.test_validMode() )
	return 1;
    if ( !efdtester.test_signal() )
	return 1;

    return 0;
}
