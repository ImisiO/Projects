using DialogueNodeEditor.ViewModels;

namespace DialogueNodeEditor.Commands.EditorCommands
{
    public sealed class AddTranslationEntryCommand : IEditorCommand
    {
        #region Init / Deinit

        /// <summary>
        /// Constructor for AddTranslationEntryCommand
        /// </summary>
        /// <param name="vm">Node that owns the translation list</param>
        /// <param name="key">Translation key to add/remove</param>
        public AddTranslationEntryCommand(DialogueNodeViewModel vm, string key)
        {
            _vm = vm;
            _key = key;
        }

        #endregion // Init / Deinit

        #region Member Variables

        /// <summary>Target node view model</summary>
        private readonly DialogueNodeViewModel _vm;

        /// <summary>Translation key to add/remove</summary>
        private readonly string _key;

        #endregion // Member Variables

        /// <summary>Adds the translation entry to the node</summary>
        public void Execute() => _vm.AddTranslation(_key);

        /// <summary>Removes the translation entry that was added</summary>
        public void Undo()
        {
            var entry = _vm.Translations.FirstOrDefault(t => string.Equals(t.Key, _key, StringComparison.OrdinalIgnoreCase));

            if (entry != null) 
            {
                _vm.RemoveTranslation(entry);
            }
        }
    }
}