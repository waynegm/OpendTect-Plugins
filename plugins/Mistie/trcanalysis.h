#pragma once

class SeisTrc;

bool    computeMistie(const SeisTrc& trcA, const SeisTrc& trcB, float maxshift, float& zdiff, float& phasediff, float& ampdiff, float&  quality);
bool    computeMistie(const SeisTrc& trcA, const SeisTrc& trcB, float maxshift, float& zdiff, float&  quality);
