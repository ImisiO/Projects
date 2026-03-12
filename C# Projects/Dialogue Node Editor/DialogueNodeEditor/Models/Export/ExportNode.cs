using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DialogueNodeEditor.Models.Export
{
    public class ExportNode
    {
        /// <summary>Unique identifier of the node</summary>
        public Guid Id { get; set; }

        /// <summary>Dialogue identifier label (e.g. "Greeting")</summary>
        public string DialogueId { get; set; } = string.Empty;

        /// <summary>Speaker / owner of this dialogue line</summary>
        public string Owner { get; set; } = string.Empty;

        /// <summary>Localised dialogue text keyed by translation key</summary>
        public Dictionary<string, string> DialogueTranslations { get; set; } = new();

        /// <summary>
        /// Nodes that follow directly from this one (OUT → IN connections).
        /// Empty for leaf nodes.
        /// </summary>
        public List<ExportNode> Children { get; set; } = new();
    }
}
