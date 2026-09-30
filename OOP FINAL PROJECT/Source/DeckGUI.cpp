/*
  ==============================================================================
    DeckGUI.cpp
  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "DeckGUI.h"

DeckGUI::DeckGUI(DJAudioPlayer* _player,
    AudioFormatManager& formatManagerToUse,
    AudioThumbnailCache& cacheToUse)
    : player(_player),
    waveformDisplay(formatManagerToUse, cacheToUse)
{
    for (int i = 0; i < NUM_HOT_CUES; ++i)
        hotCues[i] = -1.0;

    // these are standard controls
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(waveformDisplay);
    addAndMakeVisible(volSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(posSlider);
    addAndMakeVisible(clearCuesButton);

    playButton.addListener(this);
    stopButton.addListener(this);
    loadButton.addListener(this);
    clearCuesButton.addListener(this);

    volSlider.addListener(this);
    speedSlider.addListener(this);
    posSlider.addListener(this);

    volSlider.setRange(0.0, 1.0);
    speedSlider.setRange(0.01, 2.0);
    posSlider.setRange(0.0, 1.0);

    // For the R4A: EQ sliders 
    // Range: 0.0[cut] to 2.0[boost], default 1.0[flat]
    lowEQSlider.setRange(0.01, 2.0);  lowEQSlider.setValue(1.0);
    midEQSlider.setRange(0.01, 2.0);  midEQSlider.setValue(1.0);
    highEQSlider.setRange(0.01, 2.0);  highEQSlider.setValue(1.0);

    lowEQSlider.setSliderStyle(Slider::LinearVertical);
    midEQSlider.setSliderStyle(Slider::LinearVertical);
    highEQSlider.setSliderStyle(Slider::LinearVertical);

    lowEQSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    midEQSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    highEQSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    lowEQSlider.addListener(this);
    midEQSlider.addListener(this);
    highEQSlider.addListener(this);

    addAndMakeVisible(lowEQSlider);
    addAndMakeVisible(midEQSlider);
    addAndMakeVisible(highEQSlider);

    lowEQLabel.setText("LOW", dontSendNotification);
    midEQLabel.setText("MID", dontSendNotification);
    highEQLabel.setText("HIGH", dontSendNotification);

    lowEQLabel.setJustificationType(Justification::centred);
    midEQLabel.setJustificationType(Justification::centred);
    highEQLabel.setJustificationType(Justification::centred);

    lowEQLabel.setFont(Font(11.0f));
    midEQLabel.setFont(Font(11.0f));
    highEQLabel.setFont(Font(11.0f));

    addAndMakeVisible(lowEQLabel);
    addAndMakeVisible(midEQLabel);
    addAndMakeVisible(highEQLabel);

    //these are hot cue buttons
    for (int i = 0; i < NUM_HOT_CUES; ++i)
    {
        cueButtons[i].setButtonText("C" + String(i + 1));
        cueButtons[i].setComponentID(String(i));
        cueButtons[i].addListener(this);
        addAndMakeVisible(cueButtons[i]);
    }

    startTimer(500);
}

DeckGUI::~DeckGUI()
{
    stopTimer();
}

void DeckGUI::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    g.setColour(Colours::grey);
    g.drawRect(getLocalBounds(), 1);

    // for the EQ section 
    g.setColour(Colours::white);
    g.setFont(12.0f);
}

void DeckGUI::resized()
{
    // Layout [14 equal rows]
    double rowH = getHeight() / 14.0;

    // these are standard controls for rows 0-7
    playButton.setBounds(0, 0, getWidth(), (int)rowH);
    stopButton.setBounds(0, (int)rowH, getWidth(), (int)rowH);
    volSlider.setBounds(0, (int)(rowH * 2), getWidth(), (int)rowH);
    speedSlider.setBounds(0, (int)(rowH * 3), getWidth(), (int)rowH);
    posSlider.setBounds(0, (int)(rowH * 4), getWidth(), (int)rowH);
    waveformDisplay.setBounds(0, (int)(rowH * 5), getWidth(), (int)(rowH * 2));
    loadButton.setBounds(0, (int)(rowH * 7), getWidth(), (int)rowH);

    // uhm R4A: EQ sliders for rows 8-10
    int eqW = getWidth() / 3;
    int eqY = (int)(rowH * 8);
    int eqH = (int)(rowH * 2);
    int lblH = (int)rowH;

    lowEQLabel.setBounds(0, eqY, eqW, lblH / 2);
    midEQLabel.setBounds(eqW, eqY, eqW, lblH / 2);
    highEQLabel.setBounds(eqW * 2, eqY, eqW, lblH / 2);

    lowEQSlider.setBounds(0, eqY + lblH / 2, eqW, eqH);
    midEQSlider.setBounds(eqW, eqY + lblH / 2, eqW, eqH);
    highEQSlider.setBounds(eqW * 2, eqY + lblH / 2, eqW, eqH);

    // for the hot cue buttons: 4 per row [rows 11-12]
    int cueW = getWidth() / 4;
    for (int i = 0; i < NUM_HOT_CUES; ++i)
    {
        int col = i % 4;
        int row = i / 4;
        cueButtons[i].setBounds(col * cueW, (int)(rowH * (11 + row)), cueW, (int)rowH);
    }

    clearCuesButton.setBounds(0, (int)(rowH * 13), getWidth(), (int)rowH);
}

void DeckGUI::buttonClicked(Button* button)
{
    if (button == &playButton) { player->start(); return; }
    if (button == &stopButton) { player->stop();  return; }
    if (button == &clearCuesButton) { clearAllCues();  return; }

    if (button == &loadButton)
    {
        fChooser.launchAsync(FileBrowserComponent::canSelectFiles,
            [this](const FileChooser& chooser) { loadFile(URL{ chooser.getResult() }); });
        return;
    }

    for (int i = 0; i < NUM_HOT_CUES; ++i)
    {
        if (button == &cueButtons[i])
        {
            if (hotCues[i] < 0.0)
                setCue(i);
            else
                player->setPositionRelative(hotCues[i]);
            return;
        }
    }
}

void DeckGUI::sliderValueChanged(Slider* slider)
{
    if (slider == &volSlider)    player->setGain(slider->getValue());
    if (slider == &speedSlider)  player->setSpeed(slider->getValue());
    if (slider == &posSlider)    player->setPositionRelative(slider->getValue());

    // also for R4A: EQ slider changes
    if (slider == &lowEQSlider) { player->setLowGain(slider->getValue());  saveCues(); }
    if (slider == &midEQSlider) { player->setMidGain(slider->getValue());  saveCues(); }
    if (slider == &highEQSlider) { player->setHighGain(slider->getValue()); saveCues(); }
}

bool DeckGUI::isInterestedInFileDrag(const StringArray&) { return true; }

void DeckGUI::filesDropped(const StringArray& files, int /*this x*/, int /*this y*/)
{
    if (files.size() == 1)
        loadFile(URL{ File{files[0]} });
}

