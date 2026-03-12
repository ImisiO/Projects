using DialogueNodeEditor.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DialogueNodeEditor.Commands.EditorCommands
{
    public sealed class DeleteNodesCommand : IEditorCommand
    {
        #region Init / Deinit
        
        /// <summary>
        /// Constructor for DeleteNodesCommand
        /// </summary>
        /// <param name="vm">Main window view model to update</param>
        /// <param name="nodesToDelete">Nodes to delete/restore</param>
        public DeleteNodesCommand(MainWindowViewModel vm, IEnumerable<DialogueNodeViewModel> nodesToDelete)
        {
            _vm = vm;

            _nodes = nodesToDelete.ToList();

            HashSet<Guid> ids = _nodes.Select(n => n.Id).ToHashSet();
            _connections = vm.Connections
                .Where(c => ids.Contains(c.FromId) || ids.Contains(c.ToId))
                .ToList();
        }

        #endregion // Init / Deinit

        #region Member Variables

        /// <summary>Main window view model</summary>
        private readonly MainWindowViewModel _vm;

        /// <summary>Nodes to restore/remove</summary>
        private readonly List<DialogueNodeViewModel> _nodes;

        /// <summary>Connections to restore/delete</summary>
        private readonly List<DialogueConnectionViewModel> _connections;

        #endregion // Member Variables

        /// <summary>
        /// Deletes nodes and their associated connections
        /// </summary>
        public void Execute()
        {
            foreach (DialogueConnectionViewModel conn in _connections) 
            {
                _vm.Connections.Remove(conn);
            }

            foreach (DialogueNodeViewModel node in _nodes) 
            {
                _vm.Nodes.Remove(node);
            }
        }

        /// <summary>
        /// Restores deleted nodes and their associated connections
        /// </summary>
        public void Undo()
        {
            foreach (DialogueNodeViewModel node in _nodes) 
            {
                _vm.Nodes.Add(node);
            }

            foreach (DialogueConnectionViewModel conn in _connections) 
            {
                _vm.Connections.Add(conn);
            }

            _vm.RefreshAllConnections();
        }
    }
}
