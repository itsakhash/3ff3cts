#include "PluginProcessor.h"
#include "PluginEditor.h"

_3ff3ctsAudioProcessor::_3ff3ctsAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "Parameters", createParameters())
{
    gainParameter = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("gain"));
    distortionParameter = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("distortion"));
    distortionTypeParameter = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("distortionType"));

    delayTimeParameter = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("delayTime"));
    delayFeedbackParameter = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("delayFeedback"));
    delayMixParameter = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("delayMix"));

    // Initialize delay buffer
    delayBufferLength = 44100 * 2; // 2 seconds maximum
    delayBuffer.resize(delayBufferLength, 0.0f);
}

_3ff3ctsAudioProcessor::~_3ff3ctsAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout _3ff3ctsAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Distortion parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gain",
        "Gain",
        0.0f,
        1.0f,
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "distortion",
        "Distortion",
        0.0f,
        2.0f,
        0.0f));

    juce::StringArray distortionTypes = {
        "Soft Clip", "Hard Clip", "Tube", "Diode", "Fold", "Sine"
    };

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "distortionType",
        "Distortion Type",
        distortionTypes,
        0));

    // Delay parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "delayTime",
        "Delay Time",
        0.01f,
        1.0f,
        0.3f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "delayFeedback",
        "Feedback",
        0.0f,
        0.95f,
        0.4f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "delayMix",
        "Mix",
        0.0f,
        1.0f,
        0.5f));

    return { params.begin(), params.end() };
}

void _3ff3ctsAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Resize delay buffer based on sample rate
    delayBufferLength = (int)(2.0 * sampleRate); // 2 seconds max delay
    delayBuffer.resize(delayBufferLength, 0.0f);
    delayWritePosition = 0;
}

void _3ff3ctsAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool _3ff3ctsAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void _3ff3ctsAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Get distortion parameters
    float currentGain = gainParameter->get();
    float currentDistortion = distortionParameter->get();
    int distortionType = distortionTypeParameter->getIndex();

    // Get delay parameters
    float delayTime = delayTimeParameter->get();
    float delayFeedback = delayFeedbackParameter->get();
    float delayMix = delayMixParameter->get();

    // Process distortion
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float cleanSample = channelData[sample];
            float distortedSample = cleanSample;

            // Apply distortion if amount > 0
            if (currentDistortion > 0.0f)
            {
                float drive = 1.0f + 20.0f * std::pow(currentDistortion, 2.0f);

                switch (distortionType)
                {
                case 0: // Soft Clip (Tanh)
                    distortedSample = std::tanh(cleanSample * drive) / std::tanh(drive);
                    break;

                case 1: // Hard Clip
                {
                    float threshold = 1.0f / drive;
                    distortedSample = juce::jlimit(-threshold, threshold, cleanSample);
                    distortedSample *= drive * 0.5f;
                }
                break;

                case 2: // Tube
                {
                    float x = cleanSample * drive;
                    if (x > 0)
                        distortedSample = 1.0f - std::exp(-x);
                    else
                        distortedSample = -1.0f + std::exp(x);
                }
                break;

                case 3: // Diode
                {
                    float x = cleanSample * drive;
                    distortedSample = x / (1.0f + std::abs(x));
                    if (x > 0)
                        distortedSample *= 0.9f;
                }
                break;

                case 4: // Fold
                {
                    float x = cleanSample * drive * 3.0f;
                    distortedSample = std::sin(x) / (1.0f + 0.2f * std::abs(x));
                }
                break;

                case 5: // Sine
                {
                    float x = cleanSample * drive;
                    distortedSample = std::sin(x * juce::MathConstants<float>::pi * 0.5f);
                }
                break;
                }
            }

            // Apply gain after distortion
            channelData[sample] = distortedSample * currentGain;
        }
    }

    // Process delay
    if (delayMix > 0.0f)
    {
        // Calculate delay in samples
        int delaySamples = (int)(delayTime * currentSampleRate);

        // For simple stereo processing
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            // Process each sample
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                // Get the current sample
                const float in = channelData[sample];

                // Calculate read position
                int readPosition = delayWritePosition - delaySamples;
                if (readPosition < 0)
                    readPosition += delayBufferLength;

                // Get delayed sample
                float delayed = delayBuffer[readPosition];

                // Write to delay buffer (input + feedback)
                delayBuffer[delayWritePosition] = in + delayed * delayFeedback;

                // Increment and wrap write position
                delayWritePosition++;
                if (delayWritePosition >= delayBufferLength)
                    delayWritePosition = 0;

                // Mix dry/wet
                channelData[sample] = in * (1.0f - delayMix) + delayed * delayMix;
            }
        }
    }
}

bool _3ff3ctsAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* _3ff3ctsAudioProcessor::createEditor()
{
    return new _3ff3ctsAudioProcessorEditor(*this);
}

void _3ff3ctsAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void _3ff3ctsAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

const juce::String _3ff3ctsAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool _3ff3ctsAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool _3ff3ctsAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool _3ff3ctsAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double _3ff3ctsAudioProcessor::getTailLengthSeconds() const
{
    // Return a longer tail length to accommodate delay
    return delayTimeParameter->get() * 5.0;
}

int _3ff3ctsAudioProcessor::getNumPrograms()
{
    return 1;
}

int _3ff3ctsAudioProcessor::getCurrentProgram()
{
    return 0;
}

void _3ff3ctsAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String _3ff3ctsAudioProcessor::getProgramName(int index)
{
    return {};
}

void _3ff3ctsAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new _3ff3ctsAudioProcessor();
}