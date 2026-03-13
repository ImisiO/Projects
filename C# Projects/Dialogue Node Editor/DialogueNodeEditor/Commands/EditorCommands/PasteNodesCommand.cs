using DialogueNodeEditor.ViewModels;

namespace DialogueNodeEditor.Commands.EditorCommands
{
    public sealed class PasteNodesCommand : IEditorCommand
    {
        #region Init / Deinit

        /// <summary>
        /// Constructor for PasteNodesCommand
        /// </summary>
        /// <param name="vm">Main window view model to update</param>
        /// <param name="pastedNodes">The new node view models created by the paste</param>
        public PasteNodesCommand(MainWindowViewModel vm, IEnumerable<DialogueNodeViewModel> pastedNodes)
        {
            _vm = vm;
            _nodes = pastedNodes.ToList();
        }

        #endregion // Init / Deinit

        #region Member Variables

        /// <summary>Main window view model</summary>
        private readonly MainWindowViewModel _vm;

        /// <summary>Nodes added by this paste — restored on undo, re-added on redo</summary>
        private readonly List<DialogueNodeViewModel> _nodes;

        #endregion // Member Variables

        /// <summary>Adds all pasted nodes to the canvas and selects them.</summary>
        public void Execute()
        {
            _vm.ClearSelection();
            foreach (DialogueNodeViewModel node in _nodes)
            {
                _vm.Nodes.Add(node);
                node.IsSelected = true;
            }
        }

        /// <summary>Removes all pasted nodes</summary>
        public void Undo()
        {
            foreach (DialogueNodeViewModel node in _nodes) 
            {
                _vm.Nodes.Remove(node);
            }
        }
    }
}