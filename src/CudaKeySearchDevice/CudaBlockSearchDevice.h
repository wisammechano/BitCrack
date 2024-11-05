#ifndef _CUDA_BLOCK_KEY_SEARCH_DEVICE
#define _CUDA_BLOCK_KEY_SEARCH_DEVICE

#include "CudaSearchDevice.h"


class CudaBlockSearchDevice : public CudaSearchDevice {

private:
    void generateStartingPoints() final;

    void getResultsInternal() final;

    secp256k1::uint256 _startExponent;
    secp256k1::uint256 _endExponent;
    secp256k1::uint256 _perIter;
    secp256k1::uint256 _gridStride;



public:
    using CudaSearchDevice::CudaSearchDevice;

    void init(const secp256k1::uint256 &start, const secp256k1::uint256 &end, int compression, const secp256k1::uint256 &stride) final;

    void doStep() final;

    secp256k1::uint256 getNextKey() final;
};

#endif