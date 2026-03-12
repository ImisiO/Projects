using DialogueNodeEditor.Models;
using Microsoft.Win32;
using Newtonsoft.Json;
using System.IO;

namespace DialogueNodeEditor.Services
{
    public class DialogueGraphService
    {
        #region Constants

        /// <summary>File filter used by the open/save dialogs</summary>
        private const string DialogFilter = "Dialogue Graph (*.dlg)|*.dlg|JSON (*.json)|*.json|All files (*.*)|*.*";

        /// <summary>Default file extension</summary>
        private const string DefaultExt = ".dlg";

        #endregion // Constants

        /// <summary>
        /// Opens a Save dialog and serializes the passed graph to the chosen path.
        /// </summary>
        /// <param name="graph">Graph to save</param>
        /// <param name="currentPath">Pre-fills the dialog with the previously used path (pass null to start fresh).</param>
        /// <returns>
        /// The path the file was written to, or null if the user cancelled.
        /// </returns>
        public static string? SaveAs(DialogueGraph graph, string? currentPath = null)
        {
            SaveFileDialog dlg = new()
            {
                Title = "Save Dialogue Graph",
                Filter = DialogFilter,
                DefaultExt = DefaultExt,
                FileName = currentPath != null ? Path.GetFileName(currentPath) : "graph",
                InitialDirectory = currentPath != null ? Path.GetDirectoryName(currentPath) : Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments),
            };

            if (dlg.ShowDialog() != true)
            {
                return null;
            }

            WriteToFile(graph, dlg.FileName);
            return dlg.FileName;
        }

        /// <summary>
        /// Serializes the passed graph directly to the passed path
        /// without opening a dialog (used for Ctrl+S when a path is already known).
        /// </summary>
        public static void Save(DialogueGraph graph, string path) => WriteToFile(graph, path);

        /// <summary>
        /// Opens an Open dialog and deserializes a <see cref="DialogueGraph"/> from the chosen file.
        /// </summary>
        /// <returns>
        /// A tuple of (graph, path) on success, or (null, null) if the user cancelled or the file could not be read.
        /// </returns>
        public static (DialogueGraph? Graph, string? Path) Open()
        {
            OpenFileDialog dlg = new()
            {
                Title = "Open Dialogue Graph",
                Filter = DialogFilter,
                DefaultExt = DefaultExt,
                InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments),
            };

            if (dlg.ShowDialog() != true) return (null, null);

            DialogueGraph? graph = ReadFromFile(dlg.FileName);
            return (graph, graph != null ? dlg.FileName : null);
        }

        /// <summary>Serializes the passed graph into to the passed file path/>.</summary>
        private static void WriteToFile(DialogueGraph graph, string path)
        {
            string jsonNodeGraph = JsonConvert.SerializeObject(graph, Formatting.Indented);
            File.WriteAllText(path, jsonNodeGraph);
        }

        /// <summary>
        /// Deserializes a <see cref="DialogueGraph"/> from the passed path/>.
        /// Returns <c>null</c> and shows a message box if the file is invalid.
        /// </summary>
        private static DialogueGraph? ReadFromFile(string path)
        {
            try
            {
                string jsonNodeGraph = File.ReadAllText(path);
                return JsonConvert.DeserializeObject<DialogueGraph>(jsonNodeGraph);
            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show(
                    $"Failed to load graph:\n{ex.Message}",
                    "Load Error",
                    System.Windows.MessageBoxButton.OK,
                    System.Windows.MessageBoxImage.Error);
                return null;
            }
        }
    }
}
