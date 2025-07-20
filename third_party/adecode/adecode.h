#ifndef ADECODE_ADECODE_H_
#define ADECODE_ADECODE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int(AudioDecoderReadFunc)(void* data, void* buffer, unsigned int size);

typedef struct _AudioDecoder AudioDecoder;

unsigned int AudioDecoder_Read(AudioDecoder* ad, void* buffer, unsigned int qty);
void AudioDecoder_Close(AudioDecoder* ad);
AudioDecoder* Create_AudioDecoder(AudioDecoderReadFunc* reader, void* data, int* channels, int* sampleRate, int* sampleCount);

#ifdef __cplusplus
}
#endif

#endif /* ADECODE_ADECODE_H_ */
