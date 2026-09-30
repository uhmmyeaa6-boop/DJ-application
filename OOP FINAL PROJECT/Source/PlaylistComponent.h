/*
  ==============================================================================
    PlaylistComponent.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <string>
#include "DeckGUI.h"

struct TrackInfo
{
    std::string fileName;  
    std::string filePath;  
    std::string duration;  
};

class PlaylistComponent : public juce::Component,
    public TableListBoxModel,
    public Button::Listener
{
public:
    
    PlaylistComponent(DeckGUI* deck1, DeckGUI* deck2,
        AudioFormatManager& formatManager);

    ~PlaylistComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    int getNumRows() override;
    void paintRowBackground(Graphics& g, int rowNumber, int width,
        int height, bool rowIsSelected) override;

    void paintCell(Graphics& g, int rowNumber, int columnId,
        int width, int height, bool rowIsSelected) override;

    Component* refreshComponentForCell(int rowNumber, int columnId,
        bool isRowSelected,
        Component* existingComponentToUpdate) override;

    void buttonClicked(Button* button) override;

private:

    void addTracksToLibrary();
    void savePlaylist();
    void loadPlaylist();

    std::string formatDuration(double seconds);

    TableListBox tableComponent;
    std::vector<TrackInfo> tracks;

    TextButton addTracksButton{ "Add Tracks" };
    FileChooser fChooser{ "Select audio files..." };

    DeckGUI* deck1;
    DeckGUI* deck2;
    AudioFormatManager& formatManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
};