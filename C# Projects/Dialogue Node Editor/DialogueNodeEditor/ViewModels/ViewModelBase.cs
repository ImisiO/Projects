using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace DialogueNodeEditor.ViewModels
{
    public abstract class ViewModelBase : INotifyPropertyChanged
    {
        /// <summary>Occurs when a property value changes.</summary>
        public event PropertyChangedEventHandler? PropertyChanged;

        /// <summary>
        /// Sets the backing field to the given value and raises <see cref="PropertyChanged"/> if the value has changed.
        /// </summary>
        /// <typeparam name="T">The type of the property.</typeparam>
        /// <param name="field">A reference to the backing field of the property.</param>
        /// <param name="value">The new value to assign to the field.</param>
        /// <param name="propertyName">
        /// The name of the property that changed. Automatically supplied by the compiler
        /// via <see cref="CallerMemberNameAttribute"/> when called from a property setter.
        /// </param>
        /// <returns>
        /// <see langword="true"/> if the value was changed and <see cref="PropertyChanged"/> was raised;
        /// <see langword="false"/> if the new value was equal to the existing value and no event was raised.
        /// </returns>
        protected bool SetField<T>(ref T field, T value, [CallerMemberName] string? propertyName = null)
        {
            if (EqualityComparer<T>.Default.Equals(field, value))
            {
                return false;
            }
            field = value;
            OnPropertyChanged(propertyName);
            return true;
        }

        /// <summary>
        /// Raises the <see cref="PropertyChanged"/> event for the specified property.
        /// </summary>
        /// <param name="propertyName">
        /// The name of the property that changed. Automatically supplied by the compiler
        /// via <see cref="CallerMemberNameAttribute"/> when called directly from a property setter.
        /// Pass <see langword="null"/> to indicate that all properties on the object have changed.
        /// </param>
        protected void OnPropertyChanged([CallerMemberName] string? propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
