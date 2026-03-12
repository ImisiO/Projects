using DialogueNodeEditor.ViewModels;

namespace DialogueNodeEditor.Commands.EditorCommands
{
    public sealed class EditTranslationTextCommand : IEditorCommand
    {
        #region Init / Deinit

        /// <summary>
        /// Constructor for EditTranslationTextCommand
        /// </summary>
        /// <param name="entry">The translation entry that was edited</param>
        /// <param name="before">Text value before the edit</param>
        /// <param name="after">Text value after the edit</param>
        public EditTranslationTextCommand(DialogueTranslationEntryViewModel entry, string before, string after)
        {
            _entry = entry;
            _before = before;
            _after = after;
        }

        #endregion // Init / Deinit

        #region Member Variables

        /// <summary>The translation entry that was edited</summary>
        private readonly DialogueTranslationEntryViewModel _entry;

        /// <summary>Text value before the edit</summary>
        private readonly string _before;

        /// <summary>Text value after the edit</summary>
        private readonly string _after;

        #endregion // Member Variables

        /// <summary>Applies the edited text</summary>
        public void Execute() => _entry.Text = _after;

        /// <summary>Reverts to the text before the edit</summary>
        public void Undo() => _entry.Text = _before;
    }
}