using System.IO;
using System.Runtime.CompilerServices;

namespace DialogueNodeEditor.Utilities
{
    /// <summary>
    /// Log levels
    /// </summary>
    public enum LogLevel
    {
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    }

    public class Logger
    {
        #region Init / Deinit

        /// <summary>
        /// Constructor for Logger object
        /// </summary>
        /// <param name="name">Name of the logger</param>
        /// <param name="logFilePath">Path to log file</param>
        public Logger(string name, string? logFilePath = null)
        {
            _name = name;
            _logFilePath = logFilePath;

            if (_logFilePath != null)
            {
                string? dir = Path.GetDirectoryName(_logFilePath);
                if (!string.IsNullOrEmpty(dir))
                {
                    Directory.CreateDirectory(dir);
                }
            }
        }

        #endregion // Init / Deinit

        #region Member Variables

        /// <summary>Logger name</summary>
        private readonly string _name;

        /// <summary>File to write log to</summary>
        private readonly string? _logFilePath;

        /// <summary>Lock for writing to console and the log file (if it exists)</summary>
        private readonly object _lock = new();

        #endregion // Member Variables

        #region Constants

        /// <summary>Console colours for each level</summary>
        private static readonly ConsoleColor[] LevelColors =
        [
            ConsoleColor.DarkGray,  // Debug
            ConsoleColor.Cyan,      // Info
            ConsoleColor.Yellow,    // Warning
            ConsoleColor.Red,       // Error
            ConsoleColor.Magenta    // Fatal
        ];

        /// <summary>Console colours for each log level</summary>
        private static readonly string[] LevelLabels =
        [
            "DBG", "INF", "WRN", "ERR", "FTL"
        ];

        #endregion // Constants

        /// <summary>
        /// Writes debug log
        /// </summary>
        /// <param name="message">Log message</param>
        /// <param name="caller">Caller of the log</param>
        /// <param name="line">Line this is being called on</param>
        public void Debug(string message,
            [CallerMemberName] string caller = "",
            [CallerLineNumber] int line = 0)
            => Log(LogLevel.Debug, message, caller, line);

        /// <summary>
        /// Writes info log
        /// </summary>
        /// <param name="message">Log message</param>
        /// <param name="caller">Caller of the log</param>
        /// <param name="line">Line this is being called on</param>
        public void Info(string message,
            [CallerMemberName] string caller = "",
            [CallerLineNumber] int line = 0)
            => Log(LogLevel.Info, message, caller, line);

        /// <summary>
        /// Writes warning log
        /// </summary>
        /// <param name="message">Log message</param>
        /// <param name="caller">Caller of the log</param>
        /// <param name="line">Line this is being called on</param>
        public void Warning(string message,
            [CallerMemberName] string caller = "",
            [CallerLineNumber] int line = 0)
            => Log(LogLevel.Warning, message, caller, line);

        /// <summary>
        /// Writes error log
        /// </summary>
        /// <param name="message">Log message</param>
        /// <param name="caller">Caller of the log</param>
        /// <param name="line">Line this is being called on</param>
        public void Error(string message,
            [CallerMemberName] string caller = "",
            [CallerLineNumber] int line = 0)
            => Log(LogLevel.Error, message, caller, line);

        /// <summary>
        /// Writes error log
        /// </summary>
        /// <param name="message">Log message</param>
        /// <param name="caller">Caller of the log</param>
        /// <param name="line">Line this is being called on</param>
        public void Error(Exception ex,
            [CallerMemberName] string caller = "",
            [CallerLineNumber] int line = 0)
            => Log(LogLevel.Error, $"{ex.GetType().Name}: {ex.Message}\n{ex.StackTrace}", caller, line);
        
        /// <summary>
        /// Writes fatal log
        /// </summary>
        /// <param name="message">Log message</param>
        /// <param name="caller">Caller of the log</param>
        /// <param name="line">Line this is being called on</param>
        public void Fatal(string message,
            [CallerMemberName] string caller = "",
            [CallerLineNumber] int line = 0)
            => Log(LogLevel.Fatal, message, caller, line);

        /// <summary>
        /// Writes message to log
        /// </summary>
        /// <param name="level">Log level</param>
        /// <param name="message">Log message</param>
        /// <param name="caller">Caller of the log</param>
        /// <param name="line">Line this is being called on</param>
        private void Log(LogLevel level, string message, string caller, int line)
        {
            string timestamp = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff");
            string label = LevelLabels[(int)level];
            string formatted = $"[{timestamp}] [{label}] [{_name}] [{caller}:{line}] {message}";

            lock (_lock)
            {
                ConsoleColor prev = Console.ForegroundColor;
                Console.ForegroundColor = LevelColors[(int)level];
                Console.WriteLine(formatted);
                Console.ForegroundColor = prev;

                if (_logFilePath != null)
                {
                    try
                    {
                        File.AppendAllText(_logFilePath, formatted + Environment.NewLine);
                    }
                    catch
                    {
                    }
                }
            }
        }
    }
}
