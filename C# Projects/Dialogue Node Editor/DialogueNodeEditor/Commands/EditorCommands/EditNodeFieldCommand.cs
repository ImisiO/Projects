using DialogueNodeEditor.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace DialogueNodeEditor.Commands.EditorCommands
{
    public sealed class EditNodeFieldCommand : IEditorCommand
    {
        #region Init / Deinit

        /// <summary>
        /// Constructor for EditNodeFieldCommand
        /// </summary>
        /// <param name="node">Dialogue node to edit</param>
        /// <param name="field">Field to edit ("DialogueId" | "Owner" | "Dialogue")</param>
        /// <param name="before">The field's value before editing</param>
        /// <param name="after">The field's value after editing</param>
        public EditNodeFieldCommand(DialogueNodeViewModel node, string field, string before, string after)
        {
            _node = node;
            _field = field;
            _before = before;
            _after = after;
        }

        #endregion // Init / Deinit

        #region Member Variables

        /// <summary>Dialogue node to edit</summary>
        private readonly DialogueNodeViewModel _node;

        /// <summary>Field to edit ("DialogueId" | "Owner" | "Dialogue")</summary>
        private readonly string _field;

        /// <summary>The field's value before editing</summary>
        private readonly string _before;

        /// <summary>The field's value after editing</summary>
        private readonly string _after;

        #endregion // Member Variables

        /// <summary>
        /// Applies changes to the node's field
        /// </summary>
        public void Execute() => Apply(_after);

        /// <summary>
        /// Reverts changes to the node's field
        /// </summary>
        public void Undo() => Apply(_before);

        /// <summary>
        /// Applies value to the node's field
        /// </summary>
        /// <param name="value">Value to apply to field</param>
        private void Apply(string value)
        {
            switch (_field)
            {
                case nameof(DialogueNodeViewModel.DialogueId): 
                    _node.DialogueId = value;
                    break;

                case nameof(DialogueNodeViewModel.Owner): 
                    _node.Owner = value; 
                    break;

                case nameof(DialogueNodeViewModel.Notes): 
                    _node.Notes = value; 
                    break;
            }
        }
    }
}
