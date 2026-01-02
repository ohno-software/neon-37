#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        // Subsequent 37 style colors - white/gray knobs instead of neon teal
        setColour(juce::Slider::thumbColourId, juce::Colours::white);
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::white);
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF3A4A55));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto fill = slider.findColour (juce::Slider::rotarySliderFillColourId);

        // Ensure the knob is circular by using the smaller dimension
        auto size = juce::jmin (width, height) - 8;
        auto centerX = x + width * 0.5f;
        auto centerY = y + height * 0.5f;
        auto rx = centerX - size * 0.5f;
        auto ry = centerY - size * 0.5f;
        auto radius = size * 0.5f;
        
        auto bounds = juce::Rectangle<float> (rx, ry, (float)size, (float)size);

        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = 2.5f;
        auto arcRadius = radius - lineW * 1.5f;

        // Draw background arc (track) - very subtle
        juce::Path backgroundArc;
        backgroundArc.addCentredArc (centerX, centerY, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colours::black.withAlpha(0.3f));
        g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Draw the knob body with a gradient for 3D hardware effect (Moog-style)
        auto knobBounds = bounds.reduced(lineW * 2.0f);
        auto knobRadius = knobBounds.getWidth() * 0.5f;
        
        // Moog-style white/light gray gradient
        juce::ColourGradient grad (juce::Colour(0xFFE8E8E8), centerX, centerY - knobRadius,
                                   juce::Colour(0xFFA0A0A0), centerX, centerY + knobRadius, false);
        g.setGradientFill (grad);
        g.fillEllipse (knobBounds);
        
        // Knob rim - dark outline for contrast
        g.setColour (juce::Colour(0xFF2A2A2A));
        g.drawEllipse (knobBounds, 1.5f);
        
        // Inner shadow/highlight for depth
        g.setColour (juce::Colours::white.withAlpha(0.4f));
        g.drawEllipse (knobBounds.reduced(2.0f), 1.0f);
        
        // Bottom shadow
        g.setColour (juce::Colours::black.withAlpha(0.2f));
        auto shadowBounds = knobBounds.translated(1.0f, 1.5f);
        g.drawEllipse (shadowBounds, 2.0f);

        // Draw the pointer - black line on white knob (Moog style)
        auto pointerLength = knobRadius * 0.7f;
        auto pointerThickness = 2.0f;
        
        juce::Path p;
        p.addRoundedRectangle (-pointerThickness * 0.5f, -knobRadius + 4.0f, pointerThickness, pointerLength, 1.0f);
        p.applyTransform (juce::AffineTransform::rotation (toAngle).translated (centerX, centerY));

        g.setColour (juce::Colour(0xFF1A1A1A));
        g.fillPath (p);

        // Draw tick marks around knob
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        auto numTicks = 11;
        for (int i = 0; i < numTicks; ++i)
        {
            auto angle = rotaryStartAngle + i * (rotaryEndAngle - rotaryStartAngle) / (numTicks - 1);
            auto tickLen = 4.0f;
            auto innerR = radius + 3.0f;
            auto outerR = innerR + tickLen;
            
            g.drawLine(centerX + innerR * std::sin(angle), centerY - innerR * std::cos(angle),
                       centerX + outerR * std::sin(angle), centerY - outerR * std::cos(angle), 1.5f);
        }
    }
};