void DeckGUI::timerCallback()
{
    waveformDisplay.setPositionRelative(player->getPositionRelative());
    waveformDisplay.setTotalLength(player->getLengthInSeconds());
}

void DeckGUI::loadFile(URL audioURL)
{
    if (currentFilePath.isNotEmpty())
        saveCues();

    currentFilePath = audioURL.getLocalFile().getFullPathName();
    player->loadURL(audioURL);
    waveformDisplay.loadURL(audioURL);
    waveformDisplay.setTotalLength(player->getLengthInSeconds());
    waveformDisplay.setBPM(0);   // this reset when detecting

    for (int i = 0; i < NUM_HOT_CUES; ++i)
        hotCues[i] = -1.0;

    loadCues();
    updateCueButtonLabels();

    // for R5D to detect BPM on a background thread 
    Thread::launch([this]()
        {
            double detectedBPM = player->detectBPM();
            MessageManager::callAsync([this, detectedBPM]()
                {
                    waveformDisplay.setBPM(detectedBPM);
                });
        });
}

void DeckGUI::setCue(int index)
{
    if (currentFilePath.isEmpty()) return;
    hotCues[index] = player->getPositionRelative();
    updateCueButtonLabels();
    saveCues();
}

void DeckGUI::clearAllCues()
{
    for (int i = 0; i < NUM_HOT_CUES; ++i)
        hotCues[i] = -1.0;
    updateCueButtonLabels();
    saveCues();
}

