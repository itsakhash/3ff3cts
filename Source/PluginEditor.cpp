#include "PluginProcessor.h"
#include "PluginEditor.h"

_3ff3ctsAudioProcessorEditor::_3ff3ctsAudioProcessorEditor(_3ff3ctsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Set up toggle buttons
    distortionButton.setButtonText("Distortion");
    distortionButton.setToggleState(true, juce::dontSendNotification);
    distortionButton.onClick = [this]() {
        distortionButton.setToggleState(true, juce::dontSendNotification);
        delayButton.setToggleState(false, juce::dontSendNotification);
        showDistortionPanel(true);
        showDelayPanel(false);
        };
    addAndMakeVisible(distortionButton);

    delayButton.setButtonText("Delay");
    delayButton.setToggleState(false, juce::dontSendNotification);
    delayButton.onClick = [this]() {
        distortionButton.setToggleState(false, juce::dontSendNotification);
        delayButton.setToggleState(true, juce::dontSendNotification);
        showDistortionPanel(false);
        showDelayPanel(true);
        };
    addAndMakeVisible(delayButton);

    // Set up distortion panel
    addChildComponent(distortionPanel);
    distortionPanel.setVisible(true);

    // Add distortion components to the panel
    distortionPanel.addChildComponent(gainSlider);
    distortionPanel.addChildComponent(gainLabel);
    distortionPanel.addChildComponent(distortionSlider);
    distortionPanel.addChildComponent(distortionLabel);
    distortionPanel.addChildComponent(distortionTypeComboBox);
    distortionPanel.addChildComponent(distortionTypeLabel);
    distortionPanel.addChildComponent(waveShapeDisplay);

    // Configure distortion components
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 20);
    gainSlider.setVisible(true);

    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.setVisible(true);

    distortionSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    distortionSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 20);
    distortionSlider.setVisible(true);

    distortionLabel.setText("Distortion", juce::dontSendNotification);
    distortionLabel.setJustificationType(juce::Justification::centred);
    distortionLabel.setVisible(true);

    distortionTypeComboBox.addItem("Soft Clip", 1);
    distortionTypeComboBox.addItem("Hard Clip", 2);
    distortionTypeComboBox.addItem("Tube", 3);
    distortionTypeComboBox.addItem("Diode", 4);
    distortionTypeComboBox.addItem("Fold", 5);
    distortionTypeComboBox.addItem("Sine", 6);
    distortionTypeComboBox.setVisible(true);

    distortionTypeLabel.setText("Type", juce::dontSendNotification);
    distortionTypeLabel.setJustificationType(juce::Justification::centred);
    distortionTypeLabel.setVisible(true);

    waveShapeDisplay.setVisible(true);

    // Connect distortion parameters
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "gain", gainSlider);

    distortionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "distortion", distortionSlider);

    distortionTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getParameters(), "distortionType", distortionTypeComboBox);

    // Set up distortion display
    distortionSlider.onValueChange = [this]() {
        waveShapeDisplay.setAmount(distortionSlider.getValue());
        };

    distortionTypeComboBox.onChange = [this]() {
        waveShapeDisplay.setDistortionType(distortionTypeComboBox.getSelectedId() - 1);
        };

    waveShapeDisplay.setAmount(distortionSlider.getValue());
    waveShapeDisplay.setDistortionType(distortionTypeComboBox.getSelectedId() - 1);

    // Set up delay panel
    addChildComponent(delayPanel);
    delayPanel.setVisible(false);

    // Add delay components to the panel
    delayPanel.addChildComponent(delayTimeSlider);
    delayPanel.addChildComponent(delayTimeLabel);
    delayPanel.addChildComponent(feedbackSlider);
    delayPanel.addChildComponent(feedbackLabel);
    delayPanel.addChildComponent(mixSlider);
    delayPanel.addChildComponent(mixLabel);
    delayPanel.addChildComponent(delayDisplay);

    // Configure delay components
    delayTimeSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    delayTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 20);
    delayTimeSlider.setVisible(true);

    delayTimeLabel.setText("Time", juce::dontSendNotification);
    delayTimeLabel.setJustificationType(juce::Justification::centred);
    delayTimeLabel.setVisible(true);

    feedbackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 20);
    feedbackSlider.setVisible(true);

    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.setJustificationType(juce::Justification::centred);
    feedbackLabel.setVisible(true);

    mixSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 20);
    mixSlider.setVisible(true);

    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    mixLabel.setVisible(true);

    delayDisplay.setVisible(true);

    // Connect delay parameters
    delayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "delayTime", delayTimeSlider);

    feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "delayFeedback", feedbackSlider);

    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "delayMix", mixSlider);

    // Set up delay display
    delayTimeSlider.onValueChange = [this]() {
        delayDisplay.setDelayTime(delayTimeSlider.getValue());
        };

    feedbackSlider.onValueChange = [this]() {
        delayDisplay.setFeedback(feedbackSlider.getValue());
        };

    mixSlider.onValueChange = [this]() {
        delayDisplay.setMix(mixSlider.getValue());
        };

    delayDisplay.setDelayTime(delayTimeSlider.getValue());
    delayDisplay.setFeedback(feedbackSlider.getValue());
    delayDisplay.setMix(mixSlider.getValue());

    // Set window size
    setSize(500, 500);
}

