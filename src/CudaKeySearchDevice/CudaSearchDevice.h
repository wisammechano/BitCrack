#ifndef _CUDA_SEARCH_DEVICE
#define _CUDA_SEARCH_DEVICE

#include "KeySearchDevice.h"
#include <vector>
#include <cuda_runtime.h>
#include "secp256k1.h"
#include "CudaDeviceKeys.h"
#include "CudaHashLookup.h"
#include "CudaAtomicList.h"
#include "cudaUtil.h"
#include "CudaDeviceResult.h"

class CudaSearchDevice : public KeySearchDevice {

protected:

    int _device;

    int _blocks;

    int _threads;

    int _pointsPerThread;

    int _compression;

    std::vector<KeySearchResult> _results;

    std::string _deviceName;

    uint64_t _iterations;

    void cudaCall(cudaError_t err);

    virtual void generateStartingPoints() = 0;

    virtual uint32_t getPrivateKeyOffset(int thread, int block, int idx);

    CudaDeviceKeys _deviceKeys;

    CudaAtomicList _resultList;

    CudaHashLookup _targetLookup;

    virtual void getResultsInternal() = 0;

    std::vector<hash160> _targets;

    bool isTargetInList(const unsigned int hash[5]);
    
    void removeTargetFromList(const unsigned int hash[5]);

    secp256k1::uint256 _stride;

    bool verifyKey(const secp256k1::uint256 &privateKey, const secp256k1::ecpoint &publicKey, const unsigned int hash[5], bool compressed);

public:

    CudaSearchDevice(int device, int threads, int pointsPerThread, int blocks = 0);

    void setTargets(const std::set<KeySearchTarget> &targets) final;

    size_t getResults(std::vector<KeySearchResult> &results) final;

    uint64_t keysPerStep() override;

    std::string getDeviceName() override;

    void getMemoryInfo(uint64_t &freeMem, uint64_t &totalMem) final ;

};

#endif