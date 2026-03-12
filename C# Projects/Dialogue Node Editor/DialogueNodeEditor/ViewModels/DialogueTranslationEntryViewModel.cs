using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace DialogueNodeEditor.ViewModels
{
    public class DialogueTranslationEntryViewModel : ViewModelBase
    {
        #region Init / Deinit

        /// <summary>
        /// Constructor for DialogueTranslationEntryViewModel
        /// </summary>
        /// <param name="key">Translation key</param>
        /// <param name="text">Dialogue text for this translation</param>
        public DialogueTranslationEntryViewModel(string key, string text = "")
        {
            _key = key;
            _text = text;
        }

        #endregion // Init / Deinit

        #region Member Variables

        /// <summary>[STORE] Translation key (e.g. "en_us")</summary>
        private string _key;
        /// <summary>Translation key</summary>
        public string Key
        {
            get => _key;
            private set => SetField(ref _key, value);
        }

        /// <summary>[STORE] Dialogue text for this translation</summary>
        private string _text = "";
        /// <summary>Dialogue text for this translation</summary>
        public string Text
        {
            get => _text;
            set => SetField(ref _text, value);
        }

        /// <summary>[STORE] Whether this is the first (non-removable) entry</summary>
        private bool _isFirst;
        /// <summary>Whether this is the first (non-removable) entry</summary>
        public bool IsFirst
        {
            get => _isFirst;
            set
            {
                if (SetField(ref _isFirst, value))
                    OnPropertyChanged(nameof(RemoveButtonVisibility));
            }
        }

        /// <summary>Visibility of the remove button — collapsed for the first entry</summary>
        public Visibility RemoveButtonVisibility => IsFirst ? Visibility.Collapsed : Visibility.Visible;

        #endregion // Member Variables
    }
}
