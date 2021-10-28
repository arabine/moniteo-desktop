#include "Settings.h"
#include "JsonReader.h"
#include "JsonWriter.h"
#include "Util.h"

void Settings::ReadSettings(IProcessEngine &engine)
{
    std::string defaultWorkspace = Util::HomePath() + Util::DIR_SEPARATOR + ".manolab";
    std::string workspace = defaultWorkspace;
    JsonReader confFile;
    JsonValue json;

    if (confFile.ParseFile(json, "manolab.json"))
    {
        workspace = json.FindValue("workspace").GetString();

        JsonArray plugins = json.FindValue("plugins").GetArray();

        for (const auto & p : plugins)
        {
            mPlugins.push_back(p.GetString());
        }
        engine.SetPlugins(mPlugins);

        mAutoLoadScriptName = json.FindValue("autoload").GetString();
        mAutoRun = json.FindValue("autorun").GetBool();
    }

    if (!Util::FolderExists(workspace))
    {
        workspace = defaultWorkspace;
        Util::Mkdir(workspace);
    }

//    std::cout << "Current workspace is: " << workspace << std::endl;

    engine.SetWorkspace(workspace);

    // Auto load specified script
    if (engine.ScriptExists(mAutoLoadScriptName))
    {
        engine.LoadScript(mAutoLoadScriptName);
        if (mAutoRun)
        {
            engine.Start();
        }
    }

    WriteSettings(engine);
}

void Settings::WriteSettings(const IProcessEngine &engine)
{
    JsonObject json;
    json.AddValue("workspace", engine.GetWorkspace());
    json.AddValue("autoload", mAutoLoadScriptName);
    json.AddValue("autorun", mAutoRun);

    JsonArray plugins;

    for (const auto &p : mPlugins)
    {
        plugins.AddValue(p);
    }

    json.AddValue("plugins", plugins);

    JsonWriter::SaveToFile(json, "manolab.json");
}

