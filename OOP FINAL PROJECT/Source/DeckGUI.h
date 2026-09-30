/*
  ==============================================================================
    DeckGUI.h
  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DJAudioPlayer.h"
#include "WaveformDisplay.h"

static const int NUM_HOT_CUES = 8;

class DeckGUI : public Component,
    public Button::Listener,
    public Slider::Listener,
    public FileDragAndDropTarget,
    public Timer
{
public:
    /** Constructors */
    DeckGUI(DJAudioPlayer* player,
        AudioFormatManager& formatManagerToUse,
        AudioThumbnailCache& cacheToUse);

    /** Destructor */
    ~DeckGUI();

    /** this draws the deck background */
    void paint(Graphics&) override;

    void resized() override;

    /** this will handle button click for play, stop, load, cue, and clear buttons. */
    void buttonClicked(Button* button) override;

    /** this will handle slider value changes for volume, speed, position, and EQ sliders */
    void sliderValueChanged(Slider* slider) override;

    /** this will returns true to this component to accept the file drag-and-drop */
    bool isInterestedInFileDrag(const StringArray& files) override;

    void filesDropped(const StringArray& files, int x, int y) override;

    /** calls for the waveform position nd total track length display*/
    void timerCallback() override;

    
    void loadFile(URL audioURL);

private:
    
    void setCue(int index);

    void clearAllCues();

    void saveCues();

    void loadCues();

    /** Refreshes the cue and colour of each cue button to know the currect state of cue */
    void updateCueButtonLabels();

    // --- Playback controls ---
    TextButton playButton{ "PLAY" };
    TextButton stopButton{ "STOP" };
    TextButton loadButton{ "LOAD" };
    TextButton clearCuesButton{ "Clear Cues" };

    // this the hot cue buttons (C1–C8) 
    TextButton cueButtons[NUM_HOT_CUES];

    // this the transport sliders 
    Slider volSlider;    // Volume:   [0.0, 1.0]
    Slider speedSlider;  // Speed:    [0.01, 2.0]
    Slider posSlider;    // Position: [0.0, 1.0]

    // this the 3-band EQ sliders 
    Slider lowEQSlider;   // Low:  [0.01, 2.0]
    Slider midEQSlider;   // Mid:   [0.01, 2.0]
    Slider highEQSlider;  // High: [0.01, 2.0]

    Label lowEQLabel;
    Label midEQLabel;
    Label highEQLabel;

    FileChooser fChooser{ "Select a file..." };
    WaveformDisplay waveformDisplay;
    DJAudioPlayer* player;

    double hotCues[NUM_HOT_CUES];   
    String currentFilePath;         

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckGUI)
};