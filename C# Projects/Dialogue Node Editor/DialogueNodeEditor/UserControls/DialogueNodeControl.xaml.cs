using DialogueNodeEditor.Commands.EditorCommands;
using DialogueNodeEditor.Services;
using DialogueNodeEditor.ViewModels;
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
    public partial class DialogueNodeControl : UserControl
    {
        #region Init / Deinit

        /// <summary>
        /// Default constructor for DialogueNodeControl object
        /// </summary>
        public DialogueNodeControl()
        {
            InitializeComponent();

            OutConnector.MouseLeftButtonDown += (s, e) =>
            {
                e.Handled = true;
                OutConnectorClicked?.Invoke(this, EventArgs.Empty);
            };

            InConnector.MouseLeftButtonDown += (s, e) =>
            {
                e.Handled = true;
                InConnectorClicked?.Invoke(this, EventArgs.Empty);
            };

            DragHandle.MouseLeftButtonDown += (s, e) =>
            {
                e.Handled = true;
                DragHandleMouseDown?.Invoke(this, e);
            };

            HoverEffect(OutConnector, Color.FromRgb(129, 199, 132), Color.FromRgb(160, 230, 160));
            HoverEffect(InConnector, Color.FromRgb(79, 195, 247), Color.FromRgb(150, 210, 255));

            TrackField(DialogueIdTextBox, nameof(DialogueNodeViewModel.DialogueId));
            TrackField(DialogueOwnerIdTextBox, nameof(DialogueNodeViewModel.Owner));
            TrackField(NotesTextBox, nameof(DialogueNodeViewModel.Notes));
        }

        #endregion // Init / Deinit

        #region Member Variables

        /// <summary>Out connector ellipse</summary>
        public Ellipse PublicOutConnector => OutConnector;

        /// <summary>In connector ellipse</summary>
        public Ellipse PublicInConnector => InConnector;

        /// <summary>Dialogue node view model</summary>
        public DialogueNodeViewModel? VM => DataContext as DialogueNodeViewModel;

        /// <summary>
        /// Stores the text value of each translation entry at the moment its TextBox receives focus,
        /// so that we can detect a change and record an undo command when focus is lost.
        /// </summary>
        private readonly Dictionary<DialogueTranslationEntryViewModel, string> _translationTextBeforeEdit = new();

        #endregion // Member Variables

        #region Undo / Redo

        /// <summary>
        /// [DependencyProperty] The undo/redo service
        /// </summary>
        public static readonly DependencyProperty UndoRedoServiceProperty =
            DependencyProperty.Register(
                nameof(UndoRedoService),
                typeof(UndoRedoService),
                typeof(DialogueNodeControl),
                new PropertyMetadata(null));

        /// <summary>Undo redo service</summary>
        public UndoRedoService? UndoRedoService
        {
            get => (UndoRedoService?)GetValue(UndoRedoServiceProperty);
            set => SetValue(UndoRedoServiceProperty, value);
        }

        /// <summary>
        /// Wires GotFocus / LostFocus on a TextBox to snapshot and record an EditNodeFieldCommand.
        /// </summary>
        private void TrackField(TextBox box, string fieldName)
        {
            string before = string.Empty;

            box.GotFocus += (_, _) =>
            {
                before = GetField(fieldName) ?? string.Empty;
            };

            box.LostFocus += (_, _) =>
            {
                if (VM == null || UndoRedoService == null) 
                {
                    return;
                }

                string after = GetField(fieldName) ?? string.Empty;

                // Only record if something actually changed.
                if (after == before) 
                {
                    return;
                }

                UndoRedoService.Record(new EditNodeFieldCommand(VM, fieldName, before, after));

                before = after;
            };
        }

        /// <summary>Reads the current value of a named field from the VM.</summary>
        private string? GetField(string fieldName) => fieldName switch
        {
            nameof(DialogueNodeViewModel.DialogueId) => VM?.DialogueId,
            nameof(DialogueNodeViewModel.Owner) => VM?.Owner,
            nameof(DialogueNodeViewModel.Notes) => VM?.Notes,
            _ => null
        };

        /// <summary>
        /// Captures text before-value when a translation TextBox gains focus.
        /// The event bubbles up from the TextBoxes inside the ItemsControl.
        /// </summary>
        private void TranslationsItemsControl_GotFocus(object sender, RoutedEventArgs e)
        {
            if (e.OriginalSource is TextBox tb
                && tb.DataContext is DialogueTranslationEntryViewModel entry)
            {
                _translationTextBeforeEdit[entry] = entry.Text;
            }
        }

        /// <summary>
        /// Records an EditTranslationTextCommand when a translation TextBox
        /// loses focus and its content has changed.
        /// </summary>
        private void TranslationsItemsControl_LostFocus(object sender, RoutedEventArgs e)
        {
            if (e.OriginalSource is TextBox tb
                && tb.DataContext is DialogueTranslationEntryViewModel entry
                && UndoRedoService != null
                && _translationTextBeforeEdit.TryGetValue(entry, out string? before))
            {
                _translationTextBeforeEdit.Remove(entry);

                if (entry.Text != before) 
                {
                    UndoRedoService.Record(new EditTranslationTextCommand(entry, before, entry.Text));
                }
            }
        }

        #endregion // Undo / Redo

        #region Event Handler

        /// <summary>Event handler for when the out connecter is clicked</summary>
        public event EventHandler? OutConnectorClicked;

        /// <summary>Event handler for when the in connecter is clicked</summary>
        public event EventHandler? InConnectorClicked;

        /// <summary>Mouse event handler for when the user is dragging the node</summary>
        public event MouseButtonEventHandler? DragHandleMouseDown;

        #region Translation Event Handlers

        /// <summary>
        /// Opens the key-picker popup, populated with translation keys not yet on this node.
        /// </summary>
        private void AddTranslationButton_Click(object sender, RoutedEventArgs e)
        {
            if (VM == null) 
            {
                return;
            }

            // (Re)build the picker items each time so the list is always fresh.
            TranslationKeyPickerPanel.Children.Clear();

            foreach (string key in VM.AvailableTranslationKeys)
            {
                string capturedKey = key;

                var item = new Button
                {
                    Content = key,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    HorizontalContentAlignment = HorizontalAlignment.Left,
                    Background = Brushes.Transparent,
                    Foreground = new SolidColorBrush(Color.FromRgb(220, 220, 220)),
                    BorderThickness = new Thickness(0),
                    Padding = new Thickness(8, 5, 8, 5),
                    Cursor = Cursors.Hand,
                    FontSize = 12,
                };

                // Hover tint
                item.MouseEnter += (_, _) => item.Background = new SolidColorBrush(Color.FromArgb(60, 120, 200, 120));
                item.MouseLeave += (_, _) => item.Background = Brushes.Transparent;

                item.Click += (_, _) =>
                {
                    AddTranslationPopup.IsOpen = false;

                    if (UndoRedoService != null)
                    {
                        AddTranslationEntryCommand cmd = new AddTranslationEntryCommand(VM, capturedKey);
                        cmd.Execute();
                        UndoRedoService.Record(cmd);
                    }
                    else
                    {
                        VM.AddTranslation(capturedKey);
                    }
                };

                TranslationKeyPickerPanel.Children.Add(item);
            }

            AddTranslationPopup.IsOpen = TranslationKeyPickerPanel.Children.Count > 0;
        }

        /// <summary>
        /// Removes the translation entry whose remove button was clicked.
        /// </summary>
        private void RemoveTranslation_Click(object sender, RoutedEventArgs e)
        {
            if (VM == null)
            {
                return;
            }

            if (sender is Button btn && btn.Tag is DialogueTranslationEntryViewModel entry)
            {
                if (UndoRedoService != null)
                {
                    RemoveTranslationEntryCommand cmd = new RemoveTranslationEntryCommand(VM, entry);
                    UndoRedoService.Record(cmd);
                }
                else
                {
                    VM.RemoveTranslation(entry);
                }
            }
        }

        #endregion // Translation Event Handlers

        #region // Notes Event Handlers
        private void NotesToggleButton_Click(object sender, RoutedEventArgs e)
        {
            NotesBorder.Visibility = NotesBorder.Visibility == Visibility.Visible
                ? Visibility.Collapsed
                : Visibility.Visible;
        }

        #endregion // Notes Event Handlers

        #endregion // Event Handler

        #region Helper Functions

        /// <summary>
        /// Gets center position of the passed connector
        /// </summary>
        /// <param name="connector">Connect to get center position of</param>
        /// <param name="relativeTo">UI element the node is relative to</param>
        /// <returns>Point - Center position of the passed connector</returns>
        public static Point GetConnectorCenter(Ellipse connector, UIElement relativeTo)
        {
            GeneralTransform transform = connector.TransformToAncestor(relativeTo);
            return transform.Transform(new Point(connector.ActualWidth / 2, connector.ActualHeight / 2));
        }

        /// <summary>
        /// Applies hover effect to connector
        /// </summary>
        /// <param name="el">Connector to apply hover effect</param>
        /// <param name="normal">The non hovered colour</param>
        /// <param name="hover">The hovered colour</param>
        private static void HoverEffect(Ellipse el, Color normal, Color hover)
        {
            el.MouseEnter += (_, _) => el.Fill = new SolidColorBrush(hover);
            el.MouseLeave += (_, _) => el.Fill = new SolidColorBrush(normal);
        }

        #endregion // Helper Functions
    }
}
