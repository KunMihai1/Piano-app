/*
  ==============================================================================

    MidiNotesTableModel.cpp
    Created: 15 Jul 2025 8:46:13pm
    Author:  Kisuke

  ==============================================================================
*/

#include "MidiNotesTableModel.h"

MidiNotesTableModel::MidiNotesTableModel(const juce::MidiMessageSequence& seq, int ch, std::unordered_map<int, MidiChangeInfo>& map): channel{ch}, changesMap{&map}
{
    for (int i = 0; i < seq.getNumEvents(); ++i)
    {
        const auto* event = seq.getEventPointer(i);
        if (event != nullptr && event->message.isNoteOn())
        {
            int indexInNoteOn = noteOnEvents.size();
            noteOnEvents.push_back({ event, i, indexInNoteOn });
            //originalIndexToRowMap[i] = noteOnEvents.size()-1;
        }
    }
}

int MidiNotesTableModel::getNumRows()
{
    return static_cast<int>(noteOnEvents.size());
}

void MidiNotesTableModel::sortOrderChanged(int newSortColumnId, bool isForwards)
{
    if (newSortColumnId == 1)
    {
        std::stable_sort(noteOnEvents.begin(), noteOnEvents.end(),
            [isForwards](const EventWithIndex& first, const EventWithIndex& second)
            {
                if (isForwards)
                    return first.event->message.getNoteNumber() < second.event->message.getNoteNumber();
                else return first.event->message.getNoteNumber() > second.event->message.getNoteNumber();
            });
    }
    else if (newSortColumnId == 2)
    {
        std::stable_sort(noteOnEvents.begin(), noteOnEvents.end(),
            [isForwards](const EventWithIndex& first, const EventWithIndex& second)
            {
                if (isForwards)
                    return first.event->message.getTimeStamp() < second.event->message.getTimeStamp();
                else return first.event->message.getTimeStamp() > second.event->message.getTimeStamp();
            });
    }
    else if (newSortColumnId == 3)
    {
        std::stable_sort(noteOnEvents.begin(), noteOnEvents.end(),
            [isForwards](const EventWithIndex& first, const EventWithIndex& second)
            {
                if (isForwards)
                    return first.event->message.getVelocity() < second.event->message.getVelocity();
                else return first.event->message.getVelocity() > second.event->message.getVelocity();
            });
    }

    if (refreshData)
        refreshData();
}

void MidiNotesTableModel::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);
    else g.fillAll(juce::Colours::lightgrey);
}

void MidiNotesTableModel::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool isSelected)
{
    if (columnId == 4)
    {
        if (rowNumber < 0 || rowNumber >= noteOnEvents.size()) return;
        auto* event = noteOnEvents[rowNumber].event;

        if (event == nullptr)
            return;

        const auto& msg = event->message;
        juce::String text;

        if (msg.isNoteOn())
        {
            switch (columnId)
            {
            case 1: text = juce::MidiMessage::getMidiNoteName(msg.getNoteNumber(), true, true, 4); break;
            case 2: text = juce::String(msg.getTimeStamp(), 6); break;
            case 3: text = juce::String(msg.getVelocity()); break;
            case 4: text = juce::String(channel); break;
            default: break;
            }
        }

        g.setColour(juce::Colours::black);
        g.setFont(14.0f);
        g.drawText(text, 2, 0, width - 4, height, juce::Justification::centredLeft);
        g.setColour(juce::Colours::lightgrey);
        g.drawRect(0, 0, width, height);
    }
}


