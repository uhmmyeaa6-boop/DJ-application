/*
  ==============================================================================
    PlaylistComponent.cpp
  ==============================================================================
*/

#include <JuceHeader.h>
#include "PlaylistComponent.h"

PlaylistComponent::PlaylistComponent(DeckGUI* _deck1,
    DeckGUI* _deck2,
    AudioFormatManager& _formatManager)
    : deck1(_deck1), deck2(_deck2), formatManager(_formatManager)
{
    tableComponent.getHeader().addColumn("Track Title", 1, 300);
    tableComponent.getHeader().addColumn("Duration", 2, 80);
    tableComponent.getHeader().addColumn("Deck 1", 3, 90);
    tableComponent.getHeader().addColumn("Deck 2", 4, 90);
    tableComponent.getHeader().addColumn("Remove", 5, 80);

    tableComponent.setModel(this);
    addAndMakeVisible(tableComponent);

    addTracksButton.addListener(this);
    addAndMakeVisible(addTracksButton);

    loadPlaylist();
}

PlaylistComponent::~PlaylistComponent()
{
}

void PlaylistComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);
}

void PlaylistComponent::resized()
{
    int btnH = 30;
    addTracksButton.setBounds(0, 0, getWidth(), btnH);
    tableComponent.setBounds(0, btnH, getWidth(), getHeight() - btnH);
}

int PlaylistComponent::getNumRows()
{
    return (int)tracks.size();
}

void PlaylistComponent::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(Colours::orange);
    else
        g.fillAll(Colours::darkgrey);
}

void PlaylistComponent::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if (rowNumber >= (int)tracks.size()) return;

    g.setColour(rowIsSelected ? Colours::black : Colours::white);
    if (columnId == 1)
        g.drawText(tracks[rowNumber].fileName, 4, 0, width - 4, height, Justification::centredLeft, true);
    else if (columnId == 2)
        g.drawText(tracks[rowNumber].duration, 4, 0, width - 4, height, Justification::centredLeft, true);
}

Component* PlaylistComponent::refreshComponentForCell(int rowNumber, int columnId,
    bool isRowSelected,
    Component* existingComponentToUpdate)
{
    if (columnId == 3 || columnId == 4 || columnId == 5)
    {
        TextButton* btn = dynamic_cast<TextButton*>(existingComponentToUpdate);
        if (btn == nullptr)
        {
            String label;
            if (columnId == 3) label = "-> Deck 1";
            else if (columnId == 4) label = "-> Deck 2";
            else label = "X Remove";
            btn = new TextButton(label);
            btn->addListener(this);
        }

        btn->setComponentID(String(rowNumber) + ":" + String(columnId));
        return btn;
    }
    return existingComponentToUpdate;
}

void PlaylistComponent::buttonClicked(Button* button)
{
    if (button == &addTracksButton)
    {
        addTracksToLibrary();
        return;
    }

    String compID = button->getComponentID();
    if (compID.contains(":"))
    {
        int row = compID.upToFirstOccurrenceOf(":", false, false).getIntValue();
        int col = compID.fromFirstOccurrenceOf(":", false, false).getIntValue();

        if (row >= 0 && row < (int)tracks.size())
        {
            if (col == 3)
            {
                deck1->loadFile(URL(File(tracks[row].filePath)));
            }
            else if (col == 4)
            {
                deck2->loadFile(URL(File(tracks[row].filePath)));
            }
            else if (col == 5)
            {
                tracks.erase(tracks.begin() + row);
                tableComponent.updateContent();
                tableComponent.repaint();
                savePlaylist();
            }
        }
    }
}

void PlaylistComponent::addTracksToLibrary()
{
    auto flags = FileBrowserComponent::canSelectFiles | FileBrowserComponent::canSelectMultipleItems;
    fChooser.launchAsync(flags, [this](const FileChooser& chooser)
        {
            for (const File& file : chooser.getResults())
            {
                TrackInfo info;
                info.fileName = file.getFileName().toStdString();
                info.filePath = file.getFullPathName().toStdString();

                std::unique_ptr<AudioFormatReader> reader(formatManager.createReaderFor(file));
                if (reader != nullptr)
                {
                    double secs = (double)reader->lengthInSamples / reader->sampleRate;
                    info.duration = formatDuration(secs);
                }
                else
                {
                    info.duration = "N/A";
                }

                tracks.push_back(info);
            }

            tableComponent.updateContent();
            savePlaylist();
        });
}

std::string PlaylistComponent::formatDuration(double seconds)
{
    int mins = (int)seconds / 60;
    int secs = (int)seconds % 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%d:%02d", mins, secs);
    return std::string(buf);
}

void PlaylistComponent::savePlaylist()
{
    Array<var> jsonArray;
    for (const auto& track : tracks)
    {
        auto* obj = new DynamicObject();
        obj->setProperty("fileName", String(track.fileName));
        obj->setProperty("filePath", String(track.filePath));
        obj->setProperty("duration", String(track.duration));
        jsonArray.add(var(obj));
    }

    File saveFile = File::getSpecialLocation(File::currentExecutableFile)
        .getParentDirectory()
        .getChildFile("OtoDecks_playlist.json");
    saveFile.replaceWithText(JSON::toString(var(jsonArray), false));
}

void PlaylistComponent::loadPlaylist()
{
    File saveFile = File::getSpecialLocation(File::currentExecutableFile)
        .getParentDirectory()
        .getChildFile("OtoDecks_playlist.json");

    if (!saveFile.existsAsFile()) return;

    var parsed = JSON::parse(saveFile.loadFileAsString());
    if (!parsed.isArray()) return;

    tracks.clear();
    for (int i = 0; i < parsed.size(); ++i)
    {
        var& item = parsed[i];
        TrackInfo info;
        info.fileName = item["fileName"].toString().toStdString();
        info.filePath = item["filePath"].toString().toStdString();
        info.duration = item["duration"].toString().toStdString();

        if (File(info.filePath).existsAsFile())
            tracks.push_back(info);
    }

    tableComponent.updateContent();
}
