/*
  ==============================================================================

    PolyRhythmMetronome.cpp
    Created: 6 Jan 2022 3:59:04pm
    Author:  Romal

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PolyRhythmMetronome.h"

const int RHYTHM_1_MIDI_VALUE = 36;
const int RHYTHM_2_MIDI_VALUE = 37;


//==============================================================================
PolyRhythmMetronome::PolyRhythmMetronome()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}



PolyRhythmMetronome::PolyRhythmMetronome(juce::AudioProcessorValueTreeState* _apvts)
{
    apvts = _apvts;
    resetAll();
    formatManager.registerBasicFormats();
   
    juce::MemoryInputStream* inputStream = new juce::MemoryInputStream(BinaryData::rimshot_low_wav, BinaryData::rimshot_low_wavSize, false);
    juce::WavAudioFormat wavFormat;
    juce::AudioFormatReader* formatReader = wavFormat.createReaderFor(inputStream, false);
    rimShotLow.reset(new juce::AudioFormatReaderSource(formatReader, true));

    juce::MemoryInputStream* inputStream2 = new juce::MemoryInputStream(BinaryData::rimshot_high_wav, BinaryData::rimshot_high_wavSize, false);
    juce::WavAudioFormat wavFormat2;
    juce::AudioFormatReader* formatReader2 = wavFormat2.createReaderFor(inputStream2, false);
    rimShotHigh.reset(new juce::AudioFormatReaderSource(formatReader2, true));

    juce::MemoryInputStream* inputStream3 = new juce::MemoryInputStream(BinaryData::rimshot_sub_wav, BinaryData::rimshot_sub_wavSize, false);
    juce::WavAudioFormat wavFormat3;
    juce::AudioFormatReader* formatReader3 = wavFormat3.createReaderFor(inputStream3, false);
    rimShotSub.reset(new juce::AudioFormatReaderSource(formatReader3, true));
    
}

PolyRhythmMetronome::~PolyRhythmMetronome()
{
}


void PolyRhythmMetronome::prepareToPlay(double _sampleRate, int samplesPerBlock)
{
    //preparetoplay should call every time we start (right before)

    resetParams();

    if (sampleRate != _sampleRate)
    {
        //if the audioprocessors samplerate hasn't changed, nothing else needs to be done
        sampleRate = _sampleRate;
        if (rimShotLow != nullptr)
        {
            rimShotLow->prepareToPlay(samplesPerBlock, sampleRate);
        }
        if (rimShotHigh != nullptr)
        {
            rimShotHigh->prepareToPlay(samplesPerBlock, sampleRate);
        }
        if (rimShotSub != nullptr)
        {
            rimShotSub->prepareToPlay(samplesPerBlock, sampleRate);
        }
    }

}
void PolyRhythmMetronome::getNextAudioBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{   

    bool isDawConnected = apvts->getRawParameterValue("DAW_CONNECTED")->load();
    bool isDawPlaying = apvts->getRawParameterValue("DAW_PLAYING")->load();

    resetParams();
    auto audioSourceChannelInfo = juce::AudioSourceChannelInfo(buffer);
    auto bufferSize = buffer.getNumSamples(); //usually 16, 32, 64... 1024...
    if (!isDawConnected && !isDawPlaying) {
        totalSamples += bufferSize;
    }
    else {
        totalSamples = ((int)apvts->getRawParameterValue("DAW_SAMPLES_ELAPSED")->load() % (int)samplesPerBar) + (int)bufferSize;
    }

    rhythm1SamplesProcessed = totalSamples % rhythm1Interval;
    rhythm2SamplesProcessed = totalSamples % rhythm2Interval;


    bool rhythm1Flag = (rhythm1SamplesProcessed + bufferSize >= rhythm1Interval && rhythm1Value > 1);
    bool rhythm2Flag = (rhythm2SamplesProcessed + bufferSize >= rhythm2Interval && rhythm2Value > 1);
   

    if (rhythm1Counter >= rhythm1Value )
    {
        rhythm1Counter = 0;
    }
    if (rhythm2Counter >= rhythm2Value )
    {
        rhythm2Counter = 0;
    }



    //TODO make this block cleaner
    if ( ((rhythm1Flag && rhythm2Flag )) )
    { // both beats hit at the same time , play a unique tick for that 

        const auto timeToStartPlaying = rhythm1Interval - rhythm1SamplesProcessed;
        rhythm1Counter += 1;
        rhythm2Counter += 1;
        int ID1 = rhythm1Counter;
        int ID2 = rhythm2Counter;
        if (rhythm1Counter == rhythm1Value){
            ID1 = 0;
        }
        if (rhythm2Counter == rhythm2Value) {
            ID2 = 0;
        }


        if (apvts->getRawParameterValue("RHYTHM1." + to_string(ID1) + "_TOGGLE")->load() == true && apvts->getRawParameterValue("RHYTHM2." + to_string(ID2) + "_TOGGLE")->load() == true) {
            handleNoteTrigger(midiBuffer, RHYTHM_1_MIDI_VALUE);
            handleNoteTrigger(midiBuffer, RHYTHM_2_MIDI_VALUE);
            rimShotHigh->setNextReadPosition(0); //reset sample to beginning
            for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
            {
                if (samplenum == timeToStartPlaying)
                {
                    rimShotHigh->getNextAudioBlock(audioSourceChannelInfo);

                }
            }

        }
        else if (apvts->getRawParameterValue("RHYTHM1." + to_string(ID1) + "_TOGGLE")->load() == true) {

            handleNoteTrigger(midiBuffer, RHYTHM_1_MIDI_VALUE);
            rimShotLow->setNextReadPosition(0); //reset sample to beginning
            for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
            {
                if (samplenum == timeToStartPlaying)
                {
                    rimShotLow->getNextAudioBlock(audioSourceChannelInfo);

                }
            }

        }
        else if (apvts->getRawParameterValue("RHYTHM2." + to_string(ID2) + "_TOGGLE")->load() == true) {

            handleNoteTrigger(midiBuffer, RHYTHM_2_MIDI_VALUE);
            rimShotSub->setNextReadPosition(0); //reset sample to beginning
            for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
            { 
                if (samplenum == timeToStartPlaying)
                {
                    rimShotSub->getNextAudioBlock(audioSourceChannelInfo);

                }
            }
        }

    }
    else if (rhythm1Flag)
    {
        rhythm1Counter += 1;
        if (apvts->getRawParameterValue("RHYTHM1." + to_string(rhythm1Counter) + "_TOGGLE")->load() == true) {

            const auto timeToStartPlaying = rhythm1Interval - rhythm1SamplesProcessed;
            rimShotLow->setNextReadPosition(0); //reset sample to beginning
            for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
            {
                if (samplenum == timeToStartPlaying)
                {
                    rimShotLow->getNextAudioBlock(audioSourceChannelInfo);
                    handleNoteTrigger(midiBuffer, RHYTHM_1_MIDI_VALUE);
                }
            }
        }
    }
    else if (rhythm2Flag )
    {
        rhythm2Counter += 1;
        if (apvts->getRawParameterValue("RHYTHM2." + to_string(rhythm2Counter) + "_TOGGLE")->load() == true) {

            const auto timeToStartPlaying = rhythm2Interval - rhythm2SamplesProcessed ;
            rimShotSub->setNextReadPosition(0); //reset sample to beginning
            for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
            { 
                if (samplenum == timeToStartPlaying)
                {
                    rimShotSub->getNextAudioBlock(audioSourceChannelInfo);
                    handleNoteTrigger(midiBuffer, RHYTHM_2_MIDI_VALUE);
                }
            }
        }
    }

    if (totalSamples >= samplesPerBar && !isDawPlaying) {
        totalSamples = totalSamples - samplesPerBar;
    }

}

void PolyRhythmMetronome::handleNoteTrigger(juce::MidiBuffer& midiBuffer, int noteNumber)
{
    auto noteDuration = sampleRate;
    auto message = juce::MidiMessage::noteOn(1, noteNumber, (juce::uint8)100);
    //message.setTimeStamp(noteDuration);

    auto messageOff = juce::MidiMessage::noteOff(message.getChannel(), message.getNoteNumber());
    //messageOff.setTimeStamp((noteDuration));

    if (! midiBuffer.addEvent(message, 0)  || ! midiBuffer.addEvent(messageOff, 100) )
    {
        DBG("error adding messages to midiBuffer");
    }
}
void PolyRhythmMetronome::resetAll()
{   //this should be called whenever the metronome is stopped
   // resetParams();
    totalSamples = 0;
    rhythm1Counter = 0;
    rhythm2Counter = 0;
    rhythm1SamplesProcessed = 0;
    rhythm2SamplesProcessed = 0;
}


void PolyRhythmMetronome::resetParams()
{  //this should be called when params change in UI to reflect changes in logic
   //the variables keeping track of time should be reset to reflect the new rhythm

    int tempR1Value = apvts->getRawParameterValue("NUMERATOR")->load();
    if (rhythm1Value != tempR1Value)
    {
        rhythm1Value = tempR1Value;
        resetAll();
    }
    int tempR2Value = apvts->getRawParameterValue("SUBDIVISION")->load();
    if (rhythm2Value != tempR2Value)
    {
        rhythm2Value = tempR2Value;
        resetAll();
    }


    bpm = apvts->getRawParameterValue("BPM")->load();
    samplesPerBar = 4 * ((60.0 / bpm) * sampleRate);    //4 * because we have 4 beats in a bar
    rhythm1Interval = samplesPerBar / rhythm1Value;
    rhythm2Interval = samplesPerBar / rhythm2Value;
    ///TODO  assumes 4/4 time, a time signature parameter could be interesting

}
