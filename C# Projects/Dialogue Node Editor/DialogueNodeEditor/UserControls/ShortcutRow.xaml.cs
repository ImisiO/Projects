using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace DialogueNodeEditor.UserControls
{
    /// <summary>
    /// Interaction logic for ShortcutRow.xaml
    /// </summary>
    public partial class ShortcutRow : UserControl
    {
        #region Init / Deinit

        public ShortcutRow()
        {
            InitializeComponent();
        }

        #endregion // Init / Deinit

        #region Dependency Properties

        /// <summary>
        /// The key combination string, e.g. "Ctrl + Shift + S".
        /// Tokens are split on '+' and each rendered as an individual chip.
        /// </summary>
        public static readonly DependencyProperty KeysProperty =
            DependencyProperty.Register(
                nameof(Keys),
                typeof(string),
                typeof(ShortcutRow),
                new PropertyMetadata(string.Empty, OnKeysChanged));

        /// <summary>Key combination string (e.g. "Ctrl + S")</summary>
        public string Keys
        {
            get => (string)GetValue(KeysProperty);
            set => SetValue(KeysProperty, value);
        }

        /// <summary>Short description of what the shortcut does</summary>
        public static readonly DependencyProperty DescriptionProperty =
            DependencyProperty.Register(
                nameof(Description),
                typeof(string),
                typeof(ShortcutRow),
                new PropertyMetadata(string.Empty));

        /// <summary>Short description of what the shortcut does</summary>
        public string Description
        {
            get => (string)GetValue(DescriptionProperty);
            set => SetValue(DescriptionProperty, value);
        }

        /// <summary>
        /// Parsed list of display tokens (chips + separators) derived from <see cref="Keys"/>.
        /// Bound by the ItemsControl in XAML.
        /// </summary>
        public static readonly DependencyProperty KeyTokensProperty =
            DependencyProperty.Register(
                nameof(KeyTokens),
                typeof(List<KeyToken>),
                typeof(ShortcutRow),
                new PropertyMetadata(null));

        /// <summary>Parsed key tokens for the ItemsControl</summary>
        public List<KeyToken> KeyTokens
        {
            get => (List<KeyToken>)GetValue(KeyTokensProperty);
            private set => SetValue(KeyTokensProperty, value);
        }

        #endregion // Dependency Properties

        #region Helpers

        /// <summary>
        /// Re-parses <see cref="Keys"/> into <see cref="KeyTokens"/> whenever the
        /// raw string changes.
        /// </summary>
        private static void OnKeysChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is ShortcutRow row)
                row.KeyTokens = ParseKeys((string)e.NewValue);
        }

        /// <summary>
        /// Splits a key string like "Ctrl + Shift + S" into a list of
        /// <see cref="KeyToken"/> objects, interleaving separator tokens between chips.
        /// </summary>
        private static List<KeyToken> ParseKeys(string keys)
        {
            var tokens = new List<KeyToken>();
            if (string.IsNullOrWhiteSpace(keys)) return tokens;

            string[] parts = keys.Split('+');

            for (int i = 0; i < parts.Length; i++)
            {
                string label = parts[i].Trim();
                if (string.IsNullOrEmpty(label)) continue;

                // Add separator between chips (but not before the first one)
                if (tokens.Count > 0)
                    tokens.Add(new KeyToken("+", isSeparator: true));

                tokens.Add(new KeyToken(label, isSeparator: false));
            }

            return tokens;
        }

        #endregion // Helpers
    }

    /// <summary>
    /// A single display token in a key combination — either a key chip or a '+' separator.
    /// </summary>
    public class KeyToken
    {
        public KeyToken(string label, bool isSeparator)
        {
            Label = label;
            IsSeparator = isSeparator;
        }

        /// <summary>Text shown inside the token</summary>
        public string Label { get; }

        /// <summary>True for '+' separators, false for key chips</summary>
        public bool IsSeparator { get; }
    }
}

