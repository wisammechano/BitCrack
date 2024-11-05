// the random searcher will go randomly on first iteration then incrementally
#ifndef _CUDA_RAND_KEY_SEARCH_DEVICE
#define _CUDA_RAND_KEY_SEARCH_DEVICE

#include <vector>
#include "CudaSearchDevice.h"


class CudaRandSearchDevice : public CudaSearchDevice {

private:
    void generateStartingPoints() final;

    void getResultsInternal() final;

    secp256k1::uint256 _startExponent;
    secp256k1::uint256 _endExponent;
    std::vector<secp256k1::uint256> _seedExponents;

public:
    using CudaSearchDevice::CudaSearchDevice;

    void init(const secp256k1::uint256 &start, const secp256k1::uint256 &end, int compression, const secp256k1::uint256 &stride) final;

    void doStep() final;

    secp256k1::uint256 getNextKey() final;
};

#endif