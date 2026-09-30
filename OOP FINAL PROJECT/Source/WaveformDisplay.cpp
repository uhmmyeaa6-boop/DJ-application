/*
  ==============================================================================
    WaveformDisplay.cpp
  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "WaveformDisplay.h"
#include <cmath>

WaveformDisplay::WaveformDisplay(AudioFormatManager& formatManagerToUse,
    AudioThumbnailCache& cacheToUse)
    : audioThumb(1000, formatManagerToUse, cacheToUse),
    fileLoaded(false),
    position(0),
    totalLength(0),
    bpm(0)
{
    audioThumb.addChangeListener(this);
}

WaveformDisplay::~WaveformDisplay()
{
}

void WaveformDisplay::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    g.setColour(Colours::grey);
    g.drawRect(getLocalBounds(), 1);

    if (fileLoaded)
    {
        g.setColour(Colours::orange);
       
        Rectangle<int> waveArea(0, 0, getWidth(), getHeight() - 18);
        audioThumb.drawChannel(g, waveArea, 0, audioThumb.getTotalLength(), 0, 1.0f);

        g.setColour(Colours::lightgreen);
        int playheadX = (int)(position * getWidth());
        g.drawLine((float)playheadX, 0, (float)playheadX, (float)(getHeight() - 18), 2.0f);

        int barY = getHeight() - 18;
        g.setColour(Colour(0xFF1A1A2E));
        g.fillRect(0, barY, getWidth(), 18);

        g.setColour(Colour(0xFF1E6B3C));
        g.fillRect(0, barY, (int)(position * getWidth()), 18);

        // Border
        g.setColour(Colours::grey);
        g.drawRect(0, barY, getWidth(), 18, 1);

        double elapsed = position * totalLength;
        String timeText = formatTime(elapsed) + " / " + formatTime(totalLength);
        if (bpm > 0)
            timeText += "   " + String((int)std::round(bpm)) + " BPM";

        g.setColour(Colours::white);
        g.setFont(Font(12.0f, Font::bold));
        g.drawText(timeText, 4, barY, getWidth() - 8, 18, Justification::centred, false);
    }
    else
    {
        g.setColour(Colours::orange);
        g.setFont(20.0f);
        g.drawText("File not loaded...", getLocalBounds(), Justification::centred, true);
    }
}

void WaveformDisplay::resized()
{
}

void WaveformDisplay::loadURL(URL audioURL)
{
    audioThumb.clear();
    fileLoaded = audioThumb.setSource(new URLInputSource(audioURL));
    if (fileLoaded)
        repaint();
}

void WaveformDisplay::changeListenerCallback(ChangeBroadcaster* source)
{
    repaint();
}

void WaveformDisplay::setPositionRelative(double pos)
{
    if (pos != position)
    {
        position = pos;
        repaint();
    }
}

void WaveformDisplay::setTotalLength(double lengthInSeconds)
{
    totalLength = lengthInSeconds;
}

void WaveformDisplay::setBPM(double detectedBPM)
{
    bpm = detectedBPM;
    repaint();
}

String WaveformDisplay::formatTime(double seconds)
{
    if (seconds < 0 || std::isnan(seconds)) seconds = 0;
    int mins = (int)seconds / 60;
    int secs = (int)seconds % 60;
    return String(mins) + ":" + (secs < 10 ? "0" : "") + String(secs);
}


