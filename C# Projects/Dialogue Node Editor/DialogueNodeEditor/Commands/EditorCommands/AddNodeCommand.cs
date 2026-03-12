using DialogueNodeEditor.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DialogueNodeEditor.Commands.EditorCommands
{
    internal class AddNodeCommand : IEditorCommand
    {
        #region Init / Deinit

        /// <summary>
        /// Constructor for AddNodeCommand 
        /// </summary>
        /// <param name="vm">Main window view model to update</param>
        /// <param name="conn">Node to add/delete</param>
        public AddNodeCommand(MainWindowViewModel vm, DialogueNodeViewModel conn)
        {
            _vm = vm;
            _node = conn;
        }

        #endregion // Init / Deinit

        #region // Member Variables

        /// <summary>Main window view model</summary>
        private readonly MainWindowViewModel _vm;

        /// <summary>Connection to delete/restore</summary>
        private readonly DialogueNodeViewModel _node;

        #endregion // Member Variables

        /// <summary>
        /// Adds dialogue node
        /// </summary>
        public void Execute()
        {
            _vm.Nodes.Add(_node);
        }

        /// <summary>
        /// Removes previously added node 
        /// </summary>
        public void Undo()
        {
            _vm.Nodes.Remove(_node);
        }
    }
}
