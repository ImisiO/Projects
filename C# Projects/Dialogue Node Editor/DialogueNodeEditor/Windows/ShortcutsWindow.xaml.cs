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
using System.Windows.Shapes;

namespace DialogueNodeEditor.Windows
{
    /// <summary>
    /// Interaction logic for ShortcutsWindow.xaml
    /// </summary>
    public partial class ShortcutsWindow : Window
    {
        public ShortcutsWindow()
        {
            InitializeComponent();
        }

        /// <summary>Close button click — closes the window.</summary>
        private void CloseButton_Click(object sender, RoutedEventArgs e) => Close();

        /// <summary>
        /// Handles shortcuts in window
        /// </summary>
        /// <param name="sender">Sender of event</param>
        /// <param name="e">Key event arguments</param>
        private void Window_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Escape) 
            {
                Close();
            }
        }
    }
}
