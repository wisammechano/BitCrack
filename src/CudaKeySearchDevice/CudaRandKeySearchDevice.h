#ifndef _CUDA_RAND_KEY_SEARCH_DEVICE
#define _CUDA_RAND_KEY_SEARCH_DEVICE

#include "CudaSearchDevice.h"


class CudaRandKeySearchDevice : public CudaSearchDevice {

private:
    void generateStartingPoints() final;

    void getResultsInternal() final;

    uint32_t getPrivateKeyOffset(int thread, int block, int point) final;

    secp256k1::uint256 _startExponent;


public:
    using CudaSearchDevice::CudaSearchDevice;

    void init(const secp256k1::uint256 &start, int compression, const secp256k1::uint256 &stride) final;

    void doStep() final;

    secp256k1::uint256 getNextKey() final;
};

#endif