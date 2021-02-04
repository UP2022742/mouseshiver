// Do the shake requirements. Needs cross platform varients...
#include <Windows.h>
#include <iostream>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>

// Do the listen requirements.
#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
using namespace std;

#define PA_SAMPLE_TYPE  paFloat32
#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (512)

// How long you want the buffer duration to last.
const float BUFFER_DURATION = 0.5f;
const int NUM_CHANNELS = 2;

const float SAMPLE_SILENCE = 0.f;
const float SENSITIVITY_VALUE = 10.f;

typedef struct
{
    int         frameIndex;
    int         maxFrameIndex;
    float*     recordedSamples;
}
paTestData;

static int recordCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    paTestData* data = (paTestData*)userData;
    const float* rptr = (const float*)inputBuffer;
    float* wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    long framesToCalc;
    long i;
    int finished;
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

    (void)outputBuffer; /* Prevent unused variable warnings. */
    (void)timeInfo;
    (void)statusFlags;
    (void)userData;

    if (framesLeft < framesPerBuffer)
    {
        framesToCalc = framesLeft;
        data->maxFrameIndex = (int)(BUFFER_DURATION * SAMPLE_RATE);
        data->frameIndex = 0;
    }
    else
    {
        framesToCalc = framesPerBuffer;
        
    }
    finished = paContinue;

    if (inputBuffer == NULL)
    {
        for (i = 0; i < framesToCalc; i++)
        {
            *wptr++ = SAMPLE_SILENCE;  /* left */
            if (NUM_CHANNELS == 2) *wptr++ = SAMPLE_SILENCE;  /* right */
        }
    }
    else
    {
        for (i = 0; i < framesToCalc; i++)
        {
            *wptr++ = *rptr++;  /* left */
            if (NUM_CHANNELS == 2) *wptr++ = *rptr++;  /* right */
        }
    }
    data->frameIndex += framesToCalc;
    return finished;
}

uint64_t timeSinceEpochMillisec() {
    using namespace chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void doTheShake(float input_volume)
{
    srand(timeSinceEpochMillisec());
    int new_x_operator;
    int new_y_operator;
    int define_x_operator;
    int define_y_operator;

    // Create operator randomisation.
    define_x_operator = rand() % 2 + 1;
    if (define_x_operator == 1) { new_x_operator = 1; }
    else { new_x_operator = -1; }
    define_y_operator = rand() % 2 + 1;
    if (define_y_operator == 1) { new_y_operator = 1; }
    else { new_y_operator = -1; }

    // Create input type.
    INPUT Input = { 0 };
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_MOVE;
    Input.mi.dx = new_x_operator * SENSITIVITY_VALUE * (input_volume * (65535.0f / GetSystemMetrics(SM_CXSCREEN) - 1));;
    Input.mi.dy = new_y_operator * SENSITIVITY_VALUE * (input_volume * (65535.0f / GetSystemMetrics(SM_CYSCREEN) - 1));

    // Send strokes from the mouse.
    ::SendInput(1, &Input, sizeof(INPUT));
    ZeroMemory(&Input, sizeof(Input));
}

int main(void)
{
    PaStreamParameters  inputParameters{};
    PaStream*           stream;
    PaError             err = paNoError;
    paTestData          data{};
    int                 i, totalFrames, numSamples, numBytes;
    float               max, val;

    data.maxFrameIndex = totalFrames = (int)(BUFFER_DURATION * SAMPLE_RATE);
    data.frameIndex = 0;
    numSamples = totalFrames * NUM_CHANNELS;
    numBytes = numSamples * sizeof(float);
    data.recordedSamples = (float*)malloc(numBytes);
    if (data.recordedSamples == NULL)
    {
        printf("Could not allocate record array.\n");
        goto done;
    }
    for (i = 0; i < numSamples; i++) data.recordedSamples[i] = 0;

    err = Pa_Initialize();
    if (err != paNoError) goto done;

    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default input device.\n");
        goto done;
    }
    inputParameters.channelCount = 2;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(&stream,&inputParameters,NULL,SAMPLE_RATE,FRAMES_PER_BUFFER,paClipOff,recordCallback,&data);
    if (err != paNoError) goto done;

    err = Pa_StartStream(stream);
    if (err != paNoError) goto done;

    printf("\n=== Start screaming... ===\n"); fflush(stdout);

    while ((err = Pa_IsStreamActive(stream)) == 1)
    {
        Pa_Sleep(20);
        /* Measure maximum peak amplitude. */
        max = 0;
        for (i = 0; i < numSamples; i++)
        {
            val = data.recordedSamples[i];
            if (val < 0) val = -val; /* ABS */
            if (val > max)
            {
                max = val;
            }
        }
        doTheShake(max);
    }
    if (err < 0) goto done;

    err = Pa_CloseStream(stream);
    if (err != paNoError) goto done;

done:
    Pa_Terminate();
    if (data.recordedSamples)       /* Sure it is NULL or valid. */
        free(data.recordedSamples);
    if (err != paNoError)
    {
        fprintf(stderr, "An error occured while using the portaudio stream\n");
        fprintf(stderr, "Error number: %d\n", err);
        fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
        err = 1;          /* Always return 0 or 1, but no other return codes. */
    }
    return err;
}