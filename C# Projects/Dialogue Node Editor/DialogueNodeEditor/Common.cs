using DialogueNodeEditor.Config;
using DialogueNodeEditor.Utilities;
using Newtonsoft.Json;
using System.IO;

namespace DialogueNodeEditor
{
    public static class Common
    {
        #region Member Variables

        /// <summary>Editor config</summary>
        public static EditorConfig? Config { get; private set;}
        
        /// <summary>Logger</summary>
        public static Logger? Logger { get; private set; }

        #endregion // Member Variables

        #region Constants

        /// <summary>Config path</summary>
        const string ConfigPath = "Config\\EditorConfig.json";

        /// <summary>Log file path</summary>
        const string LogPath = "Log\\{logFileName}.txt";

        #endregion // Constants

        /// <summary>
        /// Initiates logger
        /// </summary>
        /// <returns>Whether or not the logger was successfully initiated</returns>
        public static void InitLogger() 
        {
            string timestamp = DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss-fff");
            string logFileName = $"[{timestamp}] [DialogueNodeEditor]";
            string LogFilePath = LogPath.Replace("{logFileName}", logFileName);

            Logger = new Logger("DialogueNodeEditor", LogFilePath);
            Logger.Info("[Common] Successfully created logger");
        }

        /// <summary>
        /// Loads editor config json file
        /// </summary>
        public static bool LoadConfig() 
        {
            if (Path.Exists(ConfigPath)) 
            {
                string jsonEditorConfig = File.ReadAllText(ConfigPath);
                Config = JsonConvert.DeserializeObject<EditorConfig>(jsonEditorConfig);

                return Config != null;
            }
            else
            {
                Config = new EditorConfig();
                return false;
            }
        }

        /// <summary>
        /// Saves the config
        /// </summary>
        /// <returns>Whether or not the config was successfully saved</returns>
        public static void SaveConfig()
        {
            if (Config != null)
            {
                string jsonEditorConfig = JsonConvert.SerializeObject(Config, Formatting.Indented);

                try 
                {
                    File.WriteAllText(ConfigPath, jsonEditorConfig);
                    Logger!.Info("[Common] - Successfully saved editor config file");
                }
                catch 
                {
                }
            }
            else 
            {
                Logger!.Warning("[Common] - Failed to save config file as config was null");
            }
        }

        /// <summary>
        /// Adds translation key to editor config
        /// </summary>
        /// <param name="newTranslationKey">Translation key to add</param>
        public static void AddTranslationKey(string newTranslationKey) 
        {
            if (Config == null) 
            {
                Config = new EditorConfig();
            }

            if (!string.IsNullOrEmpty(newTranslationKey) && !Config.TranslationKeys.Contains(newTranslationKey, StringComparer.OrdinalIgnoreCase))
            {
                Config.TranslationKeys.Add(newTranslationKey);
            }
        }

        /// <summary>
        /// Removes translation key to editor config
        /// </summary>
        /// <param name="newTranslationKey">Translation key to remove</param>
        public static void RemoveTranslationKey(string newTranslationKey)
        {
            if (Config == null)
            {
                Config = new EditorConfig();
            }

            if (string.IsNullOrEmpty(newTranslationKey) && Config.TranslationKeys.Contains(newTranslationKey))
            {
                Config.TranslationKeys.Remove(newTranslationKey);
            }
        }
    }
}
