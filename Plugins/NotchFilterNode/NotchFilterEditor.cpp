/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "NotchFilterEditor.h"
#include "NotchFilterNode.h"
#include <stdio.h>


NotchFilterEditor::NotchFilterEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{
    desiredWidth = 150;

    lastLowCutString = " ";
    lastHighCutString = " ";

    highCutLabel = new Label("Centre Freq label", "Centre Freq:");
    highCutLabel->setBounds(10,65,80,20);
    highCutLabel->setFont(Font("Small Text", 12, Font::plain));
    highCutLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(highCutLabel);

    lowCutLabel = new Label("Bandwidth label", "Bandwidth:");
    lowCutLabel->setBounds(10,25,80,20);
    lowCutLabel->setFont(Font("Small Text", 12, Font::plain));
    lowCutLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(lowCutLabel);

    lowCutValue = new Label("bandwidth value", lastLowCutString);
    lowCutValue->setBounds(15,42,60,18);
    lowCutValue->setFont(Font("Default", 15, Font::plain));
    lowCutValue->setColour(Label::textColourId, Colours::white);
    lowCutValue->setColour(Label::backgroundColourId, Colours::grey);
    lowCutValue->setEditable(true);
    lowCutValue->addListener(this);
    lowCutValue->setTooltip("Set the bandwidth for the selected channels");
    addAndMakeVisible(lowCutValue);

    highCutValue = new Label("centre freq value", lastHighCutString);
    highCutValue->setBounds(15,82,60,18);
    highCutValue->setFont(Font("Default", 15, Font::plain));
    highCutValue->setColour(Label::textColourId, Colours::white);
    highCutValue->setColour(Label::backgroundColourId, Colours::grey);
    highCutValue->setEditable(true);
    highCutValue->addListener(this);
    highCutValue->setTooltip("Set the centre freq for the selected channels");
    addAndMakeVisible(highCutValue);

    applyFilterOnADC = new UtilityButton("+ADCs",Font("Default", 10, Font::plain));
    applyFilterOnADC->addListener(this);
    applyFilterOnADC->setBounds(90,70,40,18);
    applyFilterOnADC->setClickingTogglesState(true);
    applyFilterOnADC->setTooltip("When this button is off, ADC channels will not be filtered");
    addAndMakeVisible(applyFilterOnADC);

    applyFilterOnChan = new UtilityButton("+CH",Font("Default", 10, Font::plain));
    applyFilterOnChan->addListener(this);
    applyFilterOnChan->setBounds(95,95,30,18);
    applyFilterOnChan->setClickingTogglesState(true);
    applyFilterOnChan->setToggleState(true, dontSendNotification);
    applyFilterOnChan->setTooltip("When this button is off, selected channels will not be filtered");
    addAndMakeVisible(applyFilterOnChan);

}

NotchFilterEditor::~NotchFilterEditor()
{

}

void NotchFilterEditor::setDefaults(double lowCut, double highCut)
{
    lastHighCutString = String(roundFloatToInt(highCut));
    lastLowCutString = String(roundFloatToInt(lowCut));

    resetToSavedText();
}

void NotchFilterEditor::resetToSavedText()
{
    highCutValue->setText(lastHighCutString, dontSendNotification);
    lowCutValue->setText(lastLowCutString, dontSendNotification);
}


void NotchFilterEditor::labelTextChanged(Label* label)
{
    NotchFilterNode* fn = (NotchFilterNode*) getProcessor();

    Value val = label->getTextValue();
    double requestedValue = double(val.getValue());

    if (requestedValue < 0.01 || requestedValue > 10000)
    {
        CoreServices::sendStatusMessage("Value out of range.");

        if (label == highCutValue)
        {
            label->setText(lastHighCutString, dontSendNotification);
            lastHighCutString = label->getText();
        }
        else
        {
            label->setText(lastLowCutString, dontSendNotification);
            lastLowCutString = label->getText();
        }

        return;
    }

    Array<int> chans = getActiveChannels();

    // This needs to change, since there's not enough feedback about whether
    // or not individual channel settings were altered:

    for (int n = 0; n < chans.size(); n++)
    {

        if (label == highCutValue)
        {
            double minVal = fn->getLowCutValueForChannel(chans[n]);

            if (requestedValue > minVal)
            {
                fn->setCurrentChannel(chans[n]);
                fn->setParameter(1, requestedValue);
            }

            lastHighCutString = label->getText();

        }
        else
        {
            double maxVal = fn->getHighCutValueForChannel(chans[n]);

            if (requestedValue < maxVal)
            {
                fn->setCurrentChannel(chans[n]);
                fn->setParameter(0, requestedValue);
            }

            lastLowCutString = label->getText();
        }

    }

}

void NotchFilterEditor::channelChanged (int channel, bool /*newState*/)
{
    NotchFilterNode* fn = (NotchFilterNode*) getProcessor();

    highCutValue->setText (String (fn->getHighCutValueForChannel (channel)), dontSendNotification);
    lowCutValue->setText  (String (fn->getLowCutValueForChannel  (channel)), dontSendNotification);
    applyFilterOnChan->setToggleState (fn->getBypassStatusForChannel (channel), dontSendNotification);

}

void NotchFilterEditor::buttonEvent(Button* button)
{

    if (button == applyFilterOnADC)
    {
        NotchFilterNode* fn = (NotchFilterNode*) getProcessor();
        fn->setApplyOnADC(applyFilterOnADC->getToggleState());

    }
    else if (button == applyFilterOnChan)
    {
        NotchFilterNode* fn = (NotchFilterNode*) getProcessor();

        Array<int> chans = getActiveChannels();

        for (int n = 0; n < chans.size(); n++)
        {
            float newValue = button->getToggleState() ? 1.0 : 0.0;

            fn->setCurrentChannel(chans[n]);
            fn->setParameter(2, newValue);
        }
    }
}


void NotchFilterEditor::saveCustomParameters(XmlElement* xml)
{

    xml->setAttribute("Type", "FilterEditor");

    lastHighCutString = highCutValue->getText();
    lastLowCutString = lowCutValue->getText();

    XmlElement* textLabelValues = xml->createNewChildElement("VALUES");
    textLabelValues->setAttribute("CentreFreq",lastHighCutString);
    textLabelValues->setAttribute("Bandwidth",lastLowCutString);
    textLabelValues->setAttribute("ApplyToADC",	applyFilterOnADC->getToggleState());
}

void NotchFilterEditor::loadCustomParameters(XmlElement* xml)
{

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("VALUES"))
        {
            lastHighCutString = xmlNode->getStringAttribute("CentreFreq", lastHighCutString);
            lastLowCutString = xmlNode->getStringAttribute("Bandwidth", lastLowCutString);
            resetToSavedText();

            applyFilterOnADC->setToggleState(xmlNode->getBoolAttribute("ApplyToADC",false), sendNotification);
        }
    }


}
