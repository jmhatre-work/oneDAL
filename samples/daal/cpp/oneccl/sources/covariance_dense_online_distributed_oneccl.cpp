/* file: covariance_dense_online_distributed_oneccl.cpp */
/*******************************************************************************
* Copyright 2020 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/*
!  Content:
!    C++ sample of dense variance-covariance matrix computation in the
!    distributed processing mode
!
!******************************************************************************/

/**
 * <a name="DAAL-SAMPLE-CPP-COVARIANCE_DENSE_ONLINE_DISTRIBUTED"></a>
 * \example covariance_dense_online_distributed_oneccl.cpp
 */

#include "daal_sycl.h"
#include "service_sycl.h"
#include "oneapi/ccl.hpp"
#include "mpi.h"
#include "stdio.h"
#include <memory>

using namespace std;
using namespace daal;
using namespace daal::algorithms;

typedef services::SharedPtr<FileDataSource<CSVFeatureManager> > FileDataSourcePtr;

/* Covariance algorithm parameters */
const size_t nProcs = 4;
const size_t nVectorsInBlock = 25;

/* Input data set parameters */
const string dataFileNames[4] = { "./data/covcormoments_dense_1.csv",
                                  "./data/covcormoments_dense_2.csv",
                                  "./data/covcormoments_dense_3.csv",
                                  "./data/covcormoments_dense_4.csv" };

#define ccl_root 0

int getLocalRank(ccl::communicator &comm, int size, int rank) {
    /* Obtain local rank among nodes sharing the same host name */
    char zero = static_cast<char>(0);
    std::vector<char> name(MPI_MAX_PROCESSOR_NAME + 1, zero);
    int resultlen = 0;
    MPI_Get_processor_name(name.data(), &resultlen);
    std::string str(name.begin(), name.end());
    std::vector<char> allNames((MPI_MAX_PROCESSOR_NAME + 1) * size, zero);
    std::vector<size_t> aReceiveCount(size, MPI_MAX_PROCESSOR_NAME + 1);
    ccl::allgatherv((int8_t *)name.data(),
                    name.size(),
                    (int8_t *)allNames.data(),
                    aReceiveCount,
                    comm)
        .wait();
    int localRank = 0;
    for (int i = 0; i < rank; i++) {
        auto nameBegin = allNames.begin() + i * (MPI_MAX_PROCESSOR_NAME + 1);
        std::string nbrName(nameBegin, nameBegin + (MPI_MAX_PROCESSOR_NAME + 1));
        if (nbrName == str)
            localRank++;
    }
    return localRank;
}

FileDataSourcePtr loadData(int rankId) {
    /* Initialize FileDataSource<CSVFeatureManager> to retrieve the input data
   * from a .csv file */
    auto data = SyclHomogenNumericTable<>::create(10, 0, NumericTable::notAllocate);

    return FileDataSourcePtr(
        new FileDataSource<CSVFeatureManager>(dataFileNames[rankId],
                                              DataSource::doAllocateNumericTable,
                                              DataSource::doDictionaryFromContext));
}

NumericTablePtr init(int rankId, const NumericTablePtr &pData, ccl::communicator &comm);
NumericTablePtr compute(int rankId,
                        const NumericTablePtr &pData,
                        const NumericTablePtr &initialCentroids,
                        ccl::communicator &comm);

int main(int argc, char *argv[]) {
    /* Initialize oneCCL */
    ccl::init();

    MPI_Init(NULL, NULL);
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    ccl::shared_ptr_class<ccl::kvs> kvs;
    ccl::kvs::address_type main_addr;
    if (rank == 0) {
        kvs = ccl::create_main_kvs();
        main_addr = kvs->get_address();
        MPI_Bcast((void *)main_addr.data(), main_addr.size(), MPI_BYTE, 0, MPI_COMM_WORLD);
    }
    else {
        MPI_Bcast((void *)main_addr.data(), main_addr.size(), MPI_BYTE, 0, MPI_COMM_WORLD);
        kvs = ccl::create_kvs(main_addr);
    }

    auto comm = ccl::create_communicator(size, rank, kvs);

    /* Create GPU device from local rank and set execution context */
    auto local_rank = getLocalRank(comm, size, rank);
    auto gpus = get_gpus();
    auto rank_gpu = gpus[local_rank % gpus.size()];
    sycl::queue queue(rank_gpu);
    daal::services::SyclExecutionContext ctx(queue);
    services::Environment::getInstance()->setDefaultExecutionContext(ctx);

    const bool isRoot = (rank == ccl_root);

    /* Retrieve the input data */
    auto pData = loadData(rank);

    covariance::Distributed<step1Local> localAlgorithm;

    while (pData->loadDataBlock(nVectorsInBlock) == nVectorsInBlock) {
        /* Set input objects for the algorithm */
        localAlgorithm.input.set(covariance::data, pData->getNumericTable());

        /* Compute partial estimates */
        localAlgorithm.compute();
    }

    /* Serialize partial results required by step 2 */
    InputDataArchive dataArch;
    localAlgorithm.getPartialResult()->serialize(dataArch);
    const uint64_t perNodeArchLength = (size_t)dataArch.getSizeOfArchive();

    std::vector<uint64_t> aPerNodeArchLength(comm.size());
    std::vector<size_t> aReceiveCount(comm.size(), 1);
    /* Transfer archive length to the step 2 on the root node */
    ccl::allgatherv(&perNodeArchLength, 1, aPerNodeArchLength.data(), aReceiveCount, comm).wait();

    ByteBuffer serializedData;
    /* Calculate total archive length */
    int totalArchLength = 0;

    for (size_t i = 0; i < nProcs; ++i) {
        totalArchLength += aPerNodeArchLength[i];
    }
    aReceiveCount[ccl_root] = totalArchLength;

    serializedData.resize(totalArchLength);

    ByteBuffer nodeResults(perNodeArchLength);
    dataArch.copyArchiveToArray(&nodeResults[0], perNodeArchLength);

    /* Transfer partial results to step 2 on the root node */
    ccl::allgatherv((int8_t *)&nodeResults[0],
                    perNodeArchLength,
                    (int8_t *)&serializedData[0],
                    aPerNodeArchLength,
                    comm)
        .wait();

    if (isRoot) {
        /* Create an algorithm to compute covariance on the master node */
        covariance::Distributed<step2Master> masterAlgorithm;
        for (size_t i = 0, shift = 0; i < nProcs; shift += aPerNodeArchLength[i], ++i) {
            /* Deserialize partial results from step 1 */
            OutputDataArchive dataArch(&serializedData[shift], aPerNodeArchLength[i]);

            covariance::PartialResultPtr dataForStep2FromStep1(new covariance::PartialResult());
            dataForStep2FromStep1->deserialize(dataArch);

            /* Set local partial results as input for the master-node algorithm */
            masterAlgorithm.input.add(covariance::partialResults, dataForStep2FromStep1);
        }

        /* Merge and finalizeCompute covariance on the master node */
        masterAlgorithm.compute();
        masterAlgorithm.finalizeCompute();

        /* Retrieve the algorithm results */
        covariance::ResultPtr result = masterAlgorithm.getResult();

        /* Print the results */
        printNumericTable(result->get(covariance::covariance), "Covariance matrix:");
        printNumericTable(result->get(covariance::mean), "Mean vector:");
    }

    MPI_Finalize();
    return 0;
}
