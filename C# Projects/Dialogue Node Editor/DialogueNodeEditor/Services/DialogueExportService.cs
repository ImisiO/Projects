using DialogueNodeEditor.Models;
using DialogueNodeEditor.Models.Export;
using Microsoft.Win32;
using Newtonsoft.Json;
using System.IO;

namespace DialogueNodeEditor.Services
{
    public static class DialogueExportService
    {
        #region Constants

        private const string DialogFilter = "JSON export (*.json)|*.json|All files (*.*)|*.*";
        private const string DefaultExt = ".json";

        #endregion

        /// <summary>
        /// Opens a Save dialog, builds the export tree from the passed graph, and writes the result to the chosen file.
        /// </summary>
        /// <returns>The path written to, or <c>null</c> if the user cancelled.</returns>
        public static string? Export(DialogueGraph graph, string? suggestedName = null)
        {
            SaveFileDialog dlg = new()
            {
                Title = "Export Dialogue Graph",
                Filter = DialogFilter,
                DefaultExt = DefaultExt,
                FileName = suggestedName != null
                    ? Path.GetFileNameWithoutExtension(suggestedName) + "_export"
                    : "dialogue_export",
                InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments),
            };

            if (dlg.ShowDialog() != true) return null;

            ExportGraph export = BuildExportTree(graph);

            string json = JsonConvert.SerializeObject(export, Formatting.Indented);
            File.WriteAllText(dlg.FileName, json);

            return dlg.FileName;
        }

        #region Tree Builder

        /// <summary>
        /// Converts a flat <see cref="DialogueGraph"/> into a tree-shaped <see cref="ExportedGraph"/>.
        /// </summary>
        /// <param name="graph">Dialogue graph to convert</param>
        /// <returns>Export graph</returns>
        private static ExportGraph BuildExportTree(DialogueGraph graph)
        {
            Dictionary<Guid, DialogueNode> nodeById = graph.Nodes.ToDictionary(n => n.Id);

            // outgoing: FromId → [ToId, ToId, …]
            Dictionary<Guid, List<Guid>> outgoing = graph.Nodes .ToDictionary(n => n.Id, _ => new List<Guid>());

            HashSet<Guid> hasIncoming = new();

            foreach (DialogueConnection conn in graph.Connections)
            {
                if (outgoing.ContainsKey(conn.FromId)) 
                {
                    outgoing[conn.FromId].Add(conn.ToId);
                }

                hasIncoming.Add(conn.ToId);
            }

            bool IsOrphan(DialogueNode n) => !hasIncoming.Contains(n.Id) && outgoing[n.Id].Count == 0;

            bool IsRoot(DialogueNode n) => !hasIncoming.Contains(n.Id) && outgoing[n.Id].Count > 0;

            // Walk the tree
            HashSet<Guid> visited = new();
            ExportGraph export = new();

            foreach (DialogueNode root in graph.Nodes.Where(IsRoot)) 
            {
                export.Roots.Add(BuildNode(root, outgoing, nodeById, visited));
            }

            // Orphans 
            foreach (DialogueNode node in graph.Nodes.Where(IsOrphan)) 
            {
                export.Orphans.Add(ToExportedNode(node));
            }

            return export;
        }

        /// <summary>
        /// Recursively builds an <see cref="ExportNode"/> from the passed dialogue node
        /// and all of its descendants
        /// </summary>
        /// <param name="node">Dialogue node to build children for</param>
        /// <param name="outgoing">Adjacency map of FromId → list of ToIds, built from the flat connection list.</param>
        /// <param name="nodeById">All dialogue nodes indexed by their ID</param>
        /// <param name="visited">Set of node IDs already expanded in this branch</param>
        /// <returns>Export node</returns>
        private static ExportNode BuildNode(DialogueNode node, Dictionary<Guid, List<Guid>> outgoing, Dictionary<Guid, DialogueNode> nodeById, HashSet<Guid> visited)
        {
            ExportNode exported = ToExportedNode(node);

            if (!visited.Add(node.Id))
            {
                exported.DialogueId = $"[cycle → {node.DialogueId}]";
                return exported;
            }

            foreach (Guid childId in outgoing[node.Id])
            {
                if (!nodeById.TryGetValue(childId, out DialogueNode? child)) 
                {
                    continue;
                }

                exported.Children.Add(BuildNode(child, outgoing, nodeById, visited));
            }

            return exported;
        }

        /// <summary>
        /// Converts a raw <see cref="DialogueNode"/> to an <see cref="ExportNode"/> without populating children.
        /// </summary>
        /// <param name="node">Node to convert</param>
        /// <returns>Export node without its children</returns>
        private static ExportNode ToExportedNode(DialogueNode node) => new()
        {
            Id = node.Id,
            DialogueId = node.DialogueId,
            Owner = node.Owner,
            DialogueTranslations = new Dictionary<string, string>(node.DialogueTranslations),
        };

        #endregion // Tree Builder
    }
}
