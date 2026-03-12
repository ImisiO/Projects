using DialogueNodeEditor.ViewModels;
using System.Drawing;

namespace DialogueNodeEditor.Commands.EditorCommands
{
    public sealed class AddConnectionCommand : IEditorCommand
    {
        #region Init / Deinit

        /// <summary>
        /// Constructor for AddConnectionCommand 
        /// </summary>
        /// <param name="vm">Main window view model to update</param>
        /// <param name="conn">Connection to add/delete</param>
        public AddConnectionCommand(MainWindowViewModel vm, DialogueConnectionViewModel conn)
        {
            _vm = vm;
            _conn = conn;
        }

        #endregion // Init / Deinit

        #region // Member Variables

        /// <summary>Main window view model</summary>
        private readonly MainWindowViewModel _vm;

        /// <summary>Connection to add/delete</summary>
        private readonly DialogueConnectionViewModel _conn;

        #endregion // Member Variables

        /// <summary>
        /// Adds node connection
        /// </summary>
        public void Execute()
        {
            _vm.Connections.Add(_conn);
            _vm.RefreshAllConnections();    
        }

        /// <summary>
        /// Deletes previously added node connection
        /// </summary>
        public void Undo()
        {
            _vm.Connections.Remove(_conn);
        }
    }
}
