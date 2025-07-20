/*
  ==============================================================================

    TrackIOHelper.cpp
    Created: 17 Jul 2025 3:29:02pm
    Author:  Kisuke

  ==============================================================================
*/

#include "IOHelper.h"

juce::File IOHelper::getFolder(const juce::String& name)
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(name);

}

juce::File IOHelper::getFile(const juce::String& name)
{
    auto appDataFolder = getFolder("Piano Synth2");

     return appDataFolder.getChildFile(name);
}

void TrackIOHelper::saveToFile(const juce::File& file, const std::unordered_map<juce::String, std::vector<TrackEntry>>& groupedTracks)
{
    juce::Array<juce::var> foldersArray;

    for (auto& [folderName, tracks] : groupedTracks)
    {
        // Create DynamicObject and wrap once in var
        auto* folderObj = new juce::DynamicObject{};
        juce::var folderVar(folderObj);

        folderObj->setProperty("folderName", folderName);

        if (!tracks.empty())
            folderObj->setProperty("filePath", tracks[0].file.getFullPathName());
        else
            folderObj->setProperty("filePath", juce::String());

        juce::Array<juce::var> trackArray;

        for (auto& tr : tracks)
        {
            auto* trackObj = new juce::DynamicObject{};
            juce::var trackVar(trackObj);

            trackObj->setProperty("trackIndex", tr.trackIndex);
            trackObj->setProperty("displayName", tr.displayName);
            trackObj->setProperty("uuid", tr.uuid.toString());

            auto* changesObj = new juce::DynamicObject();
            juce::var changesVar(changesObj);

            if (!tr.changesMap.empty())
            {
                for (const auto& [row, change] : tr.changesMap)
                {
                    auto* changeObj = new juce::DynamicObject();
                    juce::var changeVar(changeObj);

                    changeObj->setProperty("oldNumber", change.oldNumber);
                    changeObj->setProperty("oldTimeStamp", change.oldTimeStamp);
                    changeObj->setProperty("oldVelocity", change.oldVelocity);

                    changeObj->setProperty("newNumber", change.newNumber);
                    changeObj->setProperty("newTimeStamp", change.newTimeStamp);
                    changeObj->setProperty("newVelocity", change.newVelocity);

                    changesObj->setProperty(juce::String(row), changeVar);

                }

                trackObj->setProperty("Changes", changesVar);
            }

            trackArray.add(trackVar);
        }

        folderObj->setProperty("Tracks", trackArray);
        foldersArray.add(folderVar);
    }

    juce::var jsonVar(foldersArray);
    juce::String jsonString = juce::JSON::toString(jsonVar);
    file.replaceWithText(jsonString);
}

void TrackIOHelper::loadFromFile(const juce::File& fileParam, std::unordered_map<juce::String, std::vector<TrackEntry>>& groupedTracks, std::vector<juce::String>& groupedTrackKeys)
{
    if (!fileParam.existsAsFile())
        return;

    juce::String jsonString = fileParam.loadFileAsString();
    juce::var jsonVar = juce::JSON::parse(jsonString);

    if (!jsonVar.isArray())
        return;

    groupedTrackKeys.clear();
    groupedTracks.clear();

    juce::Array<juce::var>* foldersArray = jsonVar.getArray();

    for (auto& item : *foldersArray)
    {
        auto* folderObj = item.getDynamicObject();
        if (folderObj == nullptr)
            continue;

        juce::String filePath = folderObj->getProperty("filePath").toString();
        juce::String folderName = folderObj->getProperty("folderName").toString();

        if (filePath.isEmpty())
        {
            // Empty folder, add folder name and an empty vector of tracks
            groupedTrackKeys.push_back(folderName);
            groupedTracks[folderName] = {};
            continue; // move on to next folder
        }

        juce::File file{ filePath };

        if (!file.existsAsFile())
            continue;

        groupedTrackKeys.push_back(folderName);
        groupedTracks[folderName] = {};

        juce::var tracksVar = folderObj->getProperty("Tracks");

        if (!tracksVar.isArray())
            continue;

        juce::Array<juce::var>* trackArray = tracksVar.getArray();

        juce::FileInputStream inputStream(file);
        if (!inputStream.openedOk())
            continue;

        juce::MidiFile midiFile;
        if (!midiFile.readFrom(inputStream))
            continue;


        double originalBpm = getOriginalBpmFromFile(midiFile);

        convertTicksToSeconds(midiFile, originalBpm);

        for (auto& trackItem : *trackArray)
        {
            auto* trackObj = trackItem.getDynamicObject();
            if (trackObj == nullptr)
                continue;

            int trackIndex = (int)trackObj->getProperty("trackIndex");
            juce::String displayName = trackObj->getProperty("displayName").toString();

            if (trackIndex >= midiFile.getNumTracks())
                continue;

            auto* sequence = midiFile.getTrack(trackIndex);
            if (sequence == nullptr)
                continue;

            juce::String uuidString = trackObj->getProperty("uuid").toString();

            TrackEntry tr;
            tr.file = file;
            tr.trackIndex = trackIndex;
            tr.displayName = displayName;
            tr.sequence = *sequence;
            tr.sequence.updateMatchedPairs();
            tr.originalBPM = originalBpm;
            tr.folderName = folderName;

            if (foundPercussion(sequence))
                tr.type = TrackType::Percussion;
            else tr.type = TrackType::Melodic;

            if (uuidString.isNotEmpty())
                tr.uuid = juce::Uuid(uuidString);
            else tr.uuid = TrackEntry::generateUUID();


            auto* changesDynamicObj = trackObj->getProperty("Changes").getDynamicObject();

            if (changesDynamicObj != nullptr)
            {
                
                for (int i = 0; i < changesDynamicObj->getProperties().size(); ++i)
                {
                    juce::Identifier key = changesDynamicObj->getProperties().getName(i);
                    juce::var changeVar = changesDynamicObj->getProperties().getValueAt(i);
                    auto* changeObj = changeVar.getDynamicObject();
                    if (changeObj == nullptr)
                        continue;

                    MidiChangeInfo changeInfo;

                    changeInfo.oldNumber= (int)changeObj->getProperty("oldNumber");
                    changeInfo.oldTimeStamp = (double)changeObj->getProperty("oldTimeStamp");
                    changeInfo.oldVelocity = (int)changeObj->getProperty("oldVelocity");

                    changeInfo.newNumber = (int)changeObj->getProperty("newNumber");
                    changeInfo.newTimeStamp = (double)changeObj->getProperty("newTimeStamp");
                    changeInfo.newVelocity = (int)changeObj->getProperty("newVelocity");

                    int row = key.toString().getIntValue();
                    tr.changesMap[row] = changeInfo;
                }
            }

            if (!tr.changesMap.empty())
            {
                applyChangesToASequence(tr.sequence, tr.changesMap);
            }

            groupedTracks[folderName].push_back(tr);
        }
    }
}

