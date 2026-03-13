using DialogueNodeEditor.ViewModels;

namespace DialogueNodeEditor.Commands.EditorCommands
{
    public sealed class DeleteConnectionCommand : IEditorCommand
    {

        #region Init / Deinit

        /// <summary>
        /// Constructor for DeleteConnectionCommand 
        /// </summary>
        /// <param name="vm">Main window view model to update</param>
        /// <param name="conn">Connection to delete/restore</param>
        public DeleteConnectionCommand(MainWindowViewModel vm, DialogueConnectionViewModel conn)
        {
            _vm = vm;
            _conn = conn;
        }

        #endregion // Init / Deinit

        #region // Member Variables
        
        /// <summary>Main window view model</summary>
        private readonly MainWindowViewModel _vm;

        /// <summary>Connection to delete/restore</summary>
        private readonly DialogueConnectionViewModel _conn;

        #endregion // Member Variables

        /// <summary>
        /// Deletes node connection
        /// </summary>
        public void Execute()
        {
            _vm.Connections.Remove(_conn);
        }

        /// <summary>
        /// Restores deleted node connection
        /// </summary>
        public void Undo()
        {
            _vm.Connections.Add(_conn);
        }
    }
}