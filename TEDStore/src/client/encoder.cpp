#include "encoder.hpp"
#include "openssl/rsa.h"
#include <sys/time.h>

extern Configure config;

struct timeval timestartEncoder;
struct timeval timeendEncoder;

void PRINT_BYTE_ARRAY_ENCODER(
    FILE* file, void* mem, uint32_t len)
{
    if (!mem || !len) {
        fprintf(file, "\n( null )\n");
        return;
    }
    uint8_t* array = (uint8_t*)mem;
    fprintf(file, "%u bytes:\n{\n", len);
    uint32_t i = 0;
    for (i = 0; i < len - 1; i++) {
        fprintf(file, "0x%x, ", array[i]);
        if (i % 8 == 7)
            fprintf(file, "\n");
    }
    fprintf(file, "0x%x ", array[i]);
    fprintf(file, "\n}\n");
}

Encoder::Encoder(Sender* senderObjTemp)
{
    inputMQ_ = new messageQueue<Data_t>;
    senderObj_ = senderObjTemp;
    cryptoObj_ = new CryptoPrimitive();
}

Encoder::~Encoder()
{
    if (cryptoObj_ != NULL) {
        delete cryptoObj_;
    }
    inputMQ_->~messageQueue();
    delete inputMQ_;
}

void Encoder::run()
{

#if SYSTEM_BREAK_DOWN == 1
    double encryptChunkContentTime = 0;
    double generateCipherChunkHashTime = 0;
    long diff;
    double second;
#endif
    bool JobDoneFlag = false;
    while (true) {

        Data_t tempChunk;
        if (inputMQ_->done_ && inputMQ_->isEmpty()) {
            JobDoneFlag = true;
        }
        if (extractMQFromKeyClient(tempChunk)) {
            if (tempChunk.dataType == DATA_TYPE_RECIPE) {
                insertMQToSender(tempChunk);
                continue;
            } else {
#if SYSTEM_BREAK_DOWN == 1
                gettimeofday(&timestartEncoder, NULL);
#endif
                u_char ciphertext[tempChunk.chunk.logicDataSize];
                bool encryptChunkContentStatus = cryptoObj_->encryptWithKey(tempChunk.chunk.logicData, tempChunk.chunk.logicDataSize, tempChunk.chunk.encryptKey, ciphertext);
#if SYSTEM_BREAK_DOWN == 1
                gettimeofday(&timeendEncoder, NULL);
                diff = 1000000 * (timeendEncoder.tv_sec - timestartEncoder.tv_sec) + timeendEncoder.tv_usec - timestartEncoder.tv_usec;
                second = diff / 1000000.0;
                encryptChunkContentTime += second;
#endif
                if (!encryptChunkContentStatus) {
                    cerr << "Encoder : cryptoPrimitive error, encrypt chunk logic data error" << endl;
                    return;
                } else {
                    memcpy(tempChunk.chunk.logicData, ciphertext, tempChunk.chunk.logicDataSize);
#if SYSTEM_BREAK_DOWN == 1
                    gettimeofday(&timestartEncoder, NULL);
#endif
                    bool generateCipherChunkHashStatus = cryptoObj_->generateHash(tempChunk.chunk.logicData, tempChunk.chunk.logicDataSize, tempChunk.chunk.chunkHash);
#if SYSTEM_BREAK_DOWN == 1
                    gettimeofday(&timeendEncoder, NULL);
                    diff = 1000000 * (timeendEncoder.tv_sec - timestartEncoder.tv_sec) + timeendEncoder.tv_usec - timestartEncoder.tv_usec;
                    second = diff / 1000000.0;
                    generateCipherChunkHashTime += second;
#endif
                    if (generateCipherChunkHashStatus) {
                        insertMQToSender(tempChunk);
                    } else {
                        cerr << "Encoder : generate cipher chunk hash error, exiting" << endl;
                        return;
                    }
                }
            }
        }
        if (JobDoneFlag) {
            if (!senderObj_->editJobDoneFlag()) {
                cerr << "Encoder : error to set job done flag for encoder" << endl;
            } else {
#if SYSTEM_BREAK_DOWN == 1
                cerr << "Encoder : encode chunk thread job done, exit now" << endl;
#endif
            }
            break;
        }
    }
#if SYSTEM_BREAK_DOWN == 1
    cout << "Encoder : chunk content encryption work time = " << encryptChunkContentTime << " s" << endl;
    cout << "Encoder : cipher chunk crypto hash generate work time = " << generateCipherChunkHashTime << " s" << endl;
#endif
    return;
}

bool Encoder::insertMQFromKeyClient(Data_t& newChunk)
{
    return inputMQ_->push(newChunk);
}

bool Encoder::extractMQFromKeyClient(Data_t& newChunk)
{
    return inputMQ_->pop(newChunk);
}

bool Encoder::insertMQToSender(Data_t& newChunk)
{
    return senderObj_->insertMQFromEncoder(newChunk);
}

bool Encoder::editJobDoneFlag()
{
    inputMQ_->done_ = true;
    if (inputMQ_->done_) {
        return true;
    } else {
        return false;
    }
}