bool TrackIOHelper::foundPercussion(const juce::MidiMessageSequence* sequence)
{
    bool perc = false;
    for (int i = 0; i < sequence->getNumEvents(); ++i)
    {
        auto msg = sequence->getEventPointer(i)->message;
        if (msg.isNoteOnOrOff() && msg.getChannel() == 10)
        {
            perc = true;
            break;
        }
    }
    return perc;
}

double TrackIOHelper::getOriginalBpmFromFile(const juce::MidiFile& midiFile)
{
    if (midiFile.getNumTracks() == 0)
        return 120.0;

    auto* firstTrack = midiFile.getTrack(0);
    if (firstTrack == nullptr)
        return 120.0;

    for (int i = 0; i < firstTrack->getNumEvents(); ++i)
    {
        const auto& msg = firstTrack->getEventPointer(i)->message;
        if (msg.isTempoMetaEvent())
        {
            double bpm;
            if (msg.getTempoSecondsPerQuarterNote() > 0.0)
                bpm = 60.0 / msg.getTempoSecondsPerQuarterNote();
            else bpm = 120.0;
            return bpm;
        }
    }
    return 120.0;
}

void TrackIOHelper::convertTicksToSeconds(juce::MidiFile& midiFile, double bpm)
{
    int tpqn = midiFile.getTimeFormat();
    if (tpqn <= 0)
        tpqn = 960;

    const double defaultTempoBPM = bpm; 
    const double secondsPerQuarterNote = 60.0 / defaultTempoBPM;
    const double secondsPerTick = secondsPerQuarterNote / double(tpqn);

    for (int t = 0; t < midiFile.getNumTracks(); ++t)
    {
        if (auto* seq = midiFile.getTrack(t))
        {
            for (int e = 0; e < seq->getNumEvents(); ++e)
            {
                auto& msg = seq->getEventPointer(e)->message;
                double ticks = msg.getTimeStamp();
                msg.setTimeStamp(ticks * secondsPerTick);
            }
        }
    }
}

void TrackIOHelper::applyChangesToASequence(juce::MidiMessageSequence& sequence, const std::unordered_map<int, MidiChangeInfo>& changesMap)
{
    for (const auto& [row, changeInfo] : changesMap)
    {
        if (row < 0 || row >= sequence.getNumEvents())
            continue;

        auto* eventHolder = sequence.getEventPointer(row);
        if (eventHolder == nullptr)
            continue;

        auto& msg = eventHolder->message;

        if (msg.isNoteOnOrOff())
        {
            int channel = msg.getChannel();
            bool isNoteOn = msg.isNoteOn();
            juce::MidiMessage newMsg;
            if (isNoteOn)
                newMsg = juce::MidiMessage::noteOn(channel, changeInfo.newNumber, (juce::uint8)changeInfo.newVelocity);
            else
                newMsg = juce::MidiMessage::noteOff(channel, changeInfo.newNumber);


            newMsg.setTimeStamp(changeInfo.newTimeStamp);

            eventHolder->message = newMsg;
        }

    }

    int eventIndexToCheck = 544;

    if (eventIndexToCheck < sequence.getNumEvents())
    {
        auto* eventHolder = sequence.getEventPointer(eventIndexToCheck);
        if (eventHolder != nullptr)
        {
            auto& msg = eventHolder->message;
            DBG("Event " << eventIndexToCheck << ": note number = " << msg.getNoteNumber()
                << ", time stamp = " << msg.getTimeStamp()
                << ", velocity = " << (int)msg.getVelocity());
        }
    }
    else
    {
        DBG("Event index " << eventIndexToCheck << " is out of range");
    }
}