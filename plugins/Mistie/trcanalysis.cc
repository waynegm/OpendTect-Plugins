#include "trcanalysis.h"

#include "seistrc.h"
#include "survinfo.h"

#include "eigen3/unsupported/Eigen/FFT"

bool computeMistie( const SeisTrc& trcA, const SeisTrc& trcB, float maxshift, float& zdiff, float& phasediff, float& ampdiff, float&  quality)
{
    Eigen::FFT<double> fft;
    const int n = trcA.size();
    int nfft = 2*n;
    float dt = trcA.info().sampling_.step_;

    Eigen::VectorXd A, B;
    A.setZero(nfft);
    B.setZero(nfft);
    for (int idt=0; idt<n; idt++) {
	const float val1 = trcA.get(idt,0);
	const float val2 = trcB.get(idt,0);
	const float valA = !Math::IsNormalNumber(val1) || mIsUdf(val1) ? 0.0 : val1;
        const float valB = !Math::IsNormalNumber(val2) || mIsUdf(val2) ? 0.0 : val2;
        A(idt) = valA;
        B(idt) = valB;
    }
    double sumAA = A.head(n).norm();
    double sumBB = B.head(n).norm();
    if (mIsZero(sumAA, mDefEps) || mIsZero(sumBB, mDefEps)) {
        zdiff = 0.0;
        phasediff = 0.0;
        ampdiff = 1.0;
        quality = 0.0;
        return false;
    }
    ampdiff = sumBB/sumAA;

    Eigen::VectorXcd Af, Bf, CCf, AS;
    Eigen::VectorXd CC, H;
    H.setConstant(nfft, 2.0);
    H(0) = 1.0;
    H(n) = 1.0;
    H.segment(n+1, n-1) *= 0.0;
    fft.fwd(Af,A);
    fft.fwd(Bf,B);
    CCf = (Bf.array() * Eigen::conj(Af.array()) * H.array()).matrix();
    fft.inv(AS,CCf);
    CC = Eigen::abs2(AS.array()).sqrt().matrix();
    Eigen::MatrixXd::Index maxIndex;
    int maxlag = maxshift/dt;
    CC.segment(maxlag,nfft-2*maxlag) *= 0.0;
    CC.maxCoeff(&maxIndex);
    int il = maxIndex-1;
    int ir = maxIndex+1;
    if (maxIndex==0)
        il = nfft-1;
    else if (maxIndex==nfft-1)
        ir = 0;
    float cp = (CC(il)-CC(ir))/(2.0*CC(il)-4.0*CC(maxIndex)+2.0*CC(ir));
    quality = (CC(maxIndex) - 0.25 * (CC(il) - CC(ir)) * cp)/(sumAA*sumBB);
    quality = quality > 1.0 ? 1.0 : quality;
    zdiff = (maxIndex < n ? float(maxIndex)+cp : float(maxIndex-nfft)+cp)*dt*SI().showZ2UserFactor();;
    float p0 = std::arg(AS(maxIndex));
    float p1 = cp>=0.0 ? std::arg(AS(ir)) : std::arg(AS(il));
    p1 = p1-p0>M_PI ? p1-M_2PI : p1-p0<-M_PI ? p1+M_2PI : p1;
    phasediff = Math::toDegrees(cp>=0.0 ? (p1-p0)*cp+p0 :(p0-p1)*cp+p0) ;
    return true;
}

bool computeMistie( const SeisTrc& trcA, const SeisTrc& trcB, float maxshift, float& zdiff, float&  quality)
{
    Eigen::FFT<double> fft;
    const int n = trcA.size();
    int nfft = 2*n;
    float dt = trcA.info().sampling_.step_;

    Eigen::VectorXd A, B;
    A.setZero(nfft);
    B.setZero(nfft);
    for (int idt=0; idt<n; idt++) {
	float valA = mIsUdf(trcA.get(idt,0)) ? 0.0 : trcA.get(idt,0);
	float valB = mIsUdf(trcB.get(idt,0)) ? 0.0 : trcB.get(idt,0);
	A(idt) = valA;
	B(idt) = valB;
    }
    double sumAA = A.head(n).norm();
    double sumBB = B.head(n).norm();
    if (mIsZero(sumAA, mDefEps) || mIsZero(sumBB, mDefEps)) {
	zdiff = 0.0;
	quality = 0.0;
	return false;
    }

    Eigen::VectorXcd Af, Bf, CCf, AS;
    Eigen::VectorXd CC, H;
    fft.fwd(Af,A);
    fft.fwd(Bf,B);
    CCf = (Bf.array() * Eigen::conj(Af.array())).matrix();
    fft.inv(CC,CCf);
    Eigen::MatrixXd::Index maxIndex;
    int maxlag = maxshift/dt;
    CC.segment(maxlag,nfft-2*maxlag) *= 0.0;
    CC.maxCoeff(&maxIndex);
    int il = maxIndex-1;
    int ir = maxIndex+1;
    if (maxIndex==0)
	il = nfft-1;
    else if (maxIndex==nfft-1)
	ir = 0;
    float cp = (CC(il)-CC(ir))/(2.0*CC(il)-4.0*CC(maxIndex)+2.0*CC(ir));
    quality = (CC(maxIndex) - 0.25 * (CC(il) - CC(ir)) * cp)/(sumAA*sumBB);
    quality = quality > 1.0 ? 1.0 : quality;
    zdiff = (maxIndex < n ? float(maxIndex)+cp : float(maxIndex-nfft)+cp)*dt*SI().showZ2UserFactor();;
    return true;
}
