using System.Windows.Input;

namespace DialogueNodeEditor.Commands
{
    public class RelayCommand : ICommand
    {
        #region Init/Deinit

        /// <summary>
        /// Constructor for RelayCommand object
        /// </summary>
        /// <param name="execute">Callback/action to call</param>
        /// <param name="canExecute">Function to check whether the command can be ran/executed</param>
        public RelayCommand(Action<object?> execute, Func<object?, bool>? canExecute = null)
        {
            _execute = execute;
            _canExecute = canExecute;
        }

        #endregion // Init/Deinit
        
        #region Member Variables

        /// <summary>Action/Callback to execute</summary>
        private readonly Action<object?> _execute;

        /// <summary>Function to check whether the action/callback can be executed/called</summary>
        private readonly Func<object?, bool>? _canExecute;

        #endregion // Member Variables

        #region Event Handlers

        public event EventHandler? CanExecuteChanged;

        #endregion // Event Handlers

        #region Functions

        /// <summary>
        /// Checks whether the command can be ran on on the passed object
        /// </summary>
        /// <param name="parameter">Object</param>
        /// <returns>bool - Whether or not the command can be run on the passed object</returns>
        public bool CanExecute(object? parameter)
        { 
            return _canExecute == null || _canExecute(parameter);
        }

        /// <summary>
        /// Executes command
        /// </summary>
        /// <param name="parameter">Object to run command on</param>
        public void Execute(object? parameter) => _execute(parameter);

        /// <summary>
        /// 
        /// </summary>
        public void RaiseCanExecuteChanged()
            => CanExecuteChanged?.Invoke(this, EventArgs.Empty);

        #endregion // Functions
    }
}