void DeckGUI::updateCueButtonLabels()
{
    for (int i = 0; i < NUM_HOT_CUES; ++i)
    {
        if (hotCues[i] < 0.0)
        {
            cueButtons[i].setButtonText("C" + String(i + 1));
            cueButtons[i].setColour(TextButton::buttonColourId,
                getLookAndFeel().findColour(TextButton::buttonColourId));
        }
        else
        {
            int totalSecs = (int)(hotCues[i] * player->getLengthInSeconds());
            int mins = totalSecs / 60;
            int secs = totalSecs % 60;
            String t = String(mins) + ":" + (secs < 10 ? "0" : "") + String(secs);
            cueButtons[i].setButtonText("C" + String(i + 1) + "\n" + t);
            cueButtons[i].setColour(TextButton::buttonColourId, Colour(0xFF1E6B3C));
        }
    }
}

// in R4B: Save cues AND EQ settings to JSON 
void DeckGUI::saveCues()
{
    if (currentFilePath.isEmpty()) return;

    File cueFile = File::getSpecialLocation(File::currentExecutableFile)
        .getParentDirectory()
        .getChildFile("OtoDecks_cues.json");

    var root;
    if (cueFile.existsAsFile())
        root = JSON::parse(cueFile.loadFileAsString());
    if (!root.isObject())
        root = var(new DynamicObject());

    // Builds object for track
    auto* trackObj = new DynamicObject();

    // Cues array
    Array<var> cueArray;
    for (int i = 0; i < NUM_HOT_CUES; ++i)
        cueArray.add(var(hotCues[i]));
    trackObj->setProperty("cues", var(cueArray));

    // this is EQ values
    trackObj->setProperty("lowEQ", var(player->getLowGain()));
    trackObj->setProperty("midEQ", var(player->getMidGain()));
    trackObj->setProperty("highEQ", var(player->getHighGain()));

    root.getDynamicObject()->setProperty(currentFilePath, var(trackObj));
    cueFile.replaceWithText(JSON::toString(root, false));
}

// R4B: this will load cues AND EQ settings from JSON
void DeckGUI::loadCues()
{
    if (currentFilePath.isEmpty()) return;

    File cueFile = File::getSpecialLocation(File::currentExecutableFile)
        .getParentDirectory()
        .getChildFile("OtoDecks_cues.json");
    if (!cueFile.existsAsFile()) return;

    var root = JSON::parse(cueFile.loadFileAsString());
    if (!root.isObject()) return;

    var trackData = root.getDynamicObject()->getProperty(currentFilePath);
    if (!trackData.isObject()) return;

    // this restore cues
    var cueArray = trackData["cues"];
    if (cueArray.isArray())
        for (int i = 0; i < NUM_HOT_CUES && i < cueArray.size(); ++i)
            hotCues[i] = (double)cueArray[i];

    // R4B: Restore EQ
    double low = trackData.getProperty("lowEQ", var(1.0));
    double mid = trackData.getProperty("midEQ", var(1.0));
    double high = trackData.getProperty("highEQ", var(1.0));

    player->setLowGain(low);
    player->setMidGain(mid);
    player->setHighGain(high);

    // will update slider positions to match the loaded values
    lowEQSlider.setValue(low, dontSendNotification);
    midEQSlider.setValue(mid, dontSendNotification);
    highEQSlider.setValue(high, dontSendNotification);
}