juce::Component* MidiNotesTableModel::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate)
{
    if (columnId == 4)
        return nullptr;

    if (rowNumber < 0 || rowNumber >= noteOnEvents.size())
        return nullptr;

    const auto* event = noteOnEvents[rowNumber].event;
    if (event == nullptr)
        return nullptr;

    int originalIndex = noteOnEvents[rowNumber].originalIndex;

    //auto* label = dynamic_cast<juce::Label*> (existingComponentToUpdate);

    auto* label = dynamic_cast<SelectableLabel*> (existingComponentToUpdate);


    if (label == nullptr)
    {
        label = new SelectableLabel{};
        label->setWantsKeyboardFocus(true);
        label->setEditable(false, true, false);
    }

    label->onEditorHide = [this, rowNumber, columnId, label, originalIndex]()
    {
        auto* e = noteOnEvents[rowNumber].event;
        if (e == nullptr)
            return;

        auto oldMsg = e->message;
        juce::MidiMessage newMsg;

        if (columnId == 1)
        {
            int newNoteNumber = getMidiNoteNumberFromName(label->getText());
            if (newNoteNumber == -1)
            {
                label->setText(juce::MidiMessage::getMidiNoteName(oldMsg.getNoteNumber(), true, true, 4), juce::dontSendNotification);
                return;
            }
            newMsg = juce::MidiMessage::noteOn(oldMsg.getChannel(), newNoteNumber, (juce::uint8)oldMsg.getVelocity());
            newMsg.setTimeStamp(oldMsg.getTimeStamp());

            if (e->noteOffObject != nullptr)
            {
                auto* noteOffEvent = e->noteOffObject;
                juce::MidiMessage offMsg = juce::MidiMessage::noteOff(
                    noteOffEvent->message.getChannel(),
                    newNoteNumber
                );
                offMsg.setTimeStamp(noteOffEvent->message.getTimeStamp());
                noteOffEvent->message = offMsg;
            }
        }
        else if (columnId == 2)
        {
            double newTime = label->getText().getDoubleValue();
            if (newTime < 0)
            {
                label->setText(juce::String(oldMsg.getTimeStamp(), 6), juce::dontSendNotification);
                return;
            }
            newMsg = juce::MidiMessage::noteOn(oldMsg.getChannel(), oldMsg.getNoteNumber(), oldMsg.getVelocity());
            newMsg.setTimeStamp(newTime);
            if (e->noteOffObject != nullptr)
            {
                auto* noteOffEvent = e->noteOffObject;
                double oldNoteOnTime = oldMsg.getTimeStamp();
                double oldNoteOffTime = noteOffEvent->message.getTimeStamp();
                double duration = oldNoteOffTime - oldNoteOnTime;

                double newNoteOffTime = newTime + duration;
                noteOffEvent->message.setTimeStamp(newNoteOffTime);
            }
            
        }
        else if (columnId == 3)
        {
            int newVelocity = label->getText().getIntValue();
            if (newVelocity < 0 || newVelocity>127)
            {
                label->setText(juce::String(oldMsg.getVelocity()), juce::dontSendNotification);
                return;
            }
            newMsg = juce::MidiMessage::noteOn(oldMsg.getChannel(), oldMsg.getNoteNumber(), (juce::uint8)newVelocity);

            newMsg.setTimeStamp(oldMsg.getTimeStamp());
        }

        if (changesMap->find(originalIndex) != changesMap->end())
        {
            auto& current = (*changesMap)[originalIndex];
            handleMapExistingCase(newMsg, current);
        }
        else
        {
            MidiChangeInfo current;
            handleMapNonExistingCase(oldMsg, newMsg, current);
            (*changesMap)[originalIndex] = current;
        }

        const_cast<juce::MidiMessageSequence::MidiEventHolder*>(e)->message = newMsg;

        if (!areMidiMessagesEqual(newMsg, oldMsg) && onUpdate)
            onUpdate(rowNumber);
    };

    label->onClick = [this, rowNumber]()
    {
        DBG("Label clicked: row " + juce::String(rowNumber));
        if (onRequestSelectRow)
            onRequestSelectRow(rowNumber);
    };

    const auto& message = event->message;
    if (message.isNoteOn())
    {
        if (columnId == 1)
        {
            auto noteName = juce::MidiMessage::getMidiNoteName(message.getNoteNumber(), true, true, 4);
            label->setText(noteName, juce::dontSendNotification);
        }
        else if (columnId == 2)
        {
            auto noteTime = juce::String(message.getTimeStamp());
            label->setText(noteTime, juce::dontSendNotification);
        }
        else if (columnId == 3)
        {
            auto noteVelocity = juce::String(message.getVelocity());
            label->setText(noteVelocity, juce::dontSendNotification);
        }
    }
    label->setColour(juce::Label::textColourId,juce::Colours::darkgrey);

    return label;
}


