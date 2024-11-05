#include "CudaRandKeySearchDevice.h"
#include "Logger.h"
#include "secp256k1.h"
#include "util.h"
#include "cudabridge.h"
#include "AddressUtil.h"

static secp256k1::uint256 _ONE(1);

void CudaRandKeySearchDevice::init(const secp256k1::uint256 &start, const secp256k1::uint256 &end, int compression, const secp256k1::uint256 &stride)
{
    if(start.cmp(secp256k1::N) >= 0) {
        throw KeySearchException("Starting key is out of range");
    }

    _startExponent = start;
    _endExponent = end;

    secp256k1::uint256 totalKeys(_endExponent - _startExponent + _ONE);
    
    
    _compression = compression;

    _stride = stride;

    // Distribute total points over the keyspace
    _perIter = secp256k1::uint256((uint64_t)_blocks * _threads * _pointsPerThread);

    // total number of iterations we need to cover the keyspace
    secp256k1::uint256 totalIters(totalKeys.div(_perIter));

    // Add one iteration for reminder if any
    if(!totalKeys.mod(_perIter).isZero()) {
        totalIters = totalIters.add(_ONE);
    }

    // we are going to increase start by gridsize for each item
    // this effectively will make the process concurrent over keyspace and not serial
    _gridStride = _stride.mul(totalIters);

    cudaCall(cudaSetDevice(_device));

    // Block on kernel calls
    cudaCall(cudaSetDeviceFlags(cudaDeviceScheduleBlockingSync));

    // Use a larger portion of shared memory for L1 cache
    cudaCall(cudaDeviceSetCacheConfig(cudaFuncCachePreferL1));

    generateStartingPoints();

    //cudaCall(allocateChainBuf(_threads * _blocks * _pointsPerThread));

    // Set the incrementor
    secp256k1::ecpoint g = secp256k1::G();
    // the grid incrementor increases by stride only
    secp256k1::ecpoint p = secp256k1::multiplyPoint(_stride, g);

    cudaCall(_resultList.init(sizeof(CudaDeviceResult), 16));

    cudaCall(setIncrementorPoint(p.x, p.y));
}


void CudaRandKeySearchDevice::generateStartingPoints()
{
    uint64_t totalPoints = (uint64_t)_pointsPerThread * _threads * _blocks;
    uint64_t totalMemory = totalPoints * 40;

    std::vector<secp256k1::uint256> exponents;

    Logger::log(LogLevel::Info, "Generating " + util::formatThousands(totalPoints) + " starting points (" + util::format("%.1f", (double)totalMemory / (double)(1024 * 1024)) + "MB)");

    // Generate key pairs for k, k+gridStride, k+2gridStride ... k + <total points in parallel - 1>*gridStride
    secp256k1::uint256 privKey = _startExponent;

    exponents.push_back(privKey);

    for(uint64_t i = 1; i < totalPoints; i++) {
        privKey = privKey.add(_gridStride);
        exponents.push_back(privKey);
    }

    cudaCall(_deviceKeys.init(_blocks, _threads, _pointsPerThread, exponents));

    // Show progress in 10% increments
    double pct = 10.0;
    for(int i = 1; i <= 256; i++) {
        cudaCall(_deviceKeys.doStep());

        if(((double)i / 256.0) * 100.0 >= pct) {
            Logger::log(LogLevel::Info, util::format("%.1f%%", pct));
            pct += 10.0;
        }
    }

    Logger::log(LogLevel::Info, "Done");

    _deviceKeys.clearPrivateKeys();
}

void CudaRandKeySearchDevice::doStep()
{
    uint64_t numKeys = (uint64_t)_blocks * _threads * _pointsPerThread;

    try {
        if(_iterations < 2 && _startExponent.cmp(numKeys) <= 0) {
            callKeyFinderKernel(_blocks, _threads, _pointsPerThread, _deviceKeys.getXPoints(), _deviceKeys.getYPoints(), _deviceKeys.getChain(), true, _compression);
        } else {
            callKeyFinderKernel(_blocks, _threads, _pointsPerThread, _deviceKeys.getXPoints(), _deviceKeys.getYPoints(), _deviceKeys.getChain(), false, _compression);
        }
    } catch(cuda::CudaException ex) {
        throw KeySearchException(ex.msg);
    }

    getResultsInternal();

    _iterations++;
}

uint32_t CudaRandKeySearchDevice::getPrivateKeyOffset(int thread, int block, int idx)
{
    // Total number of threads
    int totalThreads = _blocks * _threads;

    int base = idx * totalThreads;

    // Global ID of the current thread
    int threadId = block * _threads + thread;

    return base + threadId;
}

void CudaRandKeySearchDevice::getResultsInternal()
{
    int count = _resultList.size();
    int actualCount = 0;
    if(count == 0) {
        return;
    }

    unsigned char *ptr = new unsigned char[count * sizeof(CudaDeviceResult)];

    _resultList.read(ptr, count);

    for(int i = 0; i < count; i++) {
        struct CudaDeviceResult *rPtr = &((struct CudaDeviceResult *)ptr)[i];

        // might be false-positive
        if(!isTargetInList(rPtr->digest)) {
            continue;
        }
        actualCount++;

        KeySearchResult minerResult;

        // Calculate the private key based on the number of iterations and the current thread
        // in each chain, k is startKey + n*gridStride where n is from 0 to numKeys
        // in each iteration it is pkey in chain + stride*itercount
        // combined it is startKey + (pkeyoffset *gridStride) + (stride * iterations)
        secp256k1::uint256 itemOffset(getPrivateKeyOffset(rPtr->thread, rPtr->block, rPtr->idx));

        secp256k1::uint256 offset = itemOffset.mul(_gridStride) + _stride.mul(_iterations);
        secp256k1::uint256 privateKey = secp256k1::addModN(_startExponent, offset);

        minerResult.privateKey = privateKey;
        minerResult.compressed = rPtr->compressed;

        memcpy(minerResult.hash, rPtr->digest, 20);

        minerResult.publicKey = secp256k1::ecpoint(secp256k1::uint256(rPtr->x, secp256k1::uint256::BigEndian), secp256k1::uint256(rPtr->y, secp256k1::uint256::BigEndian));

        removeTargetFromList(rPtr->digest);

        _results.push_back(minerResult);
    }

    delete[] ptr;

    _resultList.clear();

    // Reload the bloom filters
    if(actualCount) {
        cudaCall(_targetLookup.setTargets(_targets));
    }
}


secp256k1::uint256 CudaRandKeySearchDevice::getNextKey()
{

    // next iteration has largest key at last n
    // this may prevent doing a complete grid if the keyspace is not divisible by total iterations
    uint64_t totalPoints = (uint64_t)_pointsPerThread * _threads * _blocks;

    secp256k1::uint256 iterationOffset(_stride.mul(_iterations));
    secp256k1::uint256 lastOffset = _gridStride.mul(totalPoints - 1);

    return _startExponent + lastOffset + iterationOffset + _stride; 
}
