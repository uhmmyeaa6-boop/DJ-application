/*
  ==============================================================================
    WaveformDisplay.h

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class WaveformDisplay : public Component,
    public ChangeListener
{
public:
   
    WaveformDisplay(AudioFormatManager& formatManagerToUse,
        AudioThumbnailCache& cacheToUse);

    ~WaveformDisplay();

    void paint(Graphics& g) override;
    void resized() override;
    void changeListenerCallback(ChangeBroadcaster* source) override;
    void loadURL(URL audioURL);
    void setPositionRelative(double pos);
    void setTotalLength(double lengthInSeconds);
    void setBPM(double bpm);

private:
    
    String formatTime(double seconds);

    AudioThumbnail audioThumb;
    bool   fileLoaded;
    double position;     
    double totalLength;  
    double bpm;          

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};