_3ff3ctsAudioProcessorEditor::~_3ff3ctsAudioProcessorEditor()
{
}

void _3ff3ctsAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Fill background
    g.fillAll(juce::Colours::darkgrey);

    // Add title
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawText("3ff3cts Plugin", getLocalBounds(), juce::Justification::centredTop, true);
}

void _3ff3ctsAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);

    // Position title
    auto titleArea = area.removeFromTop(30);

    // Position toggle buttons at the top
    auto toggleArea = area.removeFromTop(30);
    distortionButton.setBounds(toggleArea.removeFromLeft(toggleArea.getWidth() / 2).reduced(5, 0));
    delayButton.setBounds(toggleArea.reduced(5, 0));

    // Position panels (they occupy the same space - only one visible at a time)
    auto contentArea = area;
    distortionPanel.setBounds(contentArea);
    delayPanel.setBounds(contentArea);

    // Layout for distortion panel
    auto distortionArea = contentArea.reduced(5);

    // Visualization area at the top
    auto vizArea = distortionArea.removeFromTop(150);
    waveShapeDisplay.setBounds(vizArea);

    // Controls below visualization
    auto controlsArea = distortionArea;

    // Two columns for the controls
    auto leftArea = controlsArea.removeFromLeft(controlsArea.getWidth() / 2);
    auto rightArea = controlsArea;

    // Position gain controls in left column
    gainLabel.setBounds(leftArea.removeFromTop(20));
    gainSlider.setBounds(leftArea.removeFromTop(100));

    // Position distortion controls in right column
    distortionLabel.setBounds(rightArea.removeFromTop(20));
    distortionSlider.setBounds(rightArea.removeFromTop(100));

    // Position type controls at the bottom
    auto typeArea = controlsArea.removeFromBottom(50);
    distortionTypeLabel.setBounds(typeArea.removeFromTop(20));

    // Center the combo box
    int comboWidth = 200;
    int comboHeight = 25;
    distortionTypeComboBox.setBounds(
        typeArea.getCentreX() - comboWidth / 2,
        typeArea.getY(),
        comboWidth,
        comboHeight
    );

    // Layout for delay panel
    auto delayArea = contentArea.reduced(5);

    // Visualization area at the top
    auto delayVizArea = delayArea.removeFromTop(150);
    delayDisplay.setBounds(delayVizArea);

    // Controls below visualization
    auto delayControlsArea = delayArea;

    // Three columns for the controls
    auto firstThird = delayControlsArea.removeFromLeft(delayControlsArea.getWidth() / 3);
    auto secondThird = delayControlsArea.removeFromLeft(delayControlsArea.getWidth() / 2);
    auto thirdThird = delayControlsArea;

    // Position delay time controls
    delayTimeLabel.setBounds(firstThird.removeFromTop(20));
    delayTimeSlider.setBounds(firstThird.removeFromTop(100));

    // Position feedback controls
    feedbackLabel.setBounds(secondThird.removeFromTop(20));
    feedbackSlider.setBounds(secondThird.removeFromTop(100));

    // Position mix controls
    mixLabel.setBounds(thirdThird.removeFromTop(20));
    mixSlider.setBounds(thirdThird.removeFromTop(100));
}

void _3ff3ctsAudioProcessorEditor::showDistortionPanel(bool shouldShow)
{
    distortionPanel.setVisible(shouldShow);

    // Make child components visible/invisible (JUCE sometimes needs this)
    gainSlider.setVisible(shouldShow);
    gainLabel.setVisible(shouldShow);
    distortionSlider.setVisible(shouldShow);
    distortionLabel.setVisible(shouldShow);
    distortionTypeComboBox.setVisible(shouldShow);
    distortionTypeLabel.setVisible(shouldShow);
    waveShapeDisplay.setVisible(shouldShow);
}

void _3ff3ctsAudioProcessorEditor::showDelayPanel(bool shouldShow)
{
    delayPanel.setVisible(shouldShow);

    // Make child components visible/invisible
    delayTimeSlider.setVisible(shouldShow);
    delayTimeLabel.setVisible(shouldShow);
    feedbackSlider.setVisible(shouldShow);
    feedbackLabel.setVisible(shouldShow);
    mixSlider.setVisible(shouldShow);
    mixLabel.setVisible(shouldShow);
    delayDisplay.setVisible(shouldShow);
}