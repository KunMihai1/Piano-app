#include <juce_core/juce_core.h>
#include "midiDevicesDB.h"
#include "FileSystemInterface.h"

class MockFileSystem : public IFileSystem
{
public:
    juce::File getJsonFile() override {
        return juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("mock.json");
    }
    void writeFile(const juce::File&, const juce::String& text) override { lastWritten = text; }
    juce::String readFile(const juce::File&) override { return lastWritten; }

    juce::String lastWritten;
};

class TestableMidiDevicesDB : public MidiDevicesDataBase
{
public:
    TestableMidiDevicesDB(IFileSystem& fs): MidiDevicesDataBase(fs)
    {
        fileSystem = &fs;

        if (!jsonData.isObject())
            jsonData = juce::var(new juce::DynamicObject());
    }

    void saveJsonFile() override
    {
        if (fileSystem)
            fileSystem->writeFile(fileSystem->getJsonFile(), juce::JSON::toString(jsonData));
        else
            MidiDevicesDataBase::saveJsonFile();
    }

    void loadJsonFile() override
    {
        if (fileSystem)
        {
            juce::String jsonString = fileSystem->readFile(fileSystem->getJsonFile());
            jsonData = juce::JSON::parse(jsonString);
        }
        else
            MidiDevicesDataBase::loadJsonFile();

        if (!jsonData.isObject())
            jsonData = juce::var(new juce::DynamicObject());
    }

private:
    IFileSystem* fileSystem = nullptr;
};

class MidiDevicesDBUnitTest : public juce::UnitTest
{
public:
    MidiDevicesDBUnitTest() : juce::UnitTest("MidiDevicesDataBase", "Unit") {}

    void runTest() override
    {
        MockFileSystem mockFS;
        TestableMidiDevicesDB db(mockFS);

        testAddDeviceAndQuery(db, mockFS);
        testDuplicateDeviceIgnored(db);
        testUpdateDevice(db);
        testUpdateWithEmptyName(db);
        testQueryNonExistentDevices(db);
        testAddMultipleDevices(db);
        testEdgeCasesEmptyVidPid(db);
        testSaveAndLoadJsonInMemory(db, mockFS);
    }

private:

    void testAddDeviceAndQuery(TestableMidiDevicesDB& db, MockFileSystem& mockFS)
    {
        beginTest("Add device and query it + save json and expect it to be there");

        db.addDeviceJson("1234", "0001", "Test Device", 61);

        expect(db.deviceExists("1234", "0001") == true);
        expect(db.getNrKeysPidVid("1234", "0001") == 61);
        expect(db.getDeviceName("1234", "0001") == "Test Device");

        db.saveJsonFile();
        expect(mockFS.lastWritten.contains("Test Device"));
    }

    void testDuplicateDeviceIgnored(TestableMidiDevicesDB& db)
    {
        beginTest("Duplicate device is ignored");

        db.addDeviceJson("1234", "0001", "Should Not Overwrite", 88);
        expect(db.getDeviceName("1234", "0001") == "Test Device");
        expect(db.getNrKeysPidVid("1234", "0001") == 61);
    }

    void testUpdateDevice(TestableMidiDevicesDB& db)
    {
        beginTest("Update device");

        db.updateDeviceJson("1234", "0001", "Updated Device", 76);
        expect(db.getDeviceName("1234", "0001") == "Updated Device");
        expect(db.getNrKeysPidVid("1234", "0001") == 76);
    }

    void testUpdateWithEmptyName(TestableMidiDevicesDB& db)
    {
        beginTest("Update with empty name keeps old name");

        db.updateDeviceJson("1234", "0001", "", 88);
        expect(db.getDeviceName("1234", "0001") == "Updated Device");
        expect(db.getNrKeysPidVid("1234", "0001") == 88);
    }

    void testQueryNonExistentDevices(TestableMidiDevicesDB& db)
    {
        beginTest("Query non-existent devices");

        expect(db.deviceExists("9999", "0000") == false);
        expect(db.getNrKeysPidVid("9999", "0000") == -1);
        expect(db.getDeviceName("9999", "0000") == "");
    }

    void testAddMultipleDevices(TestableMidiDevicesDB& db)
    {
        beginTest("Add multiple devices");

        db.addDeviceJson("abcd", "0010", "Another Device", 49);
        db.addDeviceJson("abcd", "0020", "Yet Another Device", 61);

        expect(db.deviceExists("abcd", "0010") == true);
        expect(db.deviceExists("abcd", "0020") == true);
        expect(db.getNrKeysPidVid("abcd", "0010") == 49);
        expect(db.getNrKeysPidVid("abcd", "0020") == 61);
    }

    void testEdgeCasesEmptyVidPid(TestableMidiDevicesDB& db)
    {
        beginTest("Edge cases: empty VID or PID");

        db.addDeviceJson("", "0001", "Empty VID", 61);
        db.addDeviceJson("1234", "", "Empty PID", 61);

        expect(db.deviceExists("", "0001") == false);
        expect(db.deviceExists("1234", "") == false);
    }

    void testSaveAndLoadJsonInMemory(TestableMidiDevicesDB& db, MockFileSystem& mockFS)
    {
        beginTest("Save and load JSON in memory");

        db.saveJsonFile();
        DBG("it has:" + mockFS.lastWritten);
        expect(mockFS.lastWritten.contains("Updated Device"));

        // Simulate new db and load from mock
        TestableMidiDevicesDB db2(mockFS);
        db2.loadJsonFile();

        DBG("it has2:" + mockFS.lastWritten);
        expect(db2.deviceExists("1234", "0001") == true);
        expect(db2.getDeviceName("1234", "0001") == "Updated Device");
        expect(db2.deviceExists("abcd", "0010") == true);
        expect(db2.getDeviceName("abcd", "0010") == "Another Device");
    }
};


static MidiDevicesDBUnitTest midiDevicesDBUnitTest;