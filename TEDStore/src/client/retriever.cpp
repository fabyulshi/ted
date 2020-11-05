#include "retriever.hpp"

struct timeval timestartRetriever;
struct timeval timeendRetriever;
struct timeval timestartRetriever_thread;
struct timeval timeendRetriever_thread;

Retriever::Retriever(string fileName, RecvDecode*& recvDecodeObjTemp)
{
    recvDecodeObj_ = recvDecodeObjTemp;
    string newFileName = fileName.append(".d");
    retrieveFile_.open(newFileName, ofstream::out | ofstream::binary);
    Recipe_t tempRecipe = recvDecodeObj_->getFileRecipeHead();
    totalChunkNumber_ = tempRecipe.fileRecipeHead.totalChunkNumber;
}

Retriever::~Retriever()
{
    retrieveFile_.close();
}

void Retriever::run()
{
#if SYSTEM_BREAK_DOWN == 1
    long diff;
    double second;
    double writeFileTime = 0;
    gettimeofday(&timestartRetriever_thread, NULL);
#endif
    RetrieverData_t newData;
    uint32_t totalRecvNumber_ = 0;
    while (totalRecvNumber_ < totalChunkNumber_) {
        if (recvDecodeObj_->extractMQ(newData)) {
#if SYSTEM_BREAK_DOWN == 1
            gettimeofday(&timestartRetriever, NULL);
#endif
            retrieveFile_.write(newData.logicData, newData.logicDataSize);
            totalRecvNumber_++;
#if SYSTEM_BREAK_DOWN == 1
            gettimeofday(&timeendRetriever, NULL);
            diff = 1000000 * (timeendRetriever.tv_sec - timestartRetriever.tv_sec) + timeendRetriever.tv_usec - timestartRetriever.tv_usec;
            second = diff / 1000000.0;
            writeFileTime += second;
#endif
        }
    }
#if SYSTEM_BREAK_DOWN == 1
    while (!recvDecodeObj_->getJobDoneFlag())
        ;
    cout << "Retriever : write file time = " << writeFileTime << " s" << endl;
    gettimeofday(&timeendRetriever_thread, NULL);
    diff = 1000000 * (timeendRetriever_thread.tv_sec - timestartRetriever_thread.tv_sec) + timeendRetriever_thread.tv_usec - timestartRetriever_thread.tv_usec;
    second = diff / 1000000.0;
    cout << "Retriever : thread work time = " << second << " s" << endl;
#endif
    return;
}