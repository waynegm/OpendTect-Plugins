#include "efd.h"

#include "moddepmgr.h"
#include "plugins.h"
#include "testprog.h"

#include <iostream>

class EFD_test : public EFD
{
public:
    EFD_test() : EFD() {}

    bool test_computeFreqSegments()
    {
	Eigen::ArrayXi segbounds(6);
	Eigen::ArrayXd centralfreqs(5);

	inputfreq_ << ;
	segbounds << ;
	computeFreqSegments();
	return (segbounds==seqbounds_).all() && (centralfreqs==centralfreqs_).all();
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

    return 0;
}
