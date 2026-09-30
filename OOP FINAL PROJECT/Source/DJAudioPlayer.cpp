/*
  ==============================================================================
    DJAudioPlayer.cpp
  ==============================================================================
*/

#include "DJAudioPlayer.h"
#include <algorithm>
#include <vector>

DJAudioPlayer::DJAudioPlayer(AudioFormatManager& _formatManager)
    : formatManager(_formatManager)
{
}

DJAudioPlayer::~DJAudioPlayer()
{
}

void DJAudioPlayer::prepareToPlay(int samplesPerBlockExpected, double _sampleRate)
{
    sampleRate = _sampleRate;
    blockSize = samplesPerBlockExpected;

    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

    // this to prepare the DSP filter chain
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (uint32)samplesPerBlockExpected;
    spec.numChannels = 2;
    filterChain.prepare(spec);

    updateEQ();
}

void DJAudioPlayer::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    resampleSource.getNextAudioBlock(bufferToFill);

    // this apply EQ filters 
    if (bufferToFill.buffer != nullptr)
    {
        dsp::AudioBlock<float> block(*bufferToFill.buffer,
            (size_t)bufferToFill.startSample);
        dsp::ProcessContextReplacing<float> context(block);
        filterChain.process(context);
    }
}

void DJAudioPlayer::releaseResources()
{
    transportSource.releaseResources();
    resampleSource.releaseResources();
}

void DJAudioPlayer::loadURL(URL audioURL)
{
    auto* reader = formatManager.createReaderFor(audioURL.createInputStream(false));
    if (reader != nullptr)
    {
        std::unique_ptr<AudioFormatReaderSource> newSource(
            new AudioFormatReaderSource(reader, true));
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.reset(newSource.release());
    }
}

void DJAudioPlayer::setGain(double gain)
{
    if (gain >= 0 && gain <= 1.0)
        transportSource.setGain((float)gain);
}

void DJAudioPlayer::setSpeed(double ratio)
{
    if (ratio > 0 && ratio <= 100.0)
        resampleSource.setResamplingRatio(ratio);
}

void DJAudioPlayer::setPosition(double posInSecs)
{
    transportSource.setPosition(posInSecs);
}

void DJAudioPlayer::setPositionRelative(double pos)
{
    if (pos >= 0 && pos <= 1.0)
        setPosition(transportSource.getLengthInSeconds() * pos);
}

void DJAudioPlayer::start() { transportSource.start(); }
void DJAudioPlayer::stop() { transportSource.stop(); }

double DJAudioPlayer::getPositionRelative()
{
    return transportSource.getCurrentPosition() / transportSource.getLengthInSeconds();
}

double DJAudioPlayer::getLengthInSeconds()
{
    return transportSource.getLengthInSeconds();
}

// for R5D == BPM detection nd IOI histogram
double DJAudioPlayer::detectBPM()
{
    if (readerSource == nullptr) return 0.0;

    auto* reader = readerSource->getAudioFormatReader();
    if (reader == nullptr) return 0.0;

    const double sr = reader->sampleRate;
    const int    windowSize = (int)(sr * 0.01);   // 10ms windows
    const int    hopSize = windowSize / 2;
    const int64  totalSamples = reader->lengthInSamples;

    int64 samplesToRead = jmin(totalSamples, (int64)(sr * 180.0));
    AudioBuffer<float> buffer(1, (int)samplesToRead);
    reader->read(&buffer, 0, (int)samplesToRead, 0, true, false);

    // this will compute energy 
    std::vector<double> onsetTimes;
    double prevEnergy = 0.0;

    for (int pos = 0; pos + windowSize < (int)samplesToRead; pos += hopSize)
    {
        double energy = 0.0;
        for (int i = 0; i < windowSize; ++i)
        {
            float s = buffer.getSample(0, pos + i);
            energy += (double)(s * s);
        }
        energy /= windowSize;

        double flux = energy - prevEnergy;
        if (flux > 0.0002 && energy > prevEnergy * 1.3)
            onsetTimes.push_back((pos + windowSize / 2) / sr);

        prevEnergy = energy;
    }

    if (onsetTimes.size() < 4) return 0.0;

    // this to build IOI histogram in BPM 
    const int   numBins = 141;   // 60..200 BPM
    std::vector<int> histogram(numBins, 0);

    for (size_t i = 1; i < onsetTimes.size(); ++i)
    {
        double ioi = onsetTimes[i] - onsetTimes[i - 1];
        if (ioi <= 0.0) continue;
        double bpm = 60.0 / ioi;

        // this will help allow octave multiples 
        for (double mult : {1.0, 2.0, 0.5})
        {
            double b = bpm * mult;
            if (b >= 60.0 && b <= 200.0)
            {
                int bin = (int)(b - 60.0);
                if (bin >= 0 && bin < numBins)
                    histogram[bin]++;
            }
        }
    }

    // to find peak bin
    int peakBin = (int)(std::max_element(histogram.begin(), histogram.end())
        - histogram.begin());

    double estimatedBPM = 60.0 + peakBin;

    while (estimatedBPM < 90.0)
        estimatedBPM *= 2.0;

    while (estimatedBPM > 160.0)
        estimatedBPM /= 2.0;

    return estimatedBPM;
}

// this for R4A: update filter coefficients 
void DJAudioPlayer::setLowGain(double gain)
{
    lowGain = gain;
    updateEQ();
}

void DJAudioPlayer::setMidGain(double gain)
{
    midGain = gain;
    updateEQ();
}

void DJAudioPlayer::setHighGain(double gain)
{
    highGain = gain;
    updateEQ();
}

void DJAudioPlayer::updateEQ()
{
    if (sampleRate <= 0) return;

    float low = jmax(0.01f, (float)lowGain);
    float mid = jmax(0.01f, (float)midGain);
    float high = jmax(0.01f, (float)highGain);

    *filterChain.get<0>().state =
        *FilterCoefs::makeLowShelf((float)sampleRate, 300.0f, 0.7f, low);

    *filterChain.get<1>().state =
        *FilterCoefs::makePeakFilter((float)sampleRate, 1000.0f, 0.7f, mid);

    *filterChain.get<2>().state =
        *FilterCoefs::makeHighShelf((float)sampleRate, 5000.0f, 0.7f, high);
}