#pragma once

#include <JuceHeader.h>

class _3ff3ctsAudioProcessor : public juce::AudioProcessor
{
public:
    _3ff3ctsAudioProcessor();
    ~_3ff3ctsAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Access to parameters
    juce::AudioProcessorValueTreeState& getParameters() { return apvts; }

private:
    // Distortion parameters
    juce::AudioParameterFloat* gainParameter;
    juce::AudioParameterFloat* distortionParameter;
    juce::AudioParameterChoice* distortionTypeParameter;

    // Delay parameters
    juce::AudioParameterFloat* delayTimeParameter;
    juce::AudioParameterFloat* delayFeedbackParameter;
    juce::AudioParameterFloat* delayMixParameter;

    // Parameter storage
    juce::AudioProcessorValueTreeState apvts;

    // Delay line buffer
    std::vector<float> delayBuffer;
    int delayBufferLength = 0;
    int delayWritePosition = 0;
    double currentSampleRate = 44100.0;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(_3ff3ctsAudioProcessor)
};