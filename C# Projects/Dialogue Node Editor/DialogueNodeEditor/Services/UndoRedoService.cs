using DialogueNodeEditor.Commands;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace DialogueNodeEditor.Services
{
    public class UndoRedoService
    {

        #region Init / Deinit

        /// <summary>
        /// Default constructor for UndoRedoService object
        /// </summary>
        public UndoRedoService()
        {
            UndoCommand = new RelayCommand(_ => Undo(), _ => CanUndo);
            RedoCommand = new RelayCommand(_ => Redo(), _ => CanRedo);
        }

        #endregion // Init / Deinit

        #region Member Variables

        /// <summary>Undo stack</summary>
        private readonly Stack<IEditorCommand> _undoStack = new();
        
        /// <summary>Redo stack</summary>
        private readonly Stack<IEditorCommand> _redoStack = new();

        /// <summary>Undo command</summary>
        public RelayCommand UndoCommand { get; }
        
        /// <summary>Redo command</summary>
        public RelayCommand RedoCommand { get; }

        /// <summary>Action to do after undoing or redoing</summary>
        public Action? AfterUndoRedo { get; set; }

        /// <summary>Whether or not an undo action can ben done</summary>
        public bool CanUndo => _undoStack.Count > 0;
        
        /// <summary>Whether or not a redo action can be done</summary>
        public bool CanRedo => _redoStack.Count > 0;

        #endregion // Member Variables

        #region Utility Functions

        /// <summary>
        /// Executes passed command and records it for undo.
        /// </summary>
        public void Record(IEditorCommand command)
        {
            command.Execute();

            _undoStack.Push(command);
            _redoStack.Clear();

            Refresh();
        }

        /// <summary>
        /// Undoes command on the top of the undo stack
        /// </summary>
        private void Undo()
        {
            if (!CanUndo)
            { 
                return;
            }

            IEditorCommand? cmd = _undoStack.Pop();
            cmd.Undo();
            _redoStack.Push(cmd);

            Refresh();
            AfterUndoRedo?.Invoke();
        }

        /// <summary>
        /// Redoes (executes) command on top of the redo stack
        /// </summary>
        private void Redo()
        {
            if (!CanRedo)
            { 
                return;
            }

            IEditorCommand cmd = _redoStack.Pop();
            cmd.Execute();
            _undoStack.Push(cmd);

            Refresh();
            AfterUndoRedo?.Invoke();
        }

        /// <summary>Raises CanExecuteChanged on both commands after any stack change.</summary>
        private void Refresh()
        {
            UndoCommand.RaiseCanExecuteChanged();
            RedoCommand.RaiseCanExecuteChanged();
        }

        /// <summary>
        /// Clears both stacks
        /// </summary>
        public void Clear()
        {
            _undoStack.Clear();
            _redoStack.Clear();
            Refresh();
        }

        #endregion // Utility Functions
    }
}
