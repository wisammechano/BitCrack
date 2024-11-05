#include "CudaSearchDevice.h"
#include "Logger.h"
#include "util.h"
#include "cudabridge.h"
#include "AddressUtil.h"

void CudaSearchDevice::cudaCall(cudaError_t err)
{
    if(err) {
        std::string errStr = cudaGetErrorString(err);

        throw KeySearchException(errStr);
    }
}

CudaSearchDevice::CudaSearchDevice(int device, int threads, int pointsPerThread, int blocks)
{
    cuda::CudaDeviceInfo info;
    try {
        info = cuda::getDeviceInfo(device);
        _deviceName = info.name;
    } catch(cuda::CudaException ex) {
        throw KeySearchException(ex.msg);
    }

    if(threads <= 0 || threads % 32 != 0) {
        throw KeySearchException("The number of threads must be a multiple of 32");
    }

    if(pointsPerThread <= 0) {
        throw KeySearchException("At least 1 point per thread required");
    }

    // Specifying blocks on the commandline is depcreated but still supported. If there is no value for
    // blocks, devide the threads evenly among the multi-processors
    if(blocks == 0) {
        if(threads % info.mpCount != 0) {
            throw KeySearchException("The number of threads must be a multiple of " + util::format("%d", info.mpCount));
        }

        _threads = threads / info.mpCount;

        _blocks = info.mpCount;

        while(_threads > 512) {
            _threads /= 2;
            _blocks *= 2;
        }
    } else {
        _threads = threads;
        _blocks = blocks;
    }

    _iterations = 0;

    _device = device;

    _pointsPerThread = pointsPerThread;
}

void CudaSearchDevice::setTargets(const std::set<KeySearchTarget> &targets)
{
    _targets.clear();
    
    for(std::set<KeySearchTarget>::iterator i = targets.begin(); i != targets.end(); ++i) {
        hash160 h(i->value);
        _targets.push_back(h);
    }

    cudaCall(_targetLookup.setTargets(_targets));
}


uint64_t CudaSearchDevice::keysPerStep()
{
    return (uint64_t)_blocks * _threads * _pointsPerThread;
}

std::string CudaSearchDevice::getDeviceName()
{
    return _deviceName;
}

void CudaSearchDevice::getMemoryInfo(uint64_t &freeMem, uint64_t &totalMem)
{
    cudaCall(cudaMemGetInfo(&freeMem, &totalMem));
}

void CudaSearchDevice::removeTargetFromList(const unsigned int hash[5])
{
    size_t count = _targets.size();

    while(count) {
        if(memcmp(hash, _targets[count - 1].h, 20) == 0) {
            _targets.erase(_targets.begin() + count - 1);
            return;
        }
        count--;
    }
}

bool CudaSearchDevice::isTargetInList(const unsigned int hash[5])
{
    size_t count = _targets.size();

    while(count) {
        if(memcmp(hash, _targets[count - 1].h, 20) == 0) {
            return true;
        }
        count--;
    }

    return false;
}

// Verify a private key produces the public key and hash
bool CudaSearchDevice::verifyKey(const secp256k1::uint256 &privateKey, const secp256k1::ecpoint &publicKey, const unsigned int hash[5], bool compressed)
{
    secp256k1::ecpoint g = secp256k1::G();

    secp256k1::ecpoint p = secp256k1::multiplyPoint(privateKey, g);

    if(!(p == publicKey)) {
        return false;
    }

    unsigned int xWords[8];
    unsigned int yWords[8];

    p.x.exportWords(xWords, 8, secp256k1::uint256::BigEndian);
    p.y.exportWords(yWords, 8, secp256k1::uint256::BigEndian);

    unsigned int digest[5];
    if(compressed) {
        Hash::hashPublicKeyCompressed(xWords, yWords, digest);
    } else {
        Hash::hashPublicKey(xWords, yWords, digest);
    }

    for(int i = 0; i < 5; i++) {
        if(digest[i] != hash[i]) {
            return false;
        }
    }

    return true;
}

uint32_t CudaSearchDevice::getPrivateKeyOffset(int thread, int block, int idx)
{
    // Total number of threads
    int totalThreads = _blocks * _threads;

    int base = idx * totalThreads;

    // Global ID of the current thread
    int threadId = block * _threads + thread;

    return base + threadId;
}

size_t CudaSearchDevice::getResults(std::vector<KeySearchResult> &resultsOut)
{
    for(int i = 0; i < _results.size(); i++) {
        resultsOut.push_back(_results[i]);
    }
    _results.clear();

    return resultsOut.size();
}
