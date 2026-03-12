using System.Configuration;
using System.Data;
using System.Windows;

namespace DialogueNodeEditor
{
    public partial class App : Application
    {
        /// <summary>
        /// Call back for when the application starts
        /// </summary>
        /// <param name="e">Startup event arguments</param>
        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);

            Common.InitLogger();

            if (!Common.LoadConfig())
            {
                Common.Logger!.Warning("[App] - Failed to load config, starting with defaults");
            }
        }
    }
}
