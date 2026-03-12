using DialogueNodeEditor.Models;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DialogueNodeEditor.ViewModels
{
    public class DialogueNodeViewModel : ViewModelBase
    {
        #region Init/Deinit

        /// <summary>
        /// Constructor for DialogueNodeViewModel object 
        /// </summary>
        /// <param name="model">Dialogue node</param>
        public DialogueNodeViewModel(DialogueNode model)
        {
            Model = model;
            _dialogueId = model.DialogueId;
            _owner = model.Owner;
            _x = model.X;
            _y = model.Y;

            // Seed translations from model — or initialise with the first config key.
            var configKeys = Common.Config?.TranslationKeys ?? new List<string>();

            if (model.DialogueTranslations.Count == 0 && configKeys.Count > 0) 
            {
                model.DialogueTranslations[configKeys[0]] = "";
            }

            foreach (var kvp in model.DialogueTranslations) 
            {
                AttachEntry(new DialogueTranslationEntryViewModel(kvp.Key, kvp.Value));
            }

            UpdateFirstFlags();
        }

        #endregion

        #region Member Variables

        /// <summary>Dialogue node model</summary>
        public DialogueNode Model { get; }
        
        /// <summary>Id of the dialogue node view model</summary>
        public Guid Id => Model.Id;

        /// <summary>[STORE] Dialogue node</summary>
        private string _dialogueId = "";
        /// <summary>Dialogue node</summary>
        public string DialogueId
        {
            get => _dialogueId;
            set
            {
                if (SetField(ref _dialogueId, value)) 
                {
                    Model.DialogueId = value; 
                }
            }
        }

        /// <summary>[STORE] Dialogue owner</summary>
        private string _owner = "";
        /// <summary>Dialogue owner</summary>
        public string Owner
        {
            get => _owner;
            set
            {
                if (SetField(ref _owner, value))
                { 
                    Model.Owner = value;
                }
            }
        }

        /// <summary>[STORE] Optional notes attached to the node</summary>
        private string _notes = "";
        /// <summary>Optional notes attached to the node</summary>
        public string Notes
        {
            get => _notes;
            set 
            { 
                if (SetField(ref _notes, value))
                {
                    Model.Notes = value;
                    OnPropertyChanged(nameof(HasNotes));
                }
            }
        }

        /// <summary>Whether or not the node has notes</summary>
        public bool HasNotes => !string.IsNullOrWhiteSpace(_notes);

        /// <summary>Per-locale dialogue entries</summary>
        public ObservableCollection<DialogueTranslationEntryViewModel> Translations { get; } = new ObservableCollection<DialogueTranslationEntryViewModel>();

        /// <summary>
        /// Translation keys from the global config that are not yet used on this node.
        /// </summary>
        public IEnumerable<string> AvailableTranslationKeys
        {
            get
            {
                var allKeys = Common.Config?.TranslationKeys ?? new List<string>();
                var usedKeys = Translations.Select(t => t.Key)
                                           .ToHashSet(StringComparer.OrdinalIgnoreCase);
                return allKeys.Where(k => !usedKeys.Contains(k));
            }
        }

        /// <summary>Whether or not more translations can been added</summary>
        public bool CanAddTranslation => AvailableTranslationKeys.Any();

        /// <summary>[STORE] Node x position</summary>
        private double _x;
        /// <summary>Node x position</summary>
        public double X
        {
            get => _x;
            set
            {
                if (SetField(ref _x, value)) 
                {
                    Model.X = value;
                }
            }
        }

        /// <summary>[STORE] Node y position</summary>
        private double _y;
        /// <summary>Node y position</summary>
        public double Y
        {
            get => _y;
            set
            {
                if (SetField(ref _y, value)) 
                {
                    Model.Y = value;
                }
            }
        }

        /// <summary>[STORE] Whether or not the node is selected</summary>
        private bool _isSelected;
        /// <summary>Whether or not the node is selected</summary>
        public bool IsSelected
        {
            get => _isSelected;
            set => SetField(ref _isSelected, value);
        }

        #endregion // Member Variables

        #region Translation Management

        /// <summary>
        /// Appends a new translation entry for the passed key and syncs the model.
        /// </summary>
        /// <param name="key">New translation key to add</param>
        public void AddTranslation(string key)
        {
            if (!AvailableTranslationKeys.Contains(key, StringComparer.OrdinalIgnoreCase)) 
            {
                return;
            }

            DialogueTranslationEntryViewModel entry = new DialogueTranslationEntryViewModel(key, "");
            AttachEntry(entry);
            Model.DialogueTranslations[key] = "";
            UpdateFirstFlags();
            NotifyTranslationCapacityChanged();
        }

        /// <summary>
        /// Inserts a translation entry at a specific index (used by undo of remove).
        /// </summary>
        /// <param name="index">Index to insert it at</param>
        /// <param name="key">Translation key</param>
        /// <param name="text">Translation text</param>
        public void InsertTranslation(int index, string key, string text)
        {
            index = Math.Clamp(index, 0, Translations.Count);
            var entry = new DialogueTranslationEntryViewModel(key, text);
            AttachEntry(entry, index);
            Model.DialogueTranslations[key] = text;
            UpdateFirstFlags();
            NotifyTranslationCapacityChanged();
        }

        /// <summary>
        /// Removes a translation entry (the first entry is protected — call is ignored).
        /// </summary>
        /// <param name="entry">Translation entry to remove</param>
        public void RemoveTranslation(DialogueTranslationEntryViewModel entry)
        {
            if (entry.IsFirst || !Translations.Contains(entry)) 
            {
                return;
            }

            entry.PropertyChanged -= OnEntryTextChanged;
            Translations.Remove(entry);
            Model.DialogueTranslations.Remove(entry.Key);
            UpdateFirstFlags();
            NotifyTranslationCapacityChanged();
        }

        /// <summary>
        /// Subscribes to text-change events and appends (or inserts at index) the entry.
        /// </summary>
        private void AttachEntry(DialogueTranslationEntryViewModel entry, int? index = null)
        {
            entry.PropertyChanged += OnEntryTextChanged;
            if (index.HasValue)
            {
                Translations.Insert(index.Value, entry);
            }
            else
            {
                Translations.Add(entry);
            }
        }

        /// <summary>
        /// Keeps the model dictionary in sync when an entry's Text changes.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnEntryTextChanged(object? sender, PropertyChangedEventArgs e)
        {
            if (sender is DialogueTranslationEntryViewModel entry && e.PropertyName == nameof(DialogueTranslationEntryViewModel.Text))
            {
                Model.DialogueTranslations[entry.Key] = entry.Text;
            }
        }

        /// <summary>Marks the first entry as non-removable; all others as removable.</summary>
        private void UpdateFirstFlags()
        {
            for (int i = 0; i < Translations.Count; i++) 
            {
                Translations[i].IsFirst = (i == 0);
            }
        }

        /// <summary>Fires change notifications for the capacity-related computed properties.</summary>
        private void NotifyTranslationCapacityChanged()
        {
            OnPropertyChanged(nameof(CanAddTranslation));
            OnPropertyChanged(nameof(AvailableTranslationKeys));
        }

        #endregion // Translation Management
    }
}
