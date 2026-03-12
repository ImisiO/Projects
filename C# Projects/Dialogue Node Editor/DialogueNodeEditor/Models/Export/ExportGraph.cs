using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DialogueNodeEditor.Models.Export
{
    public class ExportGraph
    {
        /// <summary>
        /// Conversation entry-point nodes, each with their full subtree nested inside.
        /// </summary>
        public List<ExportNode> Roots { get; set; } = new();

        /// <summary>
        /// Nodes that have neither incoming nor outgoing connections.
        /// Exported flat rather than as a tree since they have no relationships.
        /// </summary>
        public List<ExportNode> Orphans { get; set; } = new();
    }
}
