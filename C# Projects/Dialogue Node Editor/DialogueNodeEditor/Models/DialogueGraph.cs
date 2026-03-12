using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DialogueNodeEditor.Models
{
    public class DialogueGraph
    {
        /// <summary>All dialogue nodes in the graph</summary>
        public List<DialogueNode> Nodes { get; set; } = new();

        /// <summary>All connections between nodes</summary>
        public List<DialogueConnection> Connections { get; set; } = new();
    }
}