int MidiNotesTableModel::getMidiNoteNumberFromName(const juce::String& noteName, int octaveNoteMiddleC)
{
    static const juce::StringArray noteNames{
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    juce::String name = noteName.trim().toUpperCase();
    if (name.length() < 2)
        return -1;

    int digit = findFirstDigitName(noteName);
    if (digit == -1)
        return -1;

    juce::String notePart = name.substring(0, digit);
    juce::String octavePart = name.substring(digit);

    if (!octavePart.containsOnly("0123456789-"))
        return -1;

    int noteIndex = noteNames.indexOf(notePart);
    int octave = octavePart.getIntValue();


    if (noteIndex == -1)
        return -1;

    int midiNumber = (octave - octaveNoteMiddleC + 5) * 12 + noteIndex;

    if (midiNumber < 0 || midiNumber>127)
        return -1;

    return midiNumber;
}

int MidiNotesTableModel::findFirstDigitName(const juce::String& name)
{
    for (int i = 0; i < name.length(); i++)
    {
        if (juce::CharacterFunctions::isDigit(name[i]) || name[i]=='-')
            return i;
    }
    return -1;
}

void MidiNotesTableModel::handleMapNonExistingCase(juce::MidiMessage& oldMessage, juce::MidiMessage& newMessage, MidiChangeInfo& information)
{
    information.oldNumber = oldMessage.getNoteNumber();
    information.oldTimeStamp = oldMessage.getTimeStamp();
    information.oldVelocity = oldMessage.getVelocity();

    information.newNumber = newMessage.getNoteNumber();
    information.newTimeStamp = newMessage.getTimeStamp();
    information.newVelocity = newMessage.getVelocity();
}

void MidiNotesTableModel::handleMapExistingCase(juce::MidiMessage& newMessage, MidiChangeInfo& information)
{
    information.newNumber = newMessage.getNoteNumber();
    information.newTimeStamp = newMessage.getTimeStamp();
    information.newVelocity = newMessage.getVelocity();
}

bool MidiNotesTableModel::areMidiMessagesEqual(const juce::MidiMessage& a, const juce::MidiMessage& b)
{
    if (a.getTimeStamp() != b.getTimeStamp())
        return false;

    if (a.getRawDataSize() != b.getRawDataSize())
        return false;

    return std::memcmp(a.getRawData(), b.getRawData(), a.getRawDataSize()) == 0; // comparing bytes
}

void MidiNotesTableModel::refreshVectorFromSequence(const juce::MidiMessageSequence& seq)
{
    noteOnEvents.clear();
    //originalIndexToRowMap.clear();
    for (int i = 0; i < seq.getNumEvents(); i++)
    {
        const auto* event = seq.getEventPointer(i);
        if (event != nullptr && event->message.isNoteOn())
        {
            noteOnEvents.push_back({ event, i, static_cast<int>(noteOnEvents.size())});
            //originalIndexToRowMap[i] = static_cast<int>(noteOnEvents.size()) - 1;
        }
    }
}

int MidiNotesTableModel::getOriginalIndexFromRow(int row)
{
    if (row >= 0 && row < noteOnEvents.size())
        return noteOnEvents[row].originalIndex;
    return -1;
}

int MidiNotesTableModel::getChangesMapIndexFromRow(int row)
{
    if (row >= 0 && row < noteOnEvents.size())
        return noteOnEvents[row].indexForChangesMap;
    return -1;
}

double MidiNotesTableModel::getFirstNoteOnTimeStamps()
{
    if (noteOnEvents.empty())
        return -1;

    return noteOnEvents[0].event->message.getTimeStamp();
}

double MidiNotesTableModel::getPreviousNoteOnTimeStamp(int currentIndex)
{
    if (noteOnEvents.empty() || currentIndex==-1)
        return -1;

    if (currentIndex == 0)
        return getFirstNoteOnTimeStamps();

    else return noteOnEvents[currentIndex - 1].event->message.getTimeStamp();
}

double MidiNotesTableModel::getCurrentNoteOnTimeStamp(int currentIndex)
{
    if (noteOnEvents.empty() || currentIndex == -1)
        return -1;

    return noteOnEvents[currentIndex].event->message.getTimeStamp();
}

double MidiNotesTableModel::getNextNoteOnTimeStamp(int currentIndex)
{
    if (noteOnEvents.empty() || currentIndex == -1)
        return -1;

    if (currentIndex >= noteOnEvents.size())
        return -1;

    if (currentIndex == noteOnEvents.size() - 1)
        return noteOnEvents[currentIndex].event->message.getTimeStamp();

    return noteOnEvents[currentIndex + 1].event->message.getTimeStamp();
}

std::vector<MidiNotesTableModel::EventWithIndex> MidiNotesTableModel::getEvents()
{
    return noteOnEvents;
}

int MidiNotesTableModel::getRowFromTime(double currentTime)
{
    if (noteOnEvents.empty())
        return -1;

    int left = 0;
    int right = static_cast<int>(noteOnEvents.size()) - 1;
    int result = -1;

    while (left <= right)
    {
        int middle = (left + right) / 2;
        double middleTime = noteOnEvents[middle].event->message.getTimeStamp();

        if (middleTime <= currentTime)
        {
            result = middle;    
            left = middle + 1;  
        }
        else
        {
            right = middle - 1;
        }
    }

    return result; 
    }

void MidiNotesTableModel::updateCurrentRowBasedOnTime(double currentTime)
{
    DBG("time:" + juce::String(currentTime));
    int toHighlitRow = getRowFromTime(currentTime);

    if (highlightedRow != toHighlitRow)
    {
        highlightedRow = toHighlitRow;
        if (onMidPlayRepaint)
            onMidPlayRepaint(highlightedRow);
    }
}