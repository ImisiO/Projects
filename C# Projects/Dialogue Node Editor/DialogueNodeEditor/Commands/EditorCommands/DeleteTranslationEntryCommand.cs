using DialogueNodeEditor.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DialogueNodeEditor.Commands.EditorCommands
{
    internal class RemoveTranslationEntryCommand : IEditorCommand
    {
        #region Init / Deinit

        /// <summary>
        /// Constructor for RemoveTranslationEntryCommand
        /// </summary>
        /// <param name="vm">Node that owns the translation list</param>
        /// <param name="entry">The entry that is about to be removed/restored</param>
        public RemoveTranslationEntryCommand(DialogueNodeViewModel vm, DialogueTranslationEntryViewModel entry)
        {
            _vm = vm;
            _entry = entry;
            _index = vm.Translations.IndexOf(entry);
            _textSnapshot = entry.Text;
        }

        #endregion // Init / Deinit

        #region Member Variables

        /// <summary>Target node view model</summary>
        private readonly DialogueNodeViewModel _vm;

        /// <summary>The entry being removed/restored</summary>
        private readonly DialogueTranslationEntryViewModel _entry;

        /// <summary>Original position in the Translations list (used to restore on undo)</summary>
        private readonly int _index;

        /// <summary>Text content at the moment of removal (used to restore on undo)</summary>
        private readonly string _textSnapshot;

        #endregion // Member Variables

        /// <summary>Removes the translation entry from the node</summary>
        public void Execute() => _vm.RemoveTranslation(_entry);

        /// <summary>Re-inserts the translation entry at its original position</summary>
        public void Undo() => _vm.InsertTranslation(_index, _entry.Key, _textSnapshot);
    }
}
