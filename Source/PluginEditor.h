#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// Distortion visualization component
class WaveShapeDisplay : public juce::Component
{
public:
    WaveShapeDisplay()
    {
        setOpaque(true);
    }

    void setDistortionType(int type) { distortionType = type; repaint(); }
    void setAmount(float amount) { distortionAmount = amount; repaint(); }

    void paint(juce::Graphics& g) override
    {
        // Background
        g.fillAll(juce::Colours::black);

        // Grid lines
        g.setColour(juce::Colours::darkgrey);
        auto bounds = getLocalBounds().toFloat();
        float centerX = bounds.getCentreX();
        float centerY = bounds.getCentreY();

        // Draw center lines
        g.drawLine(0, centerY, bounds.getWidth(), centerY, 1.0f);
        g.drawLine(centerX, 0, centerX, bounds.getHeight(), 1.0f);

        // Draw the input/output curve
        g.setColour(juce::Colours::lightgreen);

        juce::Path curvePath;
        bool pathStarted = false;

        // Generate the distortion curve
        for (float x = -1.0f; x <= 1.0f; x += 0.01f)
        {
            float input = x;
            float output = processSample(input, distortionAmount, distortionType);

            // Map to component coordinates
            float displayX = juce::jmap(input, -1.0f, 1.0f, 0.0f, bounds.getWidth());
            float displayY = juce::jmap(output, 1.0f, -1.0f, 0.0f, bounds.getHeight());

            if (!pathStarted)
            {
                curvePath.startNewSubPath(displayX, displayY);
                pathStarted = true;
            }
            else
            {
                curvePath.lineTo(displayX, displayY);
            }
        }

        g.strokePath(curvePath, juce::PathStrokeType(2.0f));

        // Draw labels
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        g.drawText("In", 5, centerY - 20, 20, 20, juce::Justification::left);
        g.drawText("Out", centerX + 5, 5, 30, 20, juce::Justification::left);
    }

private:
    float processSample(float sample, float drive, int type)
    {
        float output = sample;
        drive = 1.0f + 20.0f * std::pow(drive, 2.0f);

        switch (type)
        {
        case 0: // Soft Clip (Tanh)
            output = std::tanh(sample * drive) / std::tanh(drive);
            break;

        case 1: // Hard Clip
        {
            float threshold = 1.0f / drive;
            output = juce::jlimit(-threshold, threshold, sample);
            output *= drive * 0.5f;
        }
        break;

        case 2: // Tube
        {
            float x = sample * drive;
            if (x > 0)
                output = 1.0f - std::exp(-x);
            else
                output = -1.0f + std::exp(x);
        }
        break;

        case 3: // Diode
        {
            float x = sample * drive;
            output = x / (1.0f + std::abs(x));
            if (x > 0)
                output *= 0.9f;
        }
        break;

        case 4: // Fold
        {
            float x = sample * drive * 3.0f;
            output = std::sin(x) / (1.0f + 0.2f * std::abs(x));
        }
        break;

        case 5: // Sine
        {
            float x = sample * drive;
            output = std::sin(x * juce::MathConstants<float>::pi * 0.5f);
        }
        break;
        }

        return output;
    }

    int distortionType = 0;
    float distortionAmount = 0.0f;
};

// Delay visualization component
class DelayDisplay : public juce::Component
{
public:
    DelayDisplay()
    {
        setOpaque(true);
    }

    void setDelayTime(float time) { delayTime = time; repaint(); }
    void setFeedback(float fb) { feedback = fb; repaint(); }
    void setMix(float mx) { mix = mx; repaint(); }

    void paint(juce::Graphics& g) override
    {
        // Background
        g.fillAll(juce::Colours::black);

        // Draw time grid
        g.setColour(juce::Colours::darkgrey);
        auto bounds = getLocalBounds().toFloat();
        float width = bounds.getWidth();
        float height = bounds.getHeight();

        // Draw center horizontal line
        g.drawLine(0, height / 2, width, height / 2, 1.0f);

        // Draw echo visualization
        g.setColour(juce::Colours::lightblue);

        // Original signal
        float centerY = height / 2;
        float amplitude = height * 0.3f;

        // Draw original impulse
        float impulseX = width * 0.1f;
        g.drawLine(impulseX, centerY - amplitude, impulseX, centerY + amplitude, 2.0f);
        g.setFont(12.0f);
        g.drawText("Input", impulseX - 20, centerY + amplitude + 5, 40, 20, juce::Justification::centred);

        // Draw delay time marker
        float delayX = impulseX + width * 0.7f * delayTime;
        g.setColour(juce::Colours::red);
        g.drawLine(delayX, centerY - 5, delayX, centerY + 5, 1.0f);
        g.drawText(juce::String(delayTime, 2) + "s", delayX - 20, centerY - 25, 40, 20, juce::Justification::centred);

        // Draw echoes with feedback decay
        g.setColour(juce::Colours::cyan);
        float currentAmp = amplitude * mix;
        float currentX = delayX;

        for (int i = 0; i < 5; i++)
        {
            if (currentAmp < 2.0f)
                break;

            g.drawLine(currentX, centerY - currentAmp, currentX, centerY + currentAmp, 2.0f);

            // Next echo
            currentAmp *= feedback;
            currentX += (delayX - impulseX);

            if (currentX > width - 10)
                break;
        }

        // Draw feedback indicator
        g.setColour(juce::Colours::white);
        g.drawText("Feedback: " + juce::String(feedback, 2), width - 100, 10, 90, 20, juce::Justification::right);
        g.drawText("Mix: " + juce::String(mix, 2), width - 100, 30, 90, 20, juce::Justification::right);
    }

private:
    float delayTime = 0.3f;
    float feedback = 0.4f;
    float mix = 0.5f;
};

// Main editor class
class _3ff3ctsAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    _3ff3ctsAudioProcessorEditor(_3ff3ctsAudioProcessor&);
    ~_3ff3ctsAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    _3ff3ctsAudioProcessor& audioProcessor;

    // Interface components
    juce::TextButton distortionButton;
    juce::TextButton delayButton;

    // Distortion components
    juce::Component distortionPanel;
    juce::Slider gainSlider;
    juce::Label gainLabel;
    juce::Slider distortionSlider;
    juce::Label distortionLabel;
    juce::ComboBox distortionTypeComboBox;
    juce::Label distortionTypeLabel;
    WaveShapeDisplay waveShapeDisplay;

    // Delay components
    juce::Component delayPanel;
    juce::Slider delayTimeSlider;
    juce::Label delayTimeLabel;
    juce::Slider feedbackSlider;
    juce::Label feedbackLabel;
    juce::Slider mixSlider;
    juce::Label mixLabel;
    DelayDisplay delayDisplay;

    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> distortionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> distortionTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    void showDistortionPanel(bool shouldShow);
    void showDelayPanel(bool shouldShow);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(_3ff3ctsAudioProcessorEditor)
};