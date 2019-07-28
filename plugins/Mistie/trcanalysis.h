#ifndef trcanalysis_h
#define trcanalysis_h

class SeisTrc;

bool    computeMistie(const SeisTrc& trcA, const SeisTrc& trcB, float maxshift, float& zdiff, float& phasediff, float& ampdiff, float&  quality);

#endif
