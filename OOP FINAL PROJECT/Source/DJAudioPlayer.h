/*
  ==============================================================================
    DJAudioPlayer.h
.
  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class DJAudioPlayer : public AudioSource
{
public:
 
    DJAudioPlayer(AudioFormatManager& formatManager);

    ~DJAudioPlayer();

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void loadURL(URL audioURL);
    void setGain(double gain);
    void setSpeed(double ratio);
    void setPosition(double posInSecs);
    void setPositionRelative(double pos);

    void start();
    void stop();

    double getPositionRelative();
    double getLengthInSeconds();

    void setLowGain(double gain);
    void setMidGain(double gain);
    void setHighGain(double gain);

    double getLowGain()  const { return lowGain; }
    double getMidGain()  const { return midGain; }
    double getHighGain() const { return highGain; }
    double detectBPM();

private:
   
    void updateEQ();

    AudioFormatManager& formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    ResamplingAudioSource resampleSource{ &transportSource, false, 2 };

    // this for DSP EQ chain
    using Filter = dsp::IIR::Filter<float>;
    using FilterCoefs = dsp::IIR::Coefficients<float>;
    using StereoFilter = dsp::ProcessorDuplicator<Filter, FilterCoefs>;

    dsp::ProcessorChain<StereoFilter, StereoFilter, StereoFilter> filterChain;
    dsp::ProcessSpec spec{ 44100, 512, 2 };

    double sampleRate = 44100.0;
    int    blockSize = 512;

    double lowGain = 1.0;   
    double midGain = 1.0;   
    double highGain = 1.0;